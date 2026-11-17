#include "tree.h"

#include <stdlib.h>
#include <string.h>

status node_create(const char *name, size_t length, Node **out)
{
    if (name == NULL || out == NULL || length == 0) {
        return ERROR_INVALID_ARGUMENT;
    }

    Node *node = malloc(sizeof(Node));
    if (node == NULL) {
        return ERROR_MEMORY_ALLOCATION;
    }

    node->name = malloc(length + 1);
    if (node->name == NULL) {
        free(node);
        return ERROR_MEMORY_ALLOCATION;
    }

    memcpy(node->name, name, length);
    node->name[length] = '\0';
    node->parent = NULL;
    node->first_child = NULL;
    node->last_child = NULL;
    node->next_sibling = NULL;

    *out = node;
    return SUCCESS;
}

status node_append_child(Node *parent, Node *child)
{
    if (parent == NULL || child == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }

    child->parent = parent;
    child->next_sibling = NULL;

    if (parent->last_child == NULL) {
        parent->first_child = child;
    } else {
        parent->last_child->next_sibling = child;
    }
    parent->last_child = child;

    return SUCCESS;
}

void tree_free(Node *root)
{
    Node *cur = root;

    while (cur != NULL) {
        if (cur->first_child != NULL) {
            cur = cur->first_child;
            continue;
        }

        Node *parent = cur->parent;
        if (parent != NULL) {
            parent->first_child = cur->next_sibling;
            if (parent->first_child == NULL) {
                parent->last_child = NULL;
            }
        }

        free(cur->name);
        free(cur);
        cur = parent;
    }
}