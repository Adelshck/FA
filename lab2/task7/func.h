#ifndef FUNC_H
#define FUNC_H

#include "status.h"

typedef struct {
    double *coords;
    int dim;
} Vector;

typedef struct {
    double **data;
    int dim;
} Matrix;

typedef status (*NormFunc)(const Vector *vec, const void *param, double epsilon, double *out_result);

status create_vector(int dim, const double *coords, Vector *out_vec);
void free_vector(Vector *vec);

status create_matrix(int dim, const double *const *data, double epsilon, Matrix *out_mat);
void free_matrix(Matrix *mat);

status norm_infinity(const Vector *vec, const void *param, double epsilon, double *out_result);
status norm_lp(const Vector *vec, const void *param, double epsilon, double *out_result);
status norm_matrix(const Vector *vec, const void *param, double epsilon, double *out_result);

status find_longest_vectors(int n, double epsilon, int num_vectors, int num_norms, Vector ***out_results, int **out_counts, ...);

#endif