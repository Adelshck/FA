#include <stdio.h>

#include "func.h"

static const char *status_message(status st)
{
    switch (st) {
    case SUCCESS:
        return "успешно";
    case ERROR_NULL_POINTER:
        return "передан нулевой указатель";
    case ERROR_INVALID_ARGUMENT:
        return "некорректный аргумент";
    case ERROR_ALLOCATION:
        return "не удалось выделить память";
    case ERROR_FILE_OPEN:
        return "не удалось открыть файл";
    case ERROR_FILE_READ:
        return "ошибка чтения файла";
    case ERROR_HEAP_EMPTY:
        return "куча пуста";
    case ERROR_HEAP_FULL:
        return "куча переполнена";
    case ERROR_CODE_TOO_LONG:
        return "код превышает допустимую длину";
    case ERROR_OVERFLOW:
        return "переполнение при вычислениях";
    }
    return "неизвестная ошибка";
}

static void print_code_table(const unsigned long long frequencies[BYTE_VALUES],
                      char *const codes[BYTE_VALUES])
{
    int i;

    printf("%-6s %-12s %s\n", "Байт", "Частота", "Код");
    for (i = 0; i < BYTE_VALUES; i++) {
        if (frequencies[i] > 0 && codes[i] != NULL) {
            printf("%-6d %-12llu %s\n", i, frequencies[i], codes[i]);
        }
    }
}

static void print_stats(const CompressionStats *stats)
{
    printf("Размер исходного файла: %llu байт\n", stats->original_bytes);
    printf("Размер закодированных данных: %llu бит\n", stats->encoded_bits);
    printf("Размер закодированных данных: %llu байт\n", stats->encoded_bytes);
    printf("Экономия: %.2f%%\n", stats->savings_percent);
}

int main(int argc, char *argv[])
{
    unsigned long long frequencies[BYTE_VALUES];
    unsigned long long file_size = 0;
    char *codes[BYTE_VALUES];
    HuffmanNode *root = NULL;
    CompressionStats stats;
    status st;
    int i;

    if (argc != 2) {
        fprintf(stderr, "Использование: %s <путь_к_файлу>\n", argc > 0 ? argv[0] : "huffman");
        return 1;
    }
    for (i = 0; i < BYTE_VALUES; i++) {
        codes[i] = NULL;
    }

    st = count_frequencies(argv[1], frequencies, &file_size);
    if (st == SUCCESS) {
        st = build_huffman_tree(frequencies, &root);
    }
    if (st == SUCCESS) {
        st = build_codes(root, codes);
    }
    if (st == SUCCESS) {
        st = compute_stats(frequencies, codes, file_size, &stats);
    }
    if (st == SUCCESS) {
        print_code_table(frequencies, codes);
        print_stats(&stats);
    } else {
        fprintf(stderr, "Ошибка: %s\n", status_message(st));
    }

    tree_free(root);
    free_codes(codes);
    return st == SUCCESS ? 0 : 1;
}