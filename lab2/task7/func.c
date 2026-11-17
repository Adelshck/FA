#include "func.h"
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>

status create_vector(int dim, const double *coords, Vector *out_vec) {
    if (dim <= 0 || !coords || !out_vec) {
        return ERROR_INVALID_ARGUMENT;
    }
    double *buf = (double *)malloc((size_t)dim * sizeof(double));
    if (!buf) {
        return ERROR_OUT_OF_MEMORY;
    }
    for (int i = 0; i < dim; ++i) {
        buf[i] = coords[i];
    }
    out_vec->coords = buf;
    out_vec->dim = dim;
    return SUCCESS;
}

void free_vector(Vector *vec) {
    if (vec) {
        free(vec->coords);
        vec->coords = NULL;
        vec->dim = 0;
    }
}

static double leading_minor(const double *const *data, int k, double epsilon, double *work) {
    for (int i = 0; i < k; ++i) {
        for (int j = 0; j < k; ++j) {
            work[i * k + j] = data[i][j];
        }
    }
    double det = 1.0;
    for (int col = 0; col < k; ++col) {
        int pivot = col;
        for (int row = col + 1; row < k; ++row) {
            if (fabs(work[row * k + col]) > fabs(work[pivot * k + col])) {
                pivot = row;
            }
        }
        if (fabs(work[pivot * k + col]) <= epsilon) {
            return 0.0;
        }
        if (pivot != col) {
            for (int j = 0; j < k; ++j) {
                double tmp = work[col * k + j];
                work[col * k + j] = work[pivot * k + j];
                work[pivot * k + j] = tmp;
            }
            det = -det;
        }
        det *= work[col * k + col];
        for (int row = col + 1; row < k; ++row) {
            double factor = work[row * k + col] / work[col * k + col];
            for (int j = col; j < k; ++j) {
                work[row * k + j] -= factor * work[col * k + j];
            }
        }
    }
    return det;
}

static status check_positive_definite(const double *const *data, int dim, double epsilon) {
    for (int i = 0; i < dim; ++i) {
        for (int j = i + 1; j < dim; ++j) {
            if (fabs(data[i][j] - data[j][i]) > epsilon) {
                return ERROR_MATRIX_NOT_POSITIVE_DEFINITE;
            }
        }
    }
    double *work = (double *)malloc((size_t)dim * (size_t)dim * sizeof(double));
    if (!work) {
        return ERROR_OUT_OF_MEMORY;
    }
    for (int k = 1; k <= dim; ++k) {
        if (leading_minor(data, k, epsilon, work) <= epsilon) {
            free(work);
            return ERROR_MATRIX_NOT_POSITIVE_DEFINITE;
        }
    }
    free(work);
    return SUCCESS;
}

status create_matrix(int dim, const double *const *data, double epsilon, Matrix *out_mat) {
    if (dim <= 0 || !data || !out_mat || epsilon <= 0.0) {
        return ERROR_INVALID_ARGUMENT;
    }
    for (int i = 0; i < dim; ++i) {
        if (!data[i]) {
            return ERROR_INVALID_ARGUMENT;
        }
    }
    status st = check_positive_definite(data, dim, epsilon);
    if (st != SUCCESS) {
        return st;
    }
    double **rows = (double **)calloc((size_t)dim, sizeof(double *));
    if (!rows) {
        return ERROR_OUT_OF_MEMORY;
    }
    for (int i = 0; i < dim; ++i) {
        rows[i] = (double *)malloc((size_t)dim * sizeof(double));
        if (!rows[i]) {
            for (int j = 0; j < i; ++j) {
                free(rows[j]);
            }
            free(rows);
            return ERROR_OUT_OF_MEMORY;
        }
        for (int j = 0; j < dim; ++j) {
            rows[i][j] = data[i][j];
        }
    }
    out_mat->data = rows;
    out_mat->dim = dim;
    return SUCCESS;
}

void free_matrix(Matrix *mat) {
    if (mat && mat->data) {
        for (int i = 0; i < mat->dim; ++i) {
            free(mat->data[i]);
        }
        free(mat->data);
    }
    if (mat) {
        mat->data = NULL;
        mat->dim = 0;
    }
}

status norm_infinity(const Vector *vec, const void *param, double epsilon, double *out_result) {
    (void)param;
    if (!vec || !vec->coords || vec->dim <= 0 || !out_result || epsilon <= 0.0) {
        return ERROR_INVALID_ARGUMENT;
    }
    double max_val = 0.0;
    for (int i = 0; i < vec->dim; ++i) {
        double a = fabs(vec->coords[i]);
        if (a > max_val) {
            max_val = a;
        }
    }
    *out_result = max_val;
    return SUCCESS;
}

