/*
    Copyright (C) 2016 Diego Darriba

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Contact: Diego Darriba <Diego.Darriba@h-its.org>,
    Exelixis Lab, Heidelberg Instutute for Theoretical Studies
    Schloss-Wolfsbrunnenweg 35, D-69118 Heidelberg, Germany
*/

#ifndef CORAX_OPTIMIZE_TREEINFO_H_
#define CORAX_OPTIMIZE_TREEINFO_H_

#include "corax/tree/treeinfo.h"
#include "opt_generic.h"

typedef struct cutoff_info
{
  double lh_start;
  double lh_cutoff;
  double lh_dec_sum;
  int    lh_dec_count;
} cutoff_info_t;

typedef int (*treeinfo_param_set_cb)(corax_treeinfo_t *treeinfo,
                                     unsigned int      part_num,
                                     const double *    param_vals,
                                     unsigned int      param_count);

typedef int (*treeinfo_param_get_cb)(const corax_treeinfo_t *treeinfo,
                                     unsigned int            part_num,
                                     double *                param_vals,
                                     unsigned int            param_count);



#ifdef __cplusplus
extern "C"
{
#endif

  /* functions to optimize multiple partitions in parallel, using treeinfo struct
   */

  CORAX_EXPORT double corax_algo_opt_onedim_treeinfo(corax_treeinfo_t *treeinfo,
                                                     int    param_to_optimize,
                                                     double min_value,
                                                     double max_value,
                                                     double tolerance);

  CORAX_EXPORT
  double
  corax_algo_opt_onedim_treeinfo_custom(corax_treeinfo_t *    treeinfo,
                                        int                   param_to_optimize,
                                        treeinfo_param_get_cb params_getter,
                                        treeinfo_param_set_cb params_setter,
                                        double                min_value,
                                        double                max_value,
                                        double                tolerance);

  /** @defgroup treeinfo_opt_algs Treeinfo Optimization Algorithms
   * These functions more or less have the same arguments and optimize their
   * particular parameter. They all use similar algorithms.
   *
   * @param bfgs_factor: A value that should be greater than 0. An opaque
   * quantity, but typical values are 1e12 for inaccurate but fast searches, or
   * 1e1 for a more complete and slow search.
   *
   * @param tolerance This controls the pgtol threshold. If the largest entry of
   * the projected gradient is larger than this value, optimization will stop.
   *
   * @ingroup corax_treeinfo_t
   *
   * @{
   */
  CORAX_EXPORT
  double corax_algo_opt_subst_rates_treeinfo(corax_treeinfo_t *treeinfo,
                                             unsigned int      params_index,
                                             double            min_rate,
                                             double            max_rate,
                                             double            bfgs_factor,
                                             double            tolerance);

  CORAX_EXPORT
  double corax_algo_opt_frequencies_treeinfo(corax_treeinfo_t *treeinfo,
                                             unsigned int      params_index,
                                             double            min_freq,
                                             double            max_freq,
                                             double            bfgs_factor,
                                             double            tolerance);

  CORAX_EXPORT
  double corax_algo_opt_rates_weights_treeinfo(corax_treeinfo_t *treeinfo,
                                               double            min_rate,
                                               double            max_rate,
                                               double            min_brlen,
                                               double            max_brlen,
                                               double            bfgs_factor,
                                               double            tolerance);

  CORAX_EXPORT
  double corax_algo_opt_alpha_pinv_treeinfo(corax_treeinfo_t *treeinfo,
                                            unsigned int      params_index,
                                            double            min_alpha,
                                            double            max_alpha,
                                            double            min_pinv,
                                            double            max_pinv,
                                            double            bfgs_factor,
                                            double            tolerance);

  CORAX_EXPORT
  double corax_algo_opt_brlen_scalers_treeinfo(corax_treeinfo_t *treeinfo,
                                               double            min_scaler,
                                               double            max_scaler,
                                               double            min_brlen,
                                               double            max_brlen,
                                               double            lh_epsilon);

  CORAX_EXPORT
  double corax_algo_opt_brlen_treeinfo(corax_treeinfo_t *treeinfo,
                                       double            min_brlen,
                                       double            max_brlen,
                                       double            lh_epsilon,
                                       int               max_iters,
                                       int               opt_method,
                                       int               radius);
  /** @} */

  /* search */

  /**
   * Perform an SPR round
   *
   * @param brlen_opt_method The optimization method to use when optimizing branch
   * lengths. Options are:
   * - CORAX_OPT_BLO_NEWTON_FAST: Standard.
   * - CORAX_OPT_BLO_NEWTON_SAFE: Adds a per branch likelihood check.
   * - CORAX_OPT_BLO_NEWTON_FALLBACK: Starts fast, but fallback to safe.
   * - CORAX_OPT_BLO_NEWTON_GLOBAL: Newton, but with additional searches to find
   *   more optima
   * - CORAX_OPT_BLO_NEWTON_OLDFAST
   * - CORAX_OPT_BLO_NEWTON_OLDSAFE
   *
   * @param smoothings: Maximum number of iterations for branch length optimization.
   * Negative = no limit (iterate until LH improvement < epsilon)
   *
   * @epsilon: Likelihood threshold to terminate the optimization. Also known as
   * the tolerance.
   *
   * @param[out] cutoff_info A struct that contains subtree descent cutoff information. It
   * is in/out parameter since cutoff info has to be preserved between subsequent SPR rounds.
   *
   * @subtree_cutoff relative likelihood cutoff for descending into subtrees. A larger value means
   * higher cutoff, i.e. deeper descent into subtrees.
   *
   */
  CORAX_EXPORT double corax_algo_spr_round(corax_treeinfo_t *treeinfo,
                                           unsigned int      radius_min,
                                           unsigned int      radius_max,
                                           unsigned int      ntopol_keep,
                                           corax_bool_t      thorough,
                                           int               brlen_opt_method,
                                           double            bl_min,
                                           double            bl_max,
                                           int               smoothings,
                                           double            epsilon,
                                           cutoff_info_t *   cutoff_info,
                                           double            subtree_cutoff);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* CORAX_OPTIMIZE_TREEINFO_H_ */
