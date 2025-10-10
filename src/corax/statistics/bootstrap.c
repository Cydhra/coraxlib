#include "bootstrap.h"

#include "corax/util/random.h"
#include <math.h>

#include "corax/optimize/lbfgsb/lbfgsb.h"

/**
 * Fill the vector argument with weights corresponding to a uniform bootstrap resampling chosen from
 * an initial set with num_sites_original elements.
 * The weight vector will not have the same length as the per-site likelihood vector because the weights are neither
 * partitioned nor compressed.
 * The vector will sum to num_sites_original * scale (rounded by normal casting rules),
 * meaning we choose a number of sites with a factor of scale.
 * The weight vector can then be multiplied with the base set to obtain the final sum of the resampling.
 * The vector can be multiplied with all per-site likelihood rows of an alignment to obtain a bootstrap sample
 * log-likelihood of the alignment with corresponding sites chosen from each vector.
 *
 * The vector is initialized with zeros before it is filled with data.
 */
void generate_resample_vector(double *const vector, corax_random_state *const rstate,
                              const unsigned int num_sites_original,
                              const double scale) {
    const unsigned int num_samples = (unsigned int) ((double) num_sites_original * scale);

    // reset vector
    for (unsigned int j = 0; j < num_sites_original; j++)
        vector[j] = 0.0;

    // fill vector with uniform sample weights
    for (unsigned int j = 0; j < num_samples; j++) {
        vector[corax_random_getint(rstate, num_sites_original)] += 1.0;
    }
}

/**
 * Find the maximum in the `replicate`-th column of the `replicates` matrix, stored in row-major format with
 * `num_replicates` columns and `num_trees` rows.
 */
double column_max(const double *const replicates, const unsigned int replicate, const unsigned int num_replicates,
                  const unsigned int num_trees) {
    double best_value = -INFINITY;
    for (unsigned int id_tree = 0; id_tree < num_trees; id_tree++) {
        if (replicates[id_tree * num_replicates + replicate] > best_value) {
            best_value = replicates[id_tree * num_replicates + replicate];
        }
    }

    return best_value;
}

/**
 * Compare two doubles for sorting
 */
int cmp_double(const void *const a, const void *const b) {
    const double da = *(const double *) a;
    const double db = *(const double *) b;
    return (da > db) - (da < db);
}

/**
 * Binary-search for the position of the threshold element, if it was in the sorted vector.
 */
unsigned int bposition(const double *const vector, const double threshold, const unsigned int len) {
    unsigned int l = 0, r = len;
    while (l < r) {
        unsigned int mid = l + (r - l) / 2;
        if (vector[mid] < threshold)
            l = mid + 1;
        else
            r = mid;
    }
    return l;
}

// TODO the trees_persite_lnl has to be a 3D array, where we also pass in multiple partitions of the alignment
CORAX_EXPORT void corax_RELL_bootstrap(double **replicates,
                                       double **const trees_persite_lnl,
                                       corax_random_state *rstate,
                                       const unsigned int num_sites,
                                       const unsigned int num_replicates,
                                       const unsigned int num_trees, const double scale) {
    double *weights = malloc(sizeof(double) * num_sites);
    // TODO proper handling
    if (!weights) {
        exit(-1);
    }

    if (!*replicates) {
        *replicates = malloc(sizeof(double) * num_replicates * num_trees);

        if (!*replicates) {
            // TODO proper handling
            exit(-1);
        }

        memset(*replicates, 0, sizeof(double) * num_replicates * num_trees);
    }

    for (unsigned int replicate = 0; replicate < num_replicates; replicate++) {
        generate_resample_vector(weights, rstate, num_sites, scale);

        // TODO select only the compressed sites and the sites from the current partition from the resample vector

        for (unsigned int id_tree = 0; id_tree < num_trees; id_tree++) {
            for (unsigned int id_site = 0; id_site < num_sites; id_site++) {
                // TODO SIMD optimization
                (*replicates)[id_tree * num_replicates + replicate] += weights[id_site] * trees_persite_lnl[id_tree][
                    id_site];
            }

            (*replicates)[id_tree * num_replicates + replicate] /= scale;
        }
    }

    free(weights);
}

CORAX_EXPORT void corax_RELL_multiscale_bootstrap(double ***replicate_matrices,
                                                  double **const trees_persite_lnl,
                                                  corax_random_state *rstate,
                                                  const unsigned int num_sites, const unsigned int num_trees,
                                                  const unsigned int *const num_replicates,
                                                  const double *const scales, const unsigned int num_scales) {
    if (!*replicate_matrices) {
        *replicate_matrices = malloc(sizeof(double *) * num_scales);

        if (!*replicate_matrices) {
            // TODO proper handling
            exit(-1);
        }

        memset(*replicate_matrices, 0, sizeof(double *) * num_scales);
    }

    for (unsigned int i = 0; i < num_scales; i++) {
        corax_RELL_bootstrap(replicate_matrices[i], trees_persite_lnl, rstate, num_sites, num_replicates[i], num_trees,
                             scales[i]);
    }
}

CORAX_EXPORT void corax_normalize_lnl_bootstrap(double *const replicates,
                                                const unsigned int num_replicates,
                                                const unsigned int num_trees) {
    // calculate maximum of each replicate set
    double *maximum = malloc(sizeof(double) * num_replicates);
    // TODO proper handling
    if (!maximum) {
        exit(-1);
    }

    for (unsigned int replicate = 0; replicate < num_replicates; replicate++) {
        maximum[replicate] = column_max(replicates, replicate, num_replicates, num_trees);
    }

    // normalize vectors
    for (unsigned int id_tree = 0; id_tree < num_trees; id_tree++) {
        for (unsigned int replicate = 0; replicate < num_replicates; replicate++) {
            replicates[id_tree * num_replicates + replicate] -= maximum[replicate];
        }

        double *tree_vec = replicates + id_tree * num_replicates;
        qsort(tree_vec, num_replicates, sizeof(double), cmp_double);
    }

    free(maximum);
}

CORAX_EXPORT double corax_empirical_bootstrap_count(double *const replicates,
                                                    const unsigned int num_replicates,
                                                    const unsigned int tree,
                                                    const double threshold) {
    double *tree_vec = replicates + tree * num_replicates;
    const unsigned int cutoff = bposition(tree_vec, threshold, num_replicates);

    double smoothed;
    if (cutoff < num_replicates - 1) {
        if (tree_vec[cutoff + 1] > tree_vec[cutoff]) {
            smoothed = 0.5 + (double) cutoff + (threshold - tree_vec[cutoff]) / (
                           tree_vec[cutoff + 1] - tree_vec[cutoff]);
        } else if (cutoff > 0) {
            smoothed = 0.5 + (double) cutoff;
        } else {
            smoothed = 0.0;
        }
    } else if (tree_vec[num_replicates - 1] - tree_vec[num_replicates - 2] > 0.0) {
        smoothed = 0.5 + (double) num_replicates +
                   (threshold - tree_vec[num_replicates - 2]) / (
                       tree_vec[num_replicates - 1] - tree_vec[num_replicates]);
    } else {
        smoothed = num_replicates;
    }

    if (smoothed > (double) num_replicates) {
        return num_replicates;
    }

    if (smoothed < 0.0) {
        return 0.0;
    }

    return smoothed;
}
