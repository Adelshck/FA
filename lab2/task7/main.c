#include <stdio.h>
#include <stdlib.h>
#include "status.h"
#include "func.h"

static void print_error(status st) {
    switch (st) {
        case ERROR_OUT_OF_MEMORY:
            printf("Error: Out of memory.\n");
            break;
        case ERROR_INVALID_DIMENSION:
            printf("Error: Invalid dimension.\n");
            break;
        case ERROR_INVALID_ARGUMENT:
            printf("Error: Invalid argument.\n");
            break;
        case ERROR_NULL_POINTER:
            printf("Error: Null pointer.\n");
            break;
        case ERROR_MATRIX_NOT_POSITIVE_DEFINITE:
            printf("Error: Matrix is not positive definite.\n");
            break;
        default:
            printf("Error: Unknown status code.\n");
            break;
    }
}

int main(void) {
    int n = 2;
    double epsilon = 1e-9;
    double c1[] = {3.0, 4.0};
    double c2[] = {-5.0, 0.0};
    double c3[] = {3.0, 4.0};
    Vector v1 = {NULL, 0}, v2 = {NULL, 0}, v3 = {NULL, 0};
    Matrix mat = {NULL, 0};
    int rc = 1;

    status st = create_vector(n, c1, &v1);
    if (st == SUCCESS) {
        st = create_vector(n, c2, &v2);
    }
    if (st == SUCCESS) {
        st = create_vector(n, c3, &v3);
    }

    if (st == SUCCESS) {
        double m_data[2][2] = {{2.0, 0.0}, {0.0, 2.0}};
        const double *m_ptr[2] = {m_data[0], m_data[1]};
        st = create_matrix(2, m_ptr, epsilon, &mat);
    }

    if (st == SUCCESS) {
        double p = 2.0;
        Vector **results = NULL;
        int *counts = NULL;
        st = find_longest_vectors(
            n, epsilon, 3, 3, &results, &counts,
            v1, v2, v3,
            norm_infinity, (const void *)NULL,
            norm_lp, (const void *)&p,
            norm_matrix, (const void *)&mat
        );
        if (st == SUCCESS) {
            printf("Calculation successful.\n");
            for (int i = 0; i < 3; ++i) {
                printf("Norm %d found %d longest vector(s):\n", i + 1, counts[i]);
                for (int j = 0; j < counts[i]; ++j) {
                    printf("  [%f, %f]\n", results[i][j].coords[0], results[i][j].coords[1]);
                }
                free(results[i]);
            }
            free(results);
            free(counts);
            rc = 0;
        }
    }

    if (st != SUCCESS) {
        print_error(st);
    }

    free_matrix(&mat);
    free_vector(&v1);
    free_vector(&v2);
    free_vector(&v3);
    return rc;
}