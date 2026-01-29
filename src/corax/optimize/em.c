#include "corax/core/common.h"
#include "corax/core/partition.h"
#include "corax/tree/treeinfo.h"
#include "opt_generic.h"
#include <limits.h>

/******************************************************************************/
/* EXPECTATION-MAXIMIZATION (EM)     */
/* Wang, Li, Susko, and Roger (2008) */
/******************************************************************************/
CORAX_EXPORT void
corax_opt_minimize_em(double *             w,
                      unsigned int         w_count,
                      double *             sitecat_lh,
                      const unsigned int * site_w,
                      unsigned int        site_w_sum,
                      unsigned int         l,
                      void *               params,
                      double (*update_sitecatlk_funk)(void *, double *))
{
  unsigned int i, c;
  unsigned int max_steps   = 10;
  int          converged   = 0;
  int          ratio_scale = 0;

  double *new_prop   = (double *)malloc(sizeof(double) * w_count);
  double *ratio_prop = (double *)malloc(sizeof(double) * w_count);

    DBG("corax_opt_minimize_em iteration %i weights = (", 10 - max_steps + 1);
    for (unsigned c = 0; c < w_count; ++c) {
      DBG(" %f ", w[c]);
    }
    DBG(")\n");

  while (!converged && max_steps--)
  {
    /* update site-cat LK */
    if (update_sitecatlk_funk) {
      update_sitecatlk_funk(params, sitecat_lh);
    }

    // Expectation
    double *this_lk_cat = sitecat_lh;
    if (ratio_scale)
    {
      for (i = 0; i < l; ++i)
      {
        for (c = 0; c < w_count; c++) { this_lk_cat[c] *= ratio_prop[c]; }
        this_lk_cat += w_count;
      }
    }
    else
      ratio_scale = 1;

    memset(new_prop, 0, w_count * sizeof(double));

    this_lk_cat = sitecat_lh;
    for (i = 0; i < l; ++i)
    {
      // TODO: Check for p_invar
      double lk_ptn = 0;
      for (c = 0; c < w_count; c++) { lk_ptn += this_lk_cat[c]; }
      lk_ptn = site_w[i] / lk_ptn;
      for (c = 0; c < w_count; c++) { new_prop[c] += this_lk_cat[c] * lk_ptn; }
      this_lk_cat += w_count;
    }

    // Maximization
    converged = 1;
    DBG("corax_opt_minimize_em iteration %i weights = (", 10 - max_steps + 1);
    for (c = 0; c < w_count; c++)
    {
      new_prop[c] /= site_w_sum;

      // check for convergence
      converged     = converged && (fabs(w[c] - new_prop[c]) < 1e-4);
      ratio_prop[c] = new_prop[c] / w[c];
      w[c]          = new_prop[c];
      DBG(" %f ", w[c]);
    }

    DBG(") converged = %i\n", converged);
  }

  free(ratio_prop);
  free(new_prop);
}


