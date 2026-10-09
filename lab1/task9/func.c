#include "func.h"
#include <stdlib.h>

status rdm_arr_mn_mx(const long a, const long b, long* nums, long size_nums) {
    if (nums == NULL || size_nums <= 0 || a > b) {
        return ERROR_INVALID_ARGUMENTS;
    }

    unsigned long long range = (unsigned long long)b - (unsigned long long)a + 1ULL;

    long idx_mn = 0;
    long idx_mx = 0;

    for (long i = 0; i < size_nums; i++) {
        long num = a + (long)(rand() % range);
        nums[i] = num;

        if (i == 0) {
            idx_mn = 0;
            idx_mx = 0;
        } else {
            if (num < nums[idx_mn]) {
                idx_mn = i;
            }
            if (num > nums[idx_mx]) {
                idx_mx = i;
            }
        }
    }

    long temp = nums[idx_mn];
    nums[idx_mn] = nums[idx_mx];
    nums[idx_mx] = temp;

    return SUCCESS;
}

static long find_closest_val(long val, const long* b, long size_b) {
    long closest = b[0];
    long min_diff = labs(val - b[0]);

    for (long i = 1; i < size_b; i++) {
        long current_diff = labs(val - b[i]);
        if (current_diff < min_diff) {
            min_diff = current_diff;
            closest = b[i];
        }
    }
    return closest;
}

status create_arrays_a_b_c(long** out_a, long* out_size_a, long** out_b, long* out_size_b, long** out_c) {
    if (out_a == NULL || out_size_a == NULL ||
        out_b == NULL || out_size_b == NULL ||
        out_c == NULL) {
        return ERROR_INVALID_ARGUMENTS;
    }

    long size_a = 10 + rand() % (10000 - 10 + 1);
    long size_b = 10 + rand() % (10000 - 10 + 1);

    long* a = (long*)malloc(sizeof(long) * size_a);
    if (a == NULL) {
        return ERROR_MEMORY_ALLOCATION;
    }

    long* b = (long*)malloc(sizeof(long) * size_b);
    if (b == NULL) {
        free(a);
        return ERROR_MEMORY_ALLOCATION;
    }

    long* c = (long*)malloc(sizeof(long) * size_a);
    if (c == NULL) {
        free(a);
        free(b);
        return ERROR_MEMORY_ALLOCATION;
    }

    for (long i = 0; i < size_a; i++) {
        a[i] = -1000 + rand() % (1000 - (-1000) + 1);
    }
    for (long i = 0; i < size_b; i++) {
        b[i] = -1000 + rand() % (1000 - (-1000) + 1);
    }

    for (long i = 0; i < size_a; i++) {
        long closest = find_closest_val(a[i], b, size_b);
        c[i] = a[i] + closest;
    }

    *out_a = a;
    *out_b = b;
    *out_c = c;
    *out_size_a = size_a;
    *out_size_b = size_b;

    return SUCCESS;
}