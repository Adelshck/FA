#include "func.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void free_names(char **names, int count) {
    if (names == NULL) {
        return;
    }
    for (int i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);
}

status select_backup_files(const file_info_t *files, int n, int capacity, backup_result_t *result) {
    if (files == NULL || result == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (n <= 0 || capacity < 0) {
        return ERROR_INVALID_PARAM;
    }
    for (int i = 0; i < n; i++) {
        if (files[i].filename == NULL || files[i].size <= 0 || files[i].importance < 0) {
            return ERROR_INVALID_PARAM;
        }
    }

    size_t rows = (size_t)n;
    size_t cols = (size_t)capacity + 1;
    if (cols > SIZE_MAX / rows || cols > SIZE_MAX / sizeof(long long)) {
        return ERROR_MEMORY_ALLOCATION;
    }

    long long *dp = (long long *)calloc(cols, sizeof(long long));
    unsigned char *take = (unsigned char *)calloc(rows * cols, sizeof(unsigned char));
    unsigned char *selected = (unsigned char *)calloc(rows, sizeof(unsigned char));
    if (dp == NULL || take == NULL || selected == NULL) {
        free(dp);
        free(take);
        free(selected);
        return ERROR_MEMORY_ALLOCATION;
    }

    for (int i = 0; i < n; i++) {
        int sz = files[i].size;
        long long imp = files[i].importance;
        for (int w = capacity; w >= sz; w--) {
            long long cand = dp[w - sz] + imp;
            if (cand > dp[w]) {
                dp[w] = cand;
                take[(size_t)i * cols + (size_t)w] = 1;
            }
        }
    }

    long long total_imp = dp[capacity];
    int selected_count = 0;
    int total_sz = 0;
    int w_curr = capacity;
    for (int i = n - 1; i >= 0; i--) {
        if (take[(size_t)i * cols + (size_t)w_curr]) {
            selected[i] = 1;
            selected_count++;
            total_sz += files[i].size;
            w_curr -= files[i].size;
        }
    }

    char **chosen_names = NULL;
    if (selected_count > 0) {
        chosen_names = (char **)calloc((size_t)selected_count, sizeof(char *));
        if (chosen_names == NULL) {
            free(dp);
            free(take);
            free(selected);
            return ERROR_MEMORY_ALLOCATION;
        }
        int pos = 0;
        for (int i = 0; i < n; i++) {
            if (!selected[i]) {
                continue;
            }
            size_t len = strlen(files[i].filename);
            chosen_names[pos] = (char *)malloc(len + 1);
            if (chosen_names[pos] == NULL) {
                free_names(chosen_names, pos);
                free(dp);
                free(take);
                free(selected);
                return ERROR_MEMORY_ALLOCATION;
            }
            memcpy(chosen_names[pos], files[i].filename, len + 1);
            pos++;
        }
    }

    result->filenames = chosen_names;
    result->count = selected_count;
    result->total_size = total_sz;
    result->total_importance = total_imp;

    free(dp);
    free(take);
    free(selected);
    return SUCCESS;
}

void free_backup_result(backup_result_t *result) {
    if (result == NULL) {
        return;
    }
    free_names(result->filenames, result->count);
    result->filenames = NULL;
    result->count = 0;
    result->total_size = 0;
    result->total_importance = 0;
}