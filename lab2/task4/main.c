#include <stdio.h>
#include <stdlib.h>
#include "func.h"

static void free_files(file_info_t *files, int count) {
    if (files == NULL) {
        return;
    }
    for (int i = 0; i < count; i++) {
        free(files[i].filename);
    }
    free(files);
}

int main(void) {
    int n = 0;
    int capacity = 0;

    if (scanf("%d %d", &n, &capacity) != 2 || n <= 0 || capacity < 0) {
        printf("Invalid input\n");
        return 1;
    }

    file_info_t *files = (file_info_t *)calloc((size_t)n, sizeof(file_info_t));
    if (files == NULL) {
        printf("Memory allocation error\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        files[i].filename = (char *)malloc(256);
        if (files[i].filename == NULL) {
            printf("Memory allocation error\n");
            free_files(files, n);
            return 1;
        }
        if (scanf("%255s %d %d", files[i].filename, &files[i].size, &files[i].importance) != 3) {
            printf("Invalid input\n");
            free_files(files, n);
            return 1;
        }
        files[i].original_index = i;
    }

    backup_result_t result;
    result.filenames = NULL;
    result.count = 0;
    result.total_size = 0;
    result.total_importance = 0;

    status st = select_backup_files(files, n, capacity, &result);
    switch (st) {
        case SUCCESS:
            printf("TOTAL_SIZE %d\n", result.total_size);
            printf("TOTAL_IMPORTANCE %lld\n", result.total_importance);
            printf("FILES %d\n", result.count);
            for (int i = 0; i < result.count; i++) {
                printf("%s\n", result.filenames[i]);
            }
            free_backup_result(&result);
            break;
        case ERROR_NULL_POINTER:
            printf("Null pointer\n");
            break;
        case ERROR_INVALID_PARAM:
            printf("Invalid parameter\n");
            break;
        case ERROR_MEMORY_ALLOCATION:
            printf("Memory allocation error\n");
            break;
        default:
            printf("Unknown error\n");
            break;
    }

    free_files(files, n);
    return (st == SUCCESS) ? 0 : 1;
}