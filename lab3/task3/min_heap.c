#include <stdlib.h>

#include "min_heap.h"

status node_create(unsigned long long weight, int symbol, HuffmanNode *left,
                   HuffmanNode *right, HuffmanNode **out)
{
    HuffmanNode *node;

    if (out == NULL) {
        return ERROR_NULL_POINTER;
    }
    *out = NULL;
    node = malloc(sizeof(*node));
    if (node == NULL) {
        return ERROR_ALLOCATION;
    }
    node->weight = weight;
    node->symbol = symbol;
    node->left = left;
    node->right = right;
    *out = node;
    return SUCCESS;
}

int node_is_leaf(const HuffmanNode *node)
{
    return node != NULL && node->left == NULL && node->right == NULL;
}

void tree_free(HuffmanNode *root)
{
    if (root == NULL) {
        return;
    }
    tree_free(root->left);
    tree_free(root->right);
    free(root);
}

static void swap_items(HuffmanNode **a, HuffmanNode **b)
{
    HuffmanNode *tmp = *a;
    *a = *b;
    *b = tmp;
}

static void sift_up(MinHeap *heap, size_t index)
{
    size_t parent;

    while (index > 0) {
        parent = (index - 1) / 2;
        if (heap->items[parent]->weight <= heap->items[index]->weight) {
            break;
        }
        swap_items(&heap->items[parent], &heap->items[index]);
        index = parent;
    }
}

static void sift_down(MinHeap *heap, size_t index)
{
    size_t left;
    size_t right;
    size_t smallest;

    for (;;) {
        left = 2 * index + 1;
        right = 2 * index + 2;
        smallest = index;
        if (left < heap->size &&
            heap->items[left]->weight < heap->items[smallest]->weight) {
            smallest = left;
        }
        if (right < heap->size &&
            heap->items[right]->weight < heap->items[smallest]->weight) {
            smallest = right;
        }
        if (smallest == index) {
            break;
        }
        swap_items(&heap->items[smallest], &heap->items[index]);
        index = smallest;
    }
}

status heap_create(size_t capacity, MinHeap **out)
{
    MinHeap *heap;

    if (out == NULL) {
        return ERROR_NULL_POINTER;
    }
    *out = NULL;
    if (capacity == 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    heap = malloc(sizeof(*heap));
    if (heap == NULL) {
        return ERROR_ALLOCATION;
    }
    heap->items = malloc(capacity * sizeof(*heap->items));
    if (heap->items == NULL) {
        free(heap);
        return ERROR_ALLOCATION;
    }
    heap->size = 0;
    heap->capacity = capacity;
    *out = heap;
    return SUCCESS;
}

status heap_push(MinHeap *heap, HuffmanNode *node)
{
    if (heap == NULL || node == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (heap->size >= heap->capacity) {
        return ERROR_HEAP_FULL;
    }
    heap->items[heap->size] = node;
    sift_up(heap, heap->size);
    heap->size++;
    return SUCCESS;
}

status heap_pop(MinHeap *heap, HuffmanNode **out)
{
    if (heap == NULL || out == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (heap->size == 0) {
        return ERROR_HEAP_EMPTY;
    }
    *out = heap->items[0];
    heap->size--;
    if (heap->size > 0) {
        heap->items[0] = heap->items[heap->size];
        sift_down(heap, 0);
    }
    return SUCCESS;
}

size_t heap_size(const MinHeap *heap)
{
    return heap == NULL ? 0 : heap->size;
}

void heap_release_nodes(MinHeap *heap)
{
    size_t i;

    if (heap == NULL) {
        return;
    }
    for (i = 0; i < heap->size; i++) {
        tree_free(heap->items[i]);
    }
    heap->size = 0;
}

void heap_destroy(MinHeap *heap)
{
    if (heap == NULL) {
        return;
    }
    free(heap->items);
    free(heap);
}