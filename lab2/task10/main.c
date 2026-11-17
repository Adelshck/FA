#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "func.h"
#include "status.h"

static void print_status(const status st) {
    switch (st) {
        case SUCCESS:
            printf("SUCCESS\n");
            break;
        case ERROR_NULL_POINTER:
            printf("Error: Passed NULL pointer.\n");
            break;
        case ERROR_INVALID_DEGREE:
            printf("Error: Invalid polynomial degree.\n");
            break;
        case ERROR_INVALID_EPS:
            printf("Error: Invalid eps value.\n");
            break;
        case ERROR_INVALID_VALUE:
            printf("Error: Invalid value (NaN or infinity).\n");
            break;
        case ERROR_MEMORY_ALLOCATION:
            printf("Error: Memory allocation failed.\n");
            break;
        default:
            printf("Error: Unknown error code.\n");
            break;
    }
}

static void print_report(const char *const title, const status st, const double *const g, const int n) {
    printf("=== %s ===\n", title);
    print_status(st);
    if (st == SUCCESS && g != NULL) {
        for (int i = 0; i <= n; ++i) {
            printf("g[%d] = %g\n", i, g[i]);
        }
    }
    printf("\n");
}

int main(void) {
    const double eps = 1e-9;
    double *result = NULL;
    status st;

    st = reexpand_polynomial(eps, 2.0, &result, 3, 1.0, -3.0, 3.0, -1.0);
    print_report("Test 1: 1 - 3x + 3x^2 - x^3, a = 2 (expected -1 -3 -3 -1)", st, result, 3);
    free(result);
    result = NULL;

    st = reexpand_polynomial(eps, 5.0, &result, 0, 7.0);
    print_report("Test 2: constant 7, a = 5 (expected 7)", st, result, 0);
    free(result);
    result = NULL;

    st = reexpand_polynomial(eps, 1.0, &result, 2, 1.0, 2.0, 1.0);
    print_report("Test 3: 1 + 2x + x^2, a = 1 (expected 4 4 1)", st, result, 2);
    free(result);
    result = NULL;

    st = reexpand_polynomial(eps, 1.0, &result, 2, 1.0, -2.0, 1.0);
    print_report("Test 4: 1 - 2x + x^2, a = 1 (expected 0 0 1)", st, result, 2);
    free(result);
    result = NULL;

    st = reexpand_polynomial(eps, 0.0, &result, 2, 1.0, 2.0, 3.0);
    print_report("Test 5: 1 + 2x + 3x^2, a = 0 (expected 1 2 3)", st, result, 2);
    free(result);
    result = NULL;

    st = reexpand_polynomial(eps, -1.5, &result, 4, 0.0, 0.0, 0.0, 0.0, 2.0);
    print_report("Test 6: 2x^4, a = -1.5", st, result, 4);
    free(result);
    result = NULL;

    st = reexpand_polynomial(eps, 2.0, NULL, 2, 1.0, 2.0, 3.0);
    print_report("Error test 1: result pointer is NULL", st, NULL, 0);

    st = reexpand_polynomial(eps, 2.0, &result, -1);
    print_report("Error test 2: negative degree", st, result, 0);
    free(result);
    result = NULL;

    st = reexpand_polynomial(eps, 2.0, &result, INT_MAX);
    print_report("Error test 3: degree INT_MAX", st, result, 0);
    free(result);
    result = NULL;

    st = reexpand_polynomial(0.0, 2.0, &result, 1, 1.0, 2.0);
    print_report("Error test 4: eps = 0", st, result, 1);
    free(result);
    result = NULL;

    st = reexpand_polynomial(-1e-9, 2.0, &result, 1, 1.0, 2.0);
    print_report("Error test 5: eps < 0", st, result, 1);
    free(result);
    result = NULL;

    st = reexpand_polynomial(eps, NAN, &result, 1, 1.0, 2.0);
    print_report("Error test 6: a = NaN", st, result, 1);
    free(result);
    result = NULL;

    st = reexpand_polynomial(eps, 2.0, &result, 2, 1.0, INFINITY, 3.0);
    print_report("Error test 7: coefficient = infinity", st, result, 2);
    free(result);
    result = NULL;

    return 0;
}