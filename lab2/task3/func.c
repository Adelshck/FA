#include "func.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#define BLOCK_SIZE 4096

static size_t string_length(const char *const str) {
    if (str == NULL) {
        return 0;
    }
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

static status init_search_result(FileSearchResult *const result) {
    if (result == NULL) {
        return ERROR_NULL_POINTER;
    }
    result->positions = NULL;
    result->count = 0;
    result->capacity = 0;
    return SUCCESS;
}

static status add_match_position(FileSearchResult *const result, const size_t line, const size_t col) {
    if (result == NULL) {
        return ERROR_NULL_POINTER;
    }

    if (result->count >= result->capacity) {
        size_t new_capacity = (result->capacity == 0) ? 8 : result->capacity * 2;
        MatchPosition *new_positions = (MatchPosition *)realloc(result->positions, new_capacity * sizeof(MatchPosition));
        if (new_positions == NULL) {
            return ERROR_MEMORY_ALLOCATION;
        }
        result->positions = new_positions;
        result->capacity = new_capacity;
    }

    result->positions[result->count].line_number = line;
    result->positions[result->count].char_number = col;
    result->count++;

    return SUCCESS;
}

static void print_escaped_string(const char *const str) {
    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (str[i] == '\n') {
            printf("\\n");
        } else if (str[i] == '\t') {
            printf("\\t");
        } else if (str[i] == ' ') {
            printf("·");
        } else {
            putchar(str[i]);
        }
    }
}

void free_search_result(FileSearchResult *const result) {
    if (result != NULL) {
        if (result->positions != NULL) {
            free(result->positions);
            result->positions = NULL;
        }
        result->count = 0;
        result->capacity = 0;
    }
}

void free_search_results(FileSearchResult *const results, const size_t file_count) {
    if (results == NULL) {
        return;
    }
    for (size_t i = 0; i < file_count; ++i) {
        free_search_result(&results[i]);
    }
    free(results);
}

status search_substring_in_file(const char *const file_path, const char *const substring, FileSearchResult *const result) {
    if (file_path == NULL || substring == NULL || result == NULL) {
        return ERROR_NULL_POINTER;
    }

    status init_status = init_search_result(result);
    if (init_status != SUCCESS) {
        return init_status;
    }

    size_t sub_len = string_length(substring);
    if (sub_len == 0 || sub_len > SIZE_MAX - BLOCK_SIZE) {
        return ERROR_INVALID_ARGUMENT;
    }

    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        return ERROR_FILE_OPEN;
    }

    char *buffer = (char *)malloc(BLOCK_SIZE + sub_len);
    if (buffer == NULL) {
        fclose(file);
        return ERROR_MEMORY_ALLOCATION;
    }

    size_t current_line = 1;
    size_t current_char = 1;
    size_t carry = 0;

    for (;;) {
        size_t read_count = fread(buffer + carry, 1, BLOCK_SIZE, file);
        if (read_count == 0) {
            break;
        }

        size_t total = carry + read_count;
        size_t i = 0;

        while (i + sub_len <= total) {
            size_t j = 0;
            while (j < sub_len && buffer[i + j] == substring[j]) {
                j++;
            }

            if (j == sub_len) {
                status add_status = add_match_position(result, current_line, current_char);
                if (add_status != SUCCESS) {
                    free(buffer);
                    fclose(file);
                    free_search_result(result);
                    return add_status;
                }
            }

            if (buffer[i] == '\n') {
                current_line++;
                current_char = 1;
            } else {
                current_char++;
            }
            i++;
        }

        carry = total - i;
        for (size_t k = 0; k < carry; ++k) {
            buffer[k] = buffer[i + k];
        }
    }

    int read_failed = ferror(file);
    free(buffer);
    fclose(file);

    if (read_failed) {
        free_search_result(result);
        return ERROR_FILE_READ;
    }

    return SUCCESS;
}

status search_in_files(const char *const substring, FileSearchResult **const results, const size_t file_count, ...) {
    if (results == NULL) {
        return ERROR_NULL_POINTER;
    }
    *results = NULL;

    if (substring == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (file_count == 0) {
        return ERROR_INVALID_ARGUMENT;
    }

    FileSearchResult *allocated = (FileSearchResult *)calloc(file_count, sizeof(FileSearchResult));
    if (allocated == NULL) {
        return ERROR_MEMORY_ALLOCATION;
    }
    for (size_t i = 0; i < file_count; ++i) {
        allocated[i].path = NULL;
        allocated[i].state = SUCCESS;
        init_search_result(&allocated[i]);
    }

    status precheck = SUCCESS;
    if (string_length(substring) == 0) {
        precheck = ERROR_INVALID_ARGUMENT;
    }

    va_list args;
    va_start(args, file_count);

    status first_error = precheck;

    for (size_t i = 0; i < file_count; ++i) {
        const char *file_path = va_arg(args, const char *);
        allocated[i].path = file_path;

        if (precheck != SUCCESS) {
            allocated[i].state = precheck;
            continue;
        }

        status search_status = search_substring_in_file(file_path, substring, &allocated[i]);
        allocated[i].state = search_status;
        if (search_status != SUCCESS && first_error == SUCCESS) {
            first_error = search_status;
        }
    }

    va_end(args);

    *results = allocated;
    return first_error;
}

status print_search_results(const char *const substring, const FileSearchResult *const results, const size_t file_count) {
    if (substring == NULL || results == NULL) {
        return ERROR_NULL_POINTER;
    }

    printf("Подстрока: \"");
    print_escaped_string(substring);
    printf("\"\n");

    for (size_t i = 0; i < file_count; ++i) {
        const char *path = (results[i].path != NULL) ? results[i].path : "(путь не задан)";
        printf("Файл: %s\n", path);

        switch (results[i].state) {
            case SUCCESS:
                if (results[i].count == 0) {
                    printf("  Совпадений не найдено.\n");
                } else {
                    for (size_t j = 0; j < results[i].count; ++j) {
                        printf("  Строка: %zu, Символ: %zu\n",
                               results[i].positions[j].line_number,
                               results[i].positions[j].char_number);
                    }
                }
                break;
            case ERROR_NULL_POINTER:
                printf("  Ошибка: передан нулевой указатель.\n");
                break;
            case ERROR_INVALID_ARGUMENT:
                printf("  Ошибка: некорректный аргумент.\n");
                break;
            case ERROR_FILE_OPEN:
                printf("  Ошибка открытия файла.\n");
                break;
            case ERROR_FILE_READ:
                printf("  Ошибка чтения файла.\n");
                break;
            case ERROR_MEMORY_ALLOCATION:
                printf("  Ошибка выделения памяти.\n");
                break;
            default:
                printf("  Неизвестная ошибка.\n");
                break;
        }
    }
    printf("\n");

    return SUCCESS;
}