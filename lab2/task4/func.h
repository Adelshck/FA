#ifndef FUNC_H
#define FUNC_H

#include "status.h"

typedef struct {
    char *filename;
    int size;
    int importance;
    int original_index;
} file_info_t;

typedef struct {
    char **filenames;
    int count;
    int total_size;
    long long total_importance;
} backup_result_t;

status select_backup_files(const file_info_t *files, int n, int capacity, backup_result_t *result);
void free_backup_result(backup_result_t *result);

#endif