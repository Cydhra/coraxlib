#include "au.h"

#include "bootstrap.h"
#include "stats.h"

/**
 * The maximum number of iterations for the AU test convergence test until non-convergence is reported. Each iteration
 * adds one WLS computation and one Newton optimization.
 */
const unsigned int CONVERGENCE_ITERATIONS = 100;

/**
 * The maximum number of iterations for the AU test's newton optimization.
 */
const unsigned int NEWTON_ITERATIONS = 30;

/**
 * Epsilon used for zero-checks in BP and AU tests
 */
const double EPS = 1E-16;

/**
 * Transform an observed Bootstrap Proportion into the linear parameter of the observed distribution (i.e., the
 * signed distance with curvature correction).
 */
double compute_distance(const double bp) {
    return -normal_quantile(bp);
}

/**
 * Solve the Weighted Least Squares system for a set of observed BP values and the model BP distribution analytically.
 * The function minimizes the loss between observed and the expected distance under normally distributed BP
 * values and uses those values to obtain estimators for the signed distance and curvature parameters.
 *
 * @param bootstrap_counts observed bootstrap counts at each scale
 * @param scales scaling factors for the multiscale bootstrap
 * @param scale_roots pre-calculate square roots of the scaling factors
 * @param num_replicates number of replicates per scaling factor
 * @param num_scales number of scaling factors in the array
 * @param df out-parameter for degrees of freedom encountered during estimation. If below 2, estimation is degenerate and
 *           an error is returned.
 * @param d out-parameter for the estimator for d
 * @param c out-parameter for the estimator for c
 * @return CORAX_SUCCESS if the WLS system was solved, CORAX_FAILURE if there is no analytical solution. Note that the
 * parameters d and c are initialized in both cases and can still be used as start parameters to a newton optimization of
 * c and d.
 */
int fit_parameters_wls(const double *const bootstrap_counts,
                       const double *const scales,
                       const double *const scale_roots,
                       const unsigned int *const num_replicates,
                       const unsigned int num_scales,
                       int *df,
                       double *d, double *c) {
    double *alloc = malloc(sizeof(double) * num_scales * 2);
    if (!alloc) {
        // TODO proper handling
        exit(-1);
    }

    double *observed_distances = alloc;
    double *weights = alloc + num_scales;

    *df = 0;
    for (unsigned int s = 0; s < num_scales; s++) {
        const double proportion = bootstrap_counts[s] / (double) num_replicates[s];
        if (proportion < EPS) {
            weights[s] = 0.0;
            observed_distances[s] = 0.0;
            continue;
        }

        observed_distances[s] = compute_distance(proportion);
        const double x = normal_pdf(observed_distances[s], 0.0, 1.0);
        weights[s] = (x * x * (double) num_replicates[s]) / ((1.0 - proportion) * proportion);
        *df += 1;
    }

    if (*df < 2) {
        *d = 0;
        *c = 0;
        goto clean_fail;
    }

    // compute model matrix
    double model_d = 0.0, model_c = 0.0, off_diagonal = 0.0;
    for (unsigned int s = 0; s < num_scales; s++) {
        model_d += weights[s] * scales[s];
        model_c += weights[s] / scales[s];
        off_diagonal += weights[s];
    }

    // compute observations
    double observed_d = 0.0, observed_c = 0.0;
    for (unsigned int s = 0; s < num_scales; s++) {
        observed_d += weights[s] * scale_roots[s] * observed_distances[s];
        observed_c += weights[s] / scale_roots[s] * observed_distances[s];
    }

    // invert model matrix
    double determinant = model_d * model_c - off_diagonal * off_diagonal;
    if (fabs(off_diagonal) == 0.0) {
        *d = 0.0;
        *c = 0.0;
        goto clean_fail;
    }

    // compute vector product of inverse matrix with observations to solve the system
    *d = (model_c * observed_d - off_diagonal * observed_c) / determinant;
    *c = (-off_diagonal * observed_d + model_d * observed_c) / determinant;

    free(alloc);
    return CORAX_SUCCESS;

clean_fail:
    free(alloc);
    return CORAX_FAILURE;
}

/**
 * One instance of the Newton-Raphson optimizer used in the AU test.
 * It optimizes a two-dimensional parameter, and is thus different from the standard optimizer in coraxlib.
 */
typedef struct _NewtonOptimizer {
    const double *bootstrap_counts;
    const double *scales;
    const double *scale_roots;
    const unsigned int *num_replicates;
    unsigned int num_scales;
} NewtonOptimizer;