status norm_lp(const Vector *vec, const void *param, double epsilon, double *out_result) {
    if (!vec || !vec->coords || vec->dim <= 0 || !param || !out_result || epsilon <= 0.0) {
        return ERROR_INVALID_ARGUMENT;
    }
    double p = *(const double *)param;
    if (p < 1.0 - epsilon) {
        return ERROR_INVALID_ARGUMENT;
    }
    double sum = 0.0;
    for (int i = 0; i < vec->dim; ++i) {
        sum += pow(fabs(vec->coords[i]), p);
    }
    *out_result = pow(sum, 1.0 / p);
    return SUCCESS;
}

status norm_matrix(const Vector *vec, const void *param, double epsilon, double *out_result) {
    const Matrix *mat = (const Matrix *)param;
    if (!vec || !vec->coords || vec->dim <= 0 || !mat || !mat->data || !out_result ||
        epsilon <= 0.0 || vec->dim != mat->dim) {
        return ERROR_INVALID_ARGUMENT;
    }
    double inner = 0.0;
    for (int i = 0; i < mat->dim; ++i) {
        double row = 0.0;
        for (int j = 0; j < mat->dim; ++j) {
            row += mat->data[i][j] * vec->coords[j];
        }
        inner += row * vec->coords[i];
    }
    if (inner < -epsilon) {
        return ERROR_MATRIX_NOT_POSITIVE_DEFINITE;
    }
    if (inner < 0.0) {
        inner = 0.0;
    }
    *out_result = sqrt(inner);
    return SUCCESS;
}

static void release_partial(Vector **res, int *counts, int filled) {
    if (res) {
        for (int k = 0; k < filled; ++k) {
            free(res[k]);
        }
    }
    free(res);
    free(counts);
}

static status compute_longest(int n,double epsilon,int num_vectors,const Vector *vectors,int num_norms,const NormFunc *funcs,
    const void *const *params, Vector ***out_results, int **out_counts ) {
    for (int j = 0; j < num_vectors; ++j) {
        if (vectors[j].dim != n || !vectors[j].coords) {
            return ERROR_INVALID_DIMENSION;
        }
    }
    Vector **res = (Vector **)calloc((size_t)num_norms, sizeof(Vector *));
    int *counts = (int *)calloc((size_t)num_norms, sizeof(int));
    double *vals = (double *)malloc((size_t)num_vectors * sizeof(double));
    if (!res || !counts || !vals) {
        free(vals);
        release_partial(res, counts, 0);
        return ERROR_OUT_OF_MEMORY;
    }
    for (int i = 0; i < num_norms; ++i) {
        for (int j = 0; j < num_vectors; ++j) {
            status st = funcs[i](&vectors[j], params[i], epsilon, &vals[j]);
            if (st != SUCCESS) {
                free(vals);
                release_partial(res, counts, i);
                return st;
            }
        }
        double max_norm = vals[0];
        for (int j = 1; j < num_vectors; ++j) {
            if (vals[j] > max_norm) {
                max_norm = vals[j];
            }
        }
        int count = 0;
        for (int j = 0; j < num_vectors; ++j) {
            if (fabs(vals[j] - max_norm) <= epsilon) {
                ++count;
            }
        }
        Vector *sub = (Vector *)malloc((size_t)count * sizeof(Vector));
        if (!sub) {
            free(vals);
            release_partial(res, counts, i);
            return ERROR_OUT_OF_MEMORY;
        }
        int idx = 0;
        for (int j = 0; j < num_vectors; ++j) {
            if (fabs(vals[j] - max_norm) <= epsilon) {
                sub[idx++] = vectors[j];
            }
        }
        res[i] = sub;
        counts[i] = count;
    }
    free(vals);
    *out_results = res;
    *out_counts = counts;
    return SUCCESS;
}

status find_longest_vectors(int n, double epsilon, int num_vectors, int num_norms, Vector ***out_results, int **out_counts, ...) {
    if (n <= 0 || epsilon <= 0.0 || num_vectors <= 0 || num_norms <= 0 || !out_results || !out_counts) {
        return ERROR_INVALID_ARGUMENT;
    }
    Vector *vectors = (Vector *)malloc((size_t)num_vectors * sizeof(Vector));
    NormFunc *funcs = (NormFunc *)malloc((size_t)num_norms * sizeof(NormFunc));
    const void **params = (const void **)malloc((size_t)num_norms * sizeof(const void *));
    if (!vectors || !funcs || !params) {
        free(vectors);
        free(funcs);
        free(params);
        return ERROR_OUT_OF_MEMORY;
    }

    va_list ap;
    va_start(ap, out_counts);
    for (int j = 0; j < num_vectors; ++j) {
        vectors[j] = va_arg(ap, Vector);
    }
    for (int i = 0; i < num_norms; ++i) {
        funcs[i] = va_arg(ap, NormFunc);
        params[i] = va_arg(ap, const void *);
        if (!funcs[i]) {
            va_end(ap);
            free(vectors);
            free(funcs);
            free(params);
            return ERROR_NULL_POINTER;
        }
    }
    va_end(ap);

    status st = compute_longest(n, epsilon, num_vectors, vectors, num_norms, funcs, params, out_results, out_counts);
    free(vectors);
    free(funcs);
    free(params);
    return st;
}