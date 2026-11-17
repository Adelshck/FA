#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "func.h"

#define READ_BUFFER_SIZE 4096

status count_frequencies(const char *path, unsigned long long frequencies[BYTE_VALUES],
                         unsigned long long *file_size)
{
    FILE *file;
    unsigned char buffer[READ_BUFFER_SIZE];
    size_t read_count;
    size_t i;
    unsigned long long total = 0;

    if (path == NULL || frequencies == NULL || file_size == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (path[0] == '\0') {
        return ERROR_INVALID_ARGUMENT;
    }
    memset(frequencies, 0, BYTE_VALUES * sizeof(*frequencies));
    *file_size = 0;

    file = fopen(path, "rb");
    if (file == NULL) {
        return ERROR_FILE_OPEN;
    }
    while ((read_count = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        for (i = 0; i < read_count; i++) {
            frequencies[buffer[i]]++;
        }
        total += read_count;
    }
    if (ferror(file)) {
        fclose(file);
        return ERROR_FILE_READ;
    }
    fclose(file);
    *file_size = total;
    return SUCCESS;
}

static status merge_two_lightest(MinHeap *heap)
{
    HuffmanNode *first = NULL;
    HuffmanNode *second = NULL;
    HuffmanNode *parent = NULL;
    status st;

    st = heap_pop(heap, &first);
    if (st != SUCCESS) {
        return st;
    }
    st = heap_pop(heap, &second);
    if (st != SUCCESS) {
        tree_free(first);
        return st;
    }
    st = node_create(first->weight + second->weight, -1, first, second, &parent);
    if (st != SUCCESS) {
        tree_free(first);
        tree_free(second);
        return st;
    }
    st = heap_push(heap, parent);
    if (st != SUCCESS) {
        tree_free(parent);
    }
    return st;
}

status build_huffman_tree(const unsigned long long frequencies[BYTE_VALUES],
                          HuffmanNode **root)
{
    MinHeap *heap = NULL;
    HuffmanNode *leaf = NULL;
    size_t distinct = 0;
    int symbol;
    status st = SUCCESS;

    if (frequencies == NULL || root == NULL) {
        return ERROR_NULL_POINTER;
    }
    *root = NULL;
    for (symbol = 0; symbol < BYTE_VALUES; symbol++) {
        if (frequencies[symbol] > 0) {
            distinct++;
        }
    }
    if (distinct == 0) {
        return SUCCESS;
    }
    st = heap_create(distinct, &heap);
    if (st != SUCCESS) {
        return st;
    }
    for (symbol = 0; symbol < BYTE_VALUES && st == SUCCESS; symbol++) {
        if (frequencies[symbol] == 0) {
            continue;
        }
        st = node_create(frequencies[symbol], symbol, NULL, NULL, &leaf);
        if (st == SUCCESS) {
            st = heap_push(heap, leaf);
            if (st != SUCCESS) {
                tree_free(leaf);
            }
        }
    }
    while (st == SUCCESS && heap_size(heap) > 1) {
        st = merge_two_lightest(heap);
    }
    if (st == SUCCESS) {
        st = heap_pop(heap, root);
    }
    heap_release_nodes(heap);
    heap_destroy(heap);
    return st;
}

static status assign_codes(const HuffmanNode *node, char *path, size_t depth,
                           char *codes[BYTE_VALUES])
{
    status st;
    char *copy;

    if (node_is_leaf(node)) {
        if (node->symbol < 0 || node->symbol >= BYTE_VALUES) {
            return ERROR_INVALID_ARGUMENT;
        }
        copy = malloc(depth + 1);
        if (copy == NULL) {
            return ERROR_ALLOCATION;
        }
        memcpy(copy, path, depth);
        copy[depth] = '\0';
        codes[node->symbol] = copy;
        return SUCCESS;
    }
    if (depth >= MAX_CODE_LENGTH) {
        return ERROR_CODE_TOO_LONG;
    }
    st = SUCCESS;
    if (node->left != NULL) {
        path[depth] = '0';
        st = assign_codes(node->left, path, depth + 1, codes);
    }
    if (st == SUCCESS && node->right != NULL) {
        path[depth] = '1';
        st = assign_codes(node->right, path, depth + 1, codes);
    }
    return st;
}

status build_codes(const HuffmanNode *root, char *codes[BYTE_VALUES])
{
    char path[MAX_CODE_LENGTH + 1];
    char *copy;
    int i;

    if (codes == NULL) {
        return ERROR_NULL_POINTER;
    }
    for (i = 0; i < BYTE_VALUES; i++) {
        codes[i] = NULL;
    }
    if (root == NULL) {
        return SUCCESS;
    }
    if (node_is_leaf(root)) {
        if (root->symbol < 0 || root->symbol >= BYTE_VALUES) {
            return ERROR_INVALID_ARGUMENT;
        }
        copy = malloc(2);
        if (copy == NULL) {
            return ERROR_ALLOCATION;
        }
        copy[0] = '0';
        copy[1] = '\0';
        codes[root->symbol] = copy;
        return SUCCESS;
    }
    return assign_codes(root, path, 0, codes);
}

status compute_stats(const unsigned long long frequencies[BYTE_VALUES],
                     char *const codes[BYTE_VALUES], unsigned long long file_size,
                     CompressionStats *stats)
{
    unsigned long long bits = 0;
    unsigned long long length;
    int i;

    if (frequencies == NULL || codes == NULL || stats == NULL) {
        return ERROR_NULL_POINTER;
    }
    for (i = 0; i < BYTE_VALUES; i++) {
        if (frequencies[i] == 0) {
            continue;
        }
        if (codes[i] == NULL) {
            return ERROR_INVALID_ARGUMENT;
        }
        length = (unsigned long long)strlen(codes[i]);
        if (length == 0 || frequencies[i] > (ULLONG_MAX - bits) / length) {
            return ERROR_OVERFLOW;
        }
        bits += frequencies[i] * length;
    }
    stats->original_bytes = file_size;
    stats->encoded_bits = bits;
    stats->encoded_bytes = bits / 8 + (bits % 8 != 0 ? 1 : 0);
    if (file_size == 0) {
        stats->savings_percent = 0.0;
    } else {
        stats->savings_percent =
            100.0 * (1.0 - (double)stats->encoded_bytes / (double)file_size);
    }
    return SUCCESS;
}

void free_codes(char *codes[BYTE_VALUES])
{
    int i;

    if (codes == NULL) {
        return;
    }
    for (i = 0; i < BYTE_VALUES; i++) {
        free(codes[i]);
        codes[i] = NULL;
    }
}