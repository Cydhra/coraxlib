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

typedef int (*treeinfo_param_set_cb)(pllmod_treeinfo_t *treeinfo,
                                     unsigned int       part_num,
                                     const double *     param_vals,
                                     unsigned int       param_count);

typedef int (*treeinfo_param_get_cb)(const pllmod_treeinfo_t *treeinfo,
                                     unsigned int             part_num,
                                     double *                 param_vals,
                                     unsigned int             param_count);

/* functions to optimize multiple partitions in parallel, using treeinfo struct
 */

PLL_EXPORT double pllmod_algo_opt_onedim_treeinfo(pllmod_treeinfo_t *treeinfo,
                                                  int    param_to_optimize,
                                                  double min_value,
                                                  double max_value,
                                                  double tolerance);

PLL_EXPORT
double
pllmod_algo_opt_onedim_treeinfo_custom(pllmod_treeinfo_t *   treeinfo,
                                       int                   param_to_optimize,
                                       treeinfo_param_get_cb params_getter,
                                       treeinfo_param_set_cb params_setter,
                                       double                min_value,
                                       double                max_value,
                                       double                tolerance);

PLL_EXPORT
double pllmod_algo_opt_subst_rates_treeinfo(pllmod_treeinfo_t *treeinfo,
                                            unsigned int       params_index,
                                            double             min_rate,
                                            double             max_rate,
                                            double             bfgs_factor,
                                            double             tolerance);

PLL_EXPORT
double pllmod_algo_opt_frequencies_treeinfo(pllmod_treeinfo_t *treeinfo,
                                            unsigned int       params_index,
                                            double             min_freq,
                                            double             max_freq,
                                            double             bfgs_factor,
                                            double             tolerance);

PLL_EXPORT
double pllmod_algo_opt_rates_weights_treeinfo(pllmod_treeinfo_t *treeinfo,
                                              double             min_rate,
                                              double             max_rate,
                                              double             min_brlen,
                                              double             max_brlen,
                                              double             bfgs_factor,
                                              double             tolerance);

PLL_EXPORT
double pllmod_algo_opt_alpha_pinv_treeinfo(pllmod_treeinfo_t *treeinfo,
                                           unsigned int       params_index,
                                           double             min_alpha,
                                           double             max_alpha,
                                           double             min_pinv,
                                           double             max_pinv,
                                           double             bfgs_factor,
                                           double             tolerance);

PLL_EXPORT
double pllmod_algo_opt_brlen_scalers_treeinfo(pllmod_treeinfo_t *treeinfo,
                                              double             min_scaler,
                                              double             max_scaler,
                                              double             min_brlen,
                                              double             max_brlen,
                                              double             lh_epsilon);

PLL_EXPORT
double pllmod_algo_opt_brlen_treeinfo(pllmod_treeinfo_t *treeinfo,
                                      double             min_brlen,
                                      double             max_brlen,
                                      double             lh_epsilon,
                                      int                max_iters,
                                      int                opt_method,
                                      int                radius);

/* search */

PLL_EXPORT double pllmod_algo_spr_round(pllmod_treeinfo_t *treeinfo,
                                        unsigned int       radius_min,
                                        unsigned int       radius_max,
                                        unsigned int       ntopol_keep,
                                        pll_bool_t         thorough,
                                        int                brlen_opt_method,
                                        double             bl_min,
                                        double             bl_max,
                                        int                smoothings,
                                        double             epsilon,
                                        cutoff_info_t *    cutoff_info,
                                        double             subtree_cutoff);

#endif /* CORAX_OPTIMIZE_TREEINFO_H_ */