CORAX_EXPORT corax_opt_multipart_em_data_t *
corax_opt_multipart_em_initialize(corax_treeinfo_t *treeinfo)
{
    const unsigned int part_count = treeinfo->partition_count;
    assert(part_count >= 1);
    corax_opt_multipart_em_data_t *r = (corax_opt_multipart_em_data_t *) malloc(sizeof(corax_opt_multipart_em_data_t));

    r->treeinfo = treeinfo;

    r->prefix_sum_category_count = (unsigned int *) malloc(sizeof(unsigned int) * (part_count + 1));

    r->pattern_weight_sum_per_part = (double *) malloc(sizeof(double *) * part_count);
    memset(r->pattern_weight_sum_per_part, 0, sizeof(double) * part_count);

    r->sitecat_lh_per_part = (double **) malloc(sizeof(double **) * part_count);
    memset(r->sitecat_lh_per_part, 0, sizeof(double **) * part_count);

    r->sitecat_posterior_per_part = (double **) malloc(sizeof(double **) * part_count);
    memset(r->sitecat_posterior_per_part, 0, sizeof(double **) * part_count);

    /* Number of rate categories for each partition as floating-point number
    * (to maintain compatibility with parallel_reduce_cb, which only
    * supports double) */
    double *rate_cats_per_part = (double *) malloc(sizeof(double) * part_count);
    memset(rate_cats_per_part, 0, sizeof(double) * part_count);

    for (unsigned int p = 0; p < part_count; ++p) {
        corax_partition_t *part = treeinfo->partitions[p];
        if (!part || !(treeinfo->params_to_optimize[p] & CORAX_OPT_PARAM_RATE_WEIGHTS)) {
            continue;
        }
        r->pattern_weight_sum_per_part[p] = part->pattern_weight_sum;
        r->sitecat_lh_per_part[p] = (double *) malloc(sizeof(double) * part->sites * part->rate_cats);
        r->sitecat_posterior_per_part[p] = (double *) malloc(sizeof(double) * part->sites * part->rate_cats);
        rate_cats_per_part[p] = part->rate_cats;
    }

    /* Get number of rate categories and global number of sites for all partitions (remote and local) */
    corax_treeinfo_parallel_reduce(treeinfo, rate_cats_per_part, part_count, CORAX_REDUCE_MAX);
    corax_treeinfo_parallel_reduce(treeinfo, r->pattern_weight_sum_per_part, part_count, CORAX_REDUCE_SUM);

    r->prefix_sum_category_count[0] = 0;
    for (unsigned int p = 0; p < part_count; ++p) {
        r->prefix_sum_category_count[p + 1] = r->prefix_sum_category_count[p] + ((unsigned int) rate_cats_per_part[p]);
    }
    free(rate_cats_per_part); rate_cats_per_part = NULL;
    r->total_rate_cats = r->prefix_sum_category_count[treeinfo->partition_count];

    r->weights = (double *) malloc(sizeof(double) * r->total_rate_cats);
    r->new_weights = (double *) malloc(sizeof(double) * r->total_rate_cats);
    r->converged = (bool *) malloc(sizeof(bool) * treeinfo->partition_count);
    r->category_lh = (double *) malloc(sizeof(double) * r->total_rate_cats);

    memset(r->converged, 0, sizeof(bool) * treeinfo->partition_count);

    // Copy initial weights from partition
    for (unsigned int p = 0; p < part_count; ++p) {
        corax_partition_t *part = treeinfo->partitions[p];
        if (!part || !(treeinfo->params_to_optimize[p] & CORAX_OPT_PARAM_RATE_WEIGHTS)) {
            continue;
        }
        memcpy(&r->weights[r->prefix_sum_category_count[p]], part->rate_weights, sizeof(double) * part->rate_cats);
    }

    /* Precompute scaling factors */
    r->scale_factor_powers[0] = CORAX_SCALE_FACTOR;
    for (unsigned int i = 1; i < CORAX_SCALE_RATE_MAXDIFF; ++i) {
        r->scale_factor_powers[i] = r->scale_factor_powers[i - 1] * CORAX_SCALE_FACTOR;
    }

    return r;
}

CORAX_EXPORT void
corax_opt_multipart_em_free(corax_opt_multipart_em_data_t *data)
{
    free(data->category_lh); data->category_lh = NULL;
    free(data->converged); data->converged = NULL;
    free(data->new_weights); data->new_weights= NULL;
    free(data->weights); data->weights = NULL;

    for (unsigned int p = 0; p < data->treeinfo->partition_count; ++p) {
        if (data->sitecat_lh_per_part[p]) {
            free(data->sitecat_posterior_per_part[p]);
            data->sitecat_posterior_per_part[p] = NULL;
            free(data->sitecat_lh_per_part[p]);
            data->sitecat_lh_per_part[p] = NULL;
        }
    }

    free(data->sitecat_posterior_per_part); data->sitecat_posterior_per_part = NULL;
    free(data->sitecat_lh_per_part); data->sitecat_lh_per_part = NULL;

    free(data->pattern_weight_sum_per_part);
    free(data->prefix_sum_category_count);
    free(data);
}

CORAX_EXPORT unsigned int
corax_compute_root_edge_site_scalings(corax_treeinfo_t *treeinfo,
                                      corax_partition_t *part,
                                      unsigned int site) {
    unsigned int site_scalings = 0;
    unsigned int parent_clv_index = treeinfo->root->clv_index;
    unsigned int child_clv_index = treeinfo->root->back->clv_index;
    int parent_scaler = treeinfo->root->scaler_index;
    int child_scaler = treeinfo->root->back->scaler_index;

    if (part->scale_buffers > 0) {
        unsigned int pid = CORAX_GET_ID(corax_get_site_id(part, parent_clv_index), site);
        unsigned int cid = CORAX_GET_ID(corax_get_site_id(part, child_clv_index), site);

        if (part->attributes & CORAX_ATTRIB_RATE_SCALERS) {
            // per-site scaling is minimum among rate scalers
            site_scalings = UINT_MAX;

            for (unsigned int c = 0; c < part->rate_cats; ++c) {
                site_scalings = CORAX_MIN(site_scalings,
                    ((parent_scaler == -1) ? 0 : part->scale_buffer[parent_scaler][pid * part->rate_cats + c]) +
                    ((child_scaler == -1) ? 0 : part->scale_buffer[child_scaler][cid * part->rate_cats + c]));

            }

        } else {
            site_scalings = \
                ((parent_scaler == -1) ? 0 : part->scale_buffer[parent_scaler][pid]) +
                ((child_scaler == -1) ? 0 : part->scale_buffer[child_scaler][cid]);
        }
    }

    return site_scalings;
}

/**
 * Transform the per-site per-category likelihood into the posterior probability
 * p_ij of site i belonging to category j and accumulate new weights.
 *
 * Equation number in source code comments refer to Kalyaanamoorthy, Subha, Bui
 * Quang Minh, Thomas K. F. Wong, Arndt von Haeseler, and Lars S. Jermiin. 2017.
 * ‘ModelFinder: Fast Model Selection for Accurate Phylogenetic Estimates’.
 * Nature Methods 14 (6): 587–89. https://doi.org/10.1038/nmeth.4285.
 */
void transform_sitecatlh_to_posterior(corax_opt_multipart_em_data_t *data) {
    for (unsigned int p = 0; p < data->treeinfo->partition_count; ++p) {
        corax_partition_t *part = data->treeinfo->partitions[p];

        if (!part || data->converged[p] || !(data->treeinfo->params_to_optimize[p] & CORAX_OPT_PARAM_RATE_WEIGHTS))
            continue;

        double *this_lk_cat = data->sitecat_lh_per_part[p];
        double *this_posterior = data->sitecat_posterior_per_part[p];
        const unsigned partition_offset = data->prefix_sum_category_count[p];



        for (unsigned int i = 0; i < part->sites; ++i) {
            const unsigned int pattern_weight = part->pattern_weights[i];

            double pattern_likelihood = 0;
            double term_inv = 0;

            // Handle invariant sites
            double prop_invar = part->prop_invar ? part->prop_invar[0] : 0.0;
            // Possible optimization: precomputing the invariant proportion per-state outside of the site loop
            if (prop_invar > 0.0 && part->invariant && part->invariant[i] != -1) {
                int site_state = part->invariant[i];

                for (unsigned int c = 0; c < part->rate_cats; ++c) {
                    term_inv += prop_invar \
                                * part->rate_weights[c] \
                                * part->frequencies[data->treeinfo->param_indices[p][c]][site_state];
                }

            }

            // compute \f$ \sum_{c=1}^k w_c L(D_i | T, S, r_c) \f$ (divisor of Eq. (1))
            for (unsigned int c = 0; c < part->rate_cats; ++c) {
                pattern_likelihood += this_lk_cat[c];
            }

            // Scale pattern likelihood in case we have an invariant proportion
            unsigned int site_scalings = corax_compute_root_edge_site_scalings(data->treeinfo, part, i);
            const unsigned int capped_scalings = CORAX_MIN(site_scalings, CORAX_SCALE_RATE_MAXDIFF);
            const double scale_factor = capped_scalings > 0 ? data->scale_factor_powers[capped_scalings - 1] : 1.0;
            if (term_inv > 0.) {
                pattern_likelihood = pattern_likelihood + term_inv * scale_factor;
                assert(pattern_likelihood > 0.);
            }

            for (unsigned int c = 0; c < part->rate_cats; ++c) {
                // TODO: check if multiplication by reciprocal of pattern_likelihood would be faster

                // Compute Eq. (1)
                this_posterior[c] = pattern_weight * this_lk_cat[c] / pattern_likelihood;

                // Eq. (3)
                data->new_weights[partition_offset + c] += this_posterior[c];
            }

            this_lk_cat += part->rate_cats;
            this_posterior += part->rate_cats;
        }
    }
}

CORAX_EXPORT void
corax_opt_minimize_em_multipartition(corax_opt_multipart_em_data_t *data) {
    const unsigned int overall_category_count = data->total_rate_cats;
    // Determine number of iterations = max rate category count
    unsigned int total_steps = 1;
    for (unsigned int p = 0; p < data->treeinfo->partition_count; ++p) {
        const unsigned int rate_cats = data->prefix_sum_category_count[p+1] - data->prefix_sum_category_count[p];

        if (rate_cats > total_steps) {
            total_steps = rate_cats;
        }

        data->converged[p] = false;
    }

    bool all_partitions_converged = false;
    for (unsigned int step = 0; (step < total_steps) && !all_partitions_converged; ++step) {
        memset(data->new_weights, 0, sizeof(double) * overall_category_count);

        // Expectation step
        double loglh = corax_treeinfo_compute_loglh_sitecat(data->treeinfo, 0, 0, data->sitecat_lh_per_part);
        transform_sitecatlh_to_posterior(data);

        corax_treeinfo_parallel_reduce(data->treeinfo,
                data->new_weights, overall_category_count, CORAX_REDUCE_SUM);


        all_partitions_converged = true;
        for (unsigned int p = 0; p < data->treeinfo->partition_count; ++p) {
            const unsigned int category_count = data->prefix_sum_category_count[p + 1] - data->prefix_sum_category_count[p];
            const unsigned partition_offset = data->prefix_sum_category_count[p];
            
            // only update partitions which have less than step categories
            data->converged[p] = data->converged[p] || (step > category_count);

            DBG("\tpart %i ", p);
            if (data->converged[p] || !(data->treeinfo->params_to_optimize[p] & CORAX_OPT_PARAM_RATE_WEIGHTS)) {
                DBG("converged\n");
                continue;
            }


            // Maximization step
            double weight_sum = 0;
            for (unsigned int c = 0; c < category_count; c++) {
                const unsigned int c_idx = partition_offset + c;
                double w = data->new_weights[c_idx] / data->pattern_weight_sum_per_part[p];

                if (w < CORAX_OPT_MIN_RATE_WEIGHT) {
                    w = CORAX_OPT_MIN_RATE_WEIGHT;
                    data->converged[p] = true;
                }

                weight_sum += w;
                data->new_weights[c_idx] = w;
            }

            for (unsigned int c = 0; c < category_count; c++) {
                const unsigned int c_idx = partition_offset + c;

                // Normalize the weights if required
                if (weight_sum < 1.0 || weight_sum > 1.0) {
                    data->new_weights[c_idx] *= (1.0 / weight_sum);
                }

                data->converged[p] = data->converged[p] || fabs(data->weights[c_idx] - data->new_weights[c_idx]) < 1e-4;
                data->weights[c_idx] = data->new_weights[c_idx];
                DBG(" %f ", data->weights[c_idx]);
            }
            DBG("\n");

            // Copy weights to partition if it is local
            corax_partition_t *part = data->treeinfo->partitions[p];
            if (part) {
                memcpy(part->rate_weights, &data->weights[data->prefix_sum_category_count[p]], sizeof(double) * part->rate_cats);
            }

            all_partitions_converged = all_partitions_converged && data->converged[p];
        }
    }
}