/**
 * Get the likelihood in the AU model.
 *
 * @param d signed distance estimate
 * @param c curvature parameter estimate
 * @param scale_root square root of the current scaling factor
 * @return likelihood of the AU test model
 */
double likelihood(const double d, const double c, const double scale_root) {
    return normal_cdf(-(d * scale_root + c / scale_root), 0.0, 1.0);
}

/**
 * Calculate the gradient of the AU model's likelihood function at (d, c).
 *
 * @param instance current newton instance of the AU test
 * @param d current estimate for d
 * @param c current estimate for c
 * @param grad_d out-parameter for the partial derivative for d
 * @param grad_c out-parameter for the partial derivative for c
 * @param df degrees of freedom, i.e., how many summands are not zero
 */
void gradient(const NewtonOptimizer *const instance, const double d, const double c, double *grad_d, double *grad_c,
              int *df) {
    *grad_d = 0.0;
    *grad_c = 0.0;
    *df = 0;

    for (unsigned int s = 0; s < instance->num_scales; s++) {
        const double root = instance->scale_roots[s];
        const double pi = likelihood(d, c, root);
        if (pi > 0.0 && pi < 1.0) {
            const double derivative = -normal_pdf(d * root + c / root, 0.0, 1.0)
                                      * (instance->bootstrap_counts[s] - (double) instance->num_replicates[s] * pi)
                                      / (pi * (1.0 - pi));
            *grad_d += derivative * root;
            *grad_c += derivative / root;
            *df += 1;
        } else {
            // prevent division by zero and similar numerical issues
            // ReSharper disable once CppRedundantControlFlowJump
            continue;
        }
    }
}

/**
 * Calculate the hessian  of the AU model's likelihood function at (d, c).
 *
 * @param instance current newton instance of the AU test
 * @param d current estimate for d
 * @param c current estimate for c
 * @param hess_dd out-parameter for the partial derivative with respect to d squared
 * @param hess_cd out-parameter for the partial derivative with respect to c and d
 * @param hess_cc out-parameter for the partial derivative with respect to c squared
 */
void hessian(const NewtonOptimizer *const instance, const double d, const double c,
             double *hess_dd, double *hess_cd, double *hess_cc) {
    *hess_cc = 0.0;
    *hess_cd = 0.0;
    *hess_dd = 0.0;

    for (unsigned int s = 0; s < instance->num_scales; s++) {
        const double root = instance->scale_roots[s];
        const double pi = likelihood(d, c, root);
        if (pi > 0.0 && pi < 1.0) {
            const double linear = d * root + c / root;
            const double density = normal_pdf(linear, 0.0, 1.0);
            const double count = instance->bootstrap_counts[s];
            const double num_replicates = instance->num_replicates[s];

            const double derivative = density * (density * (-count + 2.0 * count * pi - num_replicates * pi * pi)
                                                 / pi / pi / (1.0 - pi) / (1.0 - pi)
                                                 + linear * (count - num_replicates * pi) / (pi * (1.0 - pi)));

            *hess_dd += derivative * instance->scales[s];
            *hess_cd += derivative;
            *hess_cc += derivative / instance->scales[s];
        } else {
            // prevent division by zero and other numerical issues
            // ReSharper disable once CppRedundantControlFlowJump
            continue;
        }
    }
}

/**
 * Newton-Raphson optimization for the signed distance and curvature parameters of the AU-test.
 * We don't use corax' Newton-Raphson optimizer for this, because we have to optimize two parameters at once.
 *
* @param instance current newton instance of the AU test
 * @param d out-parameter for the signed distance, initialized with the initial guess
 * @param c out-parameter for the curvature, initialized with the initial guess
 * @param error out-parameter for the standard error of the estimator
 * @param p_value out-parameter for the final p-value of the AU test with the current parameters
 * @param df out-parameter for the degrees of freedom during analysis. If this drops below 0.0, the problem is
 *           degenerate.
 */
void fit_parameters_newton(const NewtonOptimizer *const instance,
                           double *d, double *c, double *error, double *p_value, int *df) {
    double prev_d = *d, prev_c = *c;
    int prev_df = 0;

    // inverse hessian
    double inv_0 = 0.0, inv_12 = 0.0, inv_3 = 0.0;

    for (unsigned int it = 0; it < NEWTON_ITERATIONS; it++) {
        double grad_d, grad_c, hess_dd, hess_cd, hess_cc;
        gradient(instance, *d, *c, &grad_d, &grad_c, df);
        hessian(instance, *d, *c, &hess_dd, &hess_cd, &hess_cc);

        const double determinant = hess_dd * hess_cc - hess_cd * hess_cd;
        if (fabs(determinant) == 0.0) {
            // hessian is singular. We interpret this as convergence, despite it being mathematically unclear,
            // because the convergence test should fail earlier if the function is degenerate.

            // revert to previous parameters to make the fisher-information (inverse hessian) well-defined.
            // we won't update the inverse hessian from last iteration.
            *d = prev_d;
            *c = prev_c;
            *df = prev_df;
            break;
        }

        // invert hessian
        inv_0 = hess_cc / determinant;
        inv_12 = -hess_cd / determinant;
        inv_3 = hess_dd / determinant;

        // calculate newton step
        const double update_d = inv_0 * grad_d + inv_12 * grad_c;
        const double update_c = inv_12 * grad_d + inv_3 * grad_c;

        prev_d = *d;
        prev_c = *c;
        prev_df = *df;

        *d -= update_d;
        *c -= update_c;
    }

    const double deriv = normal_pdf(*d - *c, 0.0, 1.0);
    *error = sqrt(deriv * deriv * (-inv_0 - inv_3 + inv_12 + inv_12));
    *p_value = normal_cdf(-(*d - *c), 0.0, 1.0);
    *df -= 2; // subtract two degrees that we need to estimate c and d.
}

CORAX_EXPORT int corax_au_p_value(double **const replicates,
                                  const unsigned int tree,
                                  const double *const scales,
                                  const unsigned int *const num_replicates,
                                  const unsigned int num_scales,
                                  const double initial_threshold,
                                  double *d, double *c, double *p_value) {
    double *alloc = malloc(sizeof(double) * num_scales * 2);
    double *counts = alloc;
    double *roots = alloc + num_scales;

    if (!counts || !roots) {
        // TODO proper handling
        exit(-1);
    }

    // prepare roots of scales so we don't need to compute them as often
    for (unsigned int s = 0; s < num_scales; s++) {
        roots[s] = sqrt(scales[s]);
    }

    double threshold = initial_threshold, last_threshold = 0.0, target_threshold = 0.0;
    double error, last_error = 0.0, last_p_value = 0.0;
    int df, last_df = 0;

    // initialize optimizer
    NewtonOptimizer optimizer;
    optimizer.scales = scales;
    optimizer.scale_roots = roots;
    optimizer.num_replicates = num_replicates;
    optimizer.num_scales = num_scales;

    for (unsigned int loops = 0; loops < CONVERGENCE_ITERATIONS; loops++) {
        // obtain smoothed bootstrap counts for current iteration
        for (unsigned int s = 0; s < num_scales; s++) {
            counts[s] = corax_empirical_bootstrap_count(replicates[s], num_replicates[s], tree, threshold);
        }

        if (fit_parameters_wls(counts, scales, roots, num_replicates, num_scales, &df, d, c) == CORAX_SUCCESS) {
            optimizer.bootstrap_counts = counts;

            fit_parameters_newton(&optimizer, d, c, &error, p_value, &df);
        } else {
            // p value and error stay the same as in previous iteration, because we ran into an error condition
            // (which is df < 2)
        }

        // check whether the optimization problem is unsolvable, or
        if (df < 2
            // whether the p value is decreasing despite the threshold also decreasing
            || ((last_p_value - *p_value) * (threshold - last_threshold) > 0.0
                // while the change in p-value is significant (i.e., larger than standard error)
                && fabs(*p_value - last_p_value) > 0.1 * last_error
                // and the function was fine before
                && last_df >= 2)) {
            // turn back a bit toward the previous threshold and prevent the threshold
            // from crossing the non-monotone region again
            target_threshold = threshold;
            threshold = 0.5 * threshold + 0.5 * last_threshold;
            continue;
        }

        // update threshold for well-definedness check next iteration
        last_threshold = threshold;

        // check whether change in p-value is much smaller than expected error
        if (fabs(last_p_value - *p_value) < 0.01 * last_error) {
            // we have reached convergence of the p-value
            goto converged;
        }

        // approach target threshold
        threshold = 0.5 * threshold + 0.5 * target_threshold;

        // update remaining parameters for checks next iteration
        last_p_value = *p_value;
        last_error = error;
        last_df = df;

        // check whether we have reached the target threshold
        if (fabs(threshold - target_threshold) < EPS) {
            goto converged;
        }
    }

    // if we are here, convergence has not been reached.
    free(alloc);
    return CORAX_FAILURE;

converged:
    free(alloc);
    return CORAX_SUCCESS;
}
