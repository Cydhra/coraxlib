#include "corax/core/common.h"
#include "corax/core/partition.h"
#include "corax/tree/treeinfo.h"
#include "opt_generic.h"

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
        rate_cats_per_part[p] = part->rate_cats;
    }

    /* Get number of rate categories and global number of sites for all partitions (remote and local) */
    corax_treeinfo_parallel_reduce(treeinfo, rate_cats_per_part, part_count, CORAX_REDUCE_MAX);
    corax_treeinfo_parallel_reduce(treeinfo, r->pattern_weight_sum_per_part, part_count, CORAX_REDUCE_SUM);

    r->prefix_sum_category_count[0] = 0;
    for (unsigned int p = 0; p < part_count; ++p) {
        r->prefix_sum_category_count[p + 1] = r->prefix_sum_category_count[p] + rate_cats_per_part[p];
    }
    free(rate_cats_per_part); rate_cats_per_part = NULL;
    r->total_rate_cats = r->prefix_sum_category_count[treeinfo->partition_count];

    r->weights = (double *) malloc(sizeof(double) * r->total_rate_cats);
    r->new_weights = (double *) malloc(sizeof(double) * r->total_rate_cats);
    r->weight_ratio = (double *) malloc(sizeof(double) * r->total_rate_cats);
    r->converged = (bool *) malloc(sizeof(bool) * treeinfo->partition_count);

    // Copy initial weights from partition
    for (unsigned int p = 0; p < part_count; ++p) {
        corax_partition_t *part = treeinfo->partitions[p];
        if (!part || !(treeinfo->params_to_optimize[p] & CORAX_OPT_PARAM_RATE_WEIGHTS)) {
            continue;
        }
        memcpy(&r->weights[r->prefix_sum_category_count[p]], part->rate_weights, sizeof(double) * part->rate_cats);
    }

    return r;
}

CORAX_EXPORT void
corax_opt_multipart_em_free(corax_opt_multipart_em_data_t *data)
{
    free(data->converged); data->converged = NULL;
    free(data->weight_ratio); data->weight_ratio = NULL;
    free(data->new_weights); data->new_weights= NULL;
    free(data->weights); data->weights = NULL;

    for (unsigned int p = 0; p < data->treeinfo->partition_count; ++p) {
        if (data->sitecat_lh_per_part[p]) {
            free(data->sitecat_lh_per_part[p]);
            data->sitecat_lh_per_part[p] = NULL;
        }
    }

    free(data->sitecat_lh_per_part); data->sitecat_lh_per_part = NULL;

    free(data->pattern_weight_sum_per_part);
    free(data->prefix_sum_category_count);
    free(data);
}

/** Multiply each per-category site likelihood with the weight ratio
 * (\f$L_{ci}' = L_{ci} \cdot \frac{w_{c,j+1}}{w_{c,j}}\f$) effectively apply
 * the new category weights. */
void rescale_sitecat_lh(corax_opt_multipart_em_data_t *data)
{
    for (unsigned int p = 0; p < data->treeinfo->partition_count; ++p) {
        corax_partition_t *part = data->treeinfo->partitions[p];
        if (!part || data->converged[p] || !(data->treeinfo->params_to_optimize[p] & CORAX_OPT_PARAM_RATE_WEIGHTS))
            continue;

        double *this_lk_cat = data->sitecat_lh_per_part[p];

        for (unsigned int i = 0; i < part->sites; ++i) {
            for (unsigned int c = 0; c < part->rate_cats; ++c) {
                this_lk_cat[c] *= data->weight_ratio[data->prefix_sum_category_count[p] + c];
            }

            this_lk_cat += part->rate_cats;
        }
    }
}

/** For each category, sum the ratios of the per-category likelihood and site
 * likelihood (\f$\sum_{i=1}^n \frac{L_{ci}}{L_i}\f$) */
void sum_local_category_likelihood_proportion(corax_opt_multipart_em_data_t *data)
{
    for (unsigned int p = 0; p < data->treeinfo->partition_count; ++p) {
        corax_partition_t *part = data->treeinfo->partitions[p];

        if (!part || data->converged[p] || !(data->treeinfo->params_to_optimize[p] & CORAX_OPT_PARAM_RATE_WEIGHTS))
            continue;

        double *this_lk_cat = data->sitecat_lh_per_part[p];
        const unsigned partition_offset = data->prefix_sum_category_count[p];

        for (unsigned int i = 0; i < part->sites; ++i) {
            double pattern_likelihood = 0;
            const unsigned int pattern_weight = part->pattern_weights[i];

            for (unsigned int c = 0; c < part->rate_cats; ++c) {
                pattern_likelihood += this_lk_cat[c];
            }

            for (unsigned int c = 0; c < part->rate_cats; ++c) {
                data->new_weights[partition_offset + c] += 
                    pattern_weight * this_lk_cat[c] / pattern_likelihood;
            }

            this_lk_cat += part->rate_cats;
        }
    }
}

CORAX_EXPORT void
corax_opt_minimize_em_multipartition(corax_opt_multipart_em_data_t *data) {

    unsigned int max_steps = 10;
    bool ratio_scale = false;
    bool all_partitions_converged = false;
    const unsigned int overall_category_count = data->total_rate_cats;

    for (unsigned int p = 0; p < data->treeinfo->partition_count; ++p)
    {
        if (data->treeinfo->params_to_optimize[p] & CORAX_OPT_PARAM_RATE_WEIGHTS) {
            data->converged[p] = false;
        } else {
            data->converged[p] = true;
        }
    }

    while (!all_partitions_converged && max_steps--)
    {
        DBG("corax_opt_minimize_em_multipartition it %i\n", 10 - max_steps);

        if (ratio_scale)
        {
            rescale_sitecat_lh(data);
        }
        else
        {
            ratio_scale = true;
        }

        memset(data->new_weights, 0, sizeof(double) * overall_category_count);

        // Expectation step
        sum_local_category_likelihood_proportion(data);
        corax_treeinfo_parallel_reduce(data->treeinfo,
                data->new_weights, overall_category_count, CORAX_REDUCE_SUM);

        all_partitions_converged = true;

        for (unsigned int p = 0; p < data->treeinfo->partition_count; ++p) {
            DBG("\tpart %i ", p);
            if (data->converged[p] || !(data->treeinfo->params_to_optimize[p] & CORAX_OPT_PARAM_RATE_WEIGHTS)) {
                DBG("converged\n");
                continue;
            }

            bool partition_converged = true;
            const unsigned int category_count = data->prefix_sum_category_count[p + 1] - data->prefix_sum_category_count[p];
            const unsigned partition_offset = data->prefix_sum_category_count[p];

            // Maximization step
            for (unsigned int c = 0; c < category_count; c++) {
                const unsigned int c_idx = partition_offset + c;
                data->new_weights[c_idx] /= data->pattern_weight_sum_per_part[p];

                partition_converged = partition_converged && fabs(data->weights[c_idx] - data->new_weights[c_idx]) < 1e-4;
                data->weight_ratio[c_idx] = data->new_weights[c_idx] / data->weights[c_idx];
                data->weights[c_idx] = data->new_weights[c_idx];
                DBG(" %f ", data->weights[c_idx]);
            }
            DBG("\n");

            // Copy weights to partition if it is local
            corax_partition_t *part = data->treeinfo->partitions[p];
            if (part) {
                memcpy(part->rate_weights, &data->weights[data->prefix_sum_category_count[p]], sizeof(double) * part->rate_cats);
            }

            data->converged[p] = partition_converged;
            all_partitions_converged = all_partitions_converged && data->converged[p];
        }
    }
}
