#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#include <stddef.h>

#include "status.h"

typedef struct HuffmanNode {
    unsigned long long weight;
    int symbol;
    struct HuffmanNode *left;
    struct HuffmanNode *right;
} HuffmanNode;

typedef struct {
    HuffmanNode **items;
    size_t size;
    size_t capacity;
} MinHeap;

status node_create(unsigned long long weight, int symbol, HuffmanNode *left, HuffmanNode *right, HuffmanNode **out);
int node_is_leaf(const HuffmanNode *node);
void tree_free(HuffmanNode *root);

status heap_create(size_t capacity, MinHeap **out);
status heap_push(MinHeap *heap, HuffmanNode *node);
status heap_pop(MinHeap *heap, HuffmanNode **out);
size_t heap_size(const MinHeap *heap);
void heap_release_nodes(MinHeap *heap);
void heap_destroy(MinHeap *heap);

#endif