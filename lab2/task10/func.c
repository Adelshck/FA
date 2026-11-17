#include "func.h"
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

status reexpand_polynomial(const double eps, const double a, double **const result_coeffs, const int n, ...) {
    if (result_coeffs == NULL) {
        return ERROR_NULL_POINTER;
    }
    *result_coeffs = NULL;

    if (n < 0 || n == INT_MAX) {
        return ERROR_INVALID_DEGREE;
    }
    if (!isfinite(eps) || eps <= 0.0) {
        return ERROR_INVALID_EPS;
    }
    if (!isfinite(a)) {
        return ERROR_INVALID_VALUE;
    }

    double *coeffs = (double *)malloc((size_t)(n + 1) * sizeof(double));
    if (coeffs == NULL) {
        return ERROR_MEMORY_ALLOCATION;
    }
    double *g = (double *)malloc((size_t)(n + 1) * sizeof(double));
    if (g == NULL) {
        free(coeffs);
        return ERROR_MEMORY_ALLOCATION;
    }

    va_list args;
    va_start(args, n);
    for (int i = 0; i <= n; ++i) {
        coeffs[i] = va_arg(args, double);
    }
    va_end(args);

    for (int i = 0; i <= n; ++i) {
        if (!isfinite(coeffs[i])) {
            free(coeffs);
            free(g);
            return ERROR_INVALID_VALUE;
        }
    }

    for (int k = 0; k <= n; ++k) {
        for (int i = n - 1; i >= k; --i) {
            coeffs[i] += a * coeffs[i + 1];
        }
        g[k] = (fabs(coeffs[k]) < eps) ? 0.0 : coeffs[k];
    }

    free(coeffs);
    *result_coeffs = g;
    return SUCCESS;
}