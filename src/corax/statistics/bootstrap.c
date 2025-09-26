#include "bootstrap.h"

#include "corax/util/random.h"

/**
 * Fill a given vector with integer weights corresponding to a uniform resampling (with replacement) chosen from
 * an initial set with num_sites elements.
 * The vector will sum to num_sites * scale (rounded by normal casting rules), meaning we chose num_sites * scale
 * elements from the base set.
 * The weight vector can then be multiplied with the base vector to obtain the final sum of the resampling.
 *
 * The vector is initialized with zeros before it is filled with data.
 */
void generate_resample_vector(double** vector, corax_random_state *rstate, int num_sites, double scale) {
    int num_samples = (int) ((double) num_sites * scale);

    // reset vector
    for (int j = 0; j < num_sites; j++)
        *vector[j] = 0.0;

    // fill vector with uniform sample weights
    for (int j = 0; j < num_samples; j++) {
        *vector[corax_random_getint(rstate, num_sites)] += 1.0;
    }
}

CORAX_EXPORT void corax_RELL_bootstrap(double ***replicates,
                                       double ***trees_persite_lnl,
                                       corax_random_state *rstate,
                                       int num_sites, int num_replicates, double scale) {
    double *weights = malloc(sizeof(double) * num_sites);

    for (unsigned int replicate = 0; replicate < num_replicates; replicate++) {
        generate_resample_vector(&weights, rstate, num_sites, scale);

        for (unsigned int id_tree = 0; id_tree < num_replicates; id_tree++) {
            for (unsigned int id_site = 0; id_site < num_sites; id_site++) {
                // TODO SIMD optimization
                *replicates[id_tree][replicate] += weights[id_site] * *trees_persite_lnl[id_tree][id_site];
            }
        }
    }

    free(weights);
}