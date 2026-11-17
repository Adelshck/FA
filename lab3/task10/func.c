#include "func.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef enum {
    STATE_EXPECT_NAME,
    STATE_AFTER_NAME,
    STATE_AFTER_CLOSE
} parse_state;

static bool is_name_char(unsigned char c)
{
    return !isspace(c) && c != '(' && c != ')' && c != ',';
}

static bool is_same_file(const char *path_a, const char *path_b) {
    struct stat sa, sb;

    if (stat(path_a, &sa) != 0 || stat(path_b, &sb) != 0) {
        return false; 
    }

    return sa.st_dev == sb.st_dev && sa.st_ino == sb.st_ino;
}

static status stack_push(Node ***stack, size_t *size, size_t *capacity, Node *node)
{
    if (*size == *capacity) {
        size_t new_capacity = (*capacity == 0) ? 16 : *capacity * 2;
        Node **tmp = realloc(*stack, new_capacity * sizeof(Node *));
        if (tmp == NULL) {
            return ERROR_MEMORY_ALLOCATION;
        }
        *stack = tmp;
        *capacity = new_capacity;
    }
    (*stack)[(*size)++] = node;
    return SUCCESS;
}

status parse_bracket_expression(const char *expression, Node **root)
{
    if (expression == NULL || root == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    *root = NULL;

    Node *result = NULL;
    Node *last = NULL;
    Node **stack = NULL;
    size_t size = 0;
    size_t capacity = 0;
    parse_state state = STATE_EXPECT_NAME;
    status st = SUCCESS;
    bool has_content = false;
    const char *p = expression;

    while (*p != '\0' && st == SUCCESS) {
        unsigned char c = (unsigned char)*p;

        if (isspace(c)) {
            p++;
            continue;
        }
        has_content = true;

        if (c == '(') {
            if (state != STATE_AFTER_NAME) {
                st = ERROR_INVALID_SYNTAX;
            } else {
                st = stack_push(&stack, &size, &capacity, last);
                state = STATE_EXPECT_NAME;
                p++;
            }
        } else if (c == ',') {
            if (state == STATE_EXPECT_NAME || size == 0) {
                st = ERROR_INVALID_SYNTAX;
            } else {
                state = STATE_EXPECT_NAME;
                p++;
            }
        } else if (c == ')') {
            if (state == STATE_EXPECT_NAME || size == 0) {
                st = ERROR_INVALID_SYNTAX;
            } else {
                size--;
                state = STATE_AFTER_CLOSE;
                p++;
            }
        } else {
            if (state != STATE_EXPECT_NAME) {
                st = ERROR_INVALID_SYNTAX;
            } else {
                const char *start = p;
                while (*p != '\0' && is_name_char((unsigned char)*p)) {
                    p++;
                }
                Node *node = NULL;
                st = node_create(start, (size_t)(p - start), &node);
                if (st == SUCCESS) {
                    if (size == 0) {
                        result = node;
                    } else {
                        node_append_child(stack[size - 1], node);
                    }
                    last = node;
                    state = STATE_AFTER_NAME;
                }
            }
        }
    }

    if (st == SUCCESS) {
        if (!has_content) {
            st = ERROR_EMPTY_LINE;
        } else if (state == STATE_EXPECT_NAME || size != 0) {
            st = ERROR_INVALID_SYNTAX;
        }
    }

    free(stack);

    if (st != SUCCESS) {
        tree_free(result);
        return st;
    }

    *root = result;
    return SUCCESS;
}

status render_tree(FILE *out, const Node *root)
{
    if (out == NULL || root == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }

    char *prefix = NULL;
    size_t capacity = 0;
    const Node *cur = root;
    size_t depth = 0;

    while (cur != NULL) {
        size_t length = (depth >= 2) ? (depth - 1) * 4 : 0;

        if (length + 1 > capacity) {
            size_t new_capacity = (length + 1) * 2;
            char *tmp = realloc(prefix, new_capacity);
            if (tmp == NULL) {
                free(prefix);
                return ERROR_MEMORY_ALLOCATION;
            }
            prefix = tmp;
            capacity = new_capacity;
        }

        if (depth >= 2) {
            const Node *ancestor = cur->parent;
            for (size_t level = depth - 1; level >= 1; level--) {
                memcpy(prefix + (level - 1) * 4,
                       ancestor->next_sibling != NULL ? "|   " : "    ", 4);
                ancestor = ancestor->parent;
            }
        }
        prefix[length] = '\0';

        int written;
        if (depth == 0) {
            written = fprintf(out, "%s\n", cur->name);
        } else {
            written = fprintf(out, "%s%s%s\n", prefix,
                              cur->next_sibling != NULL ? "|-- " : "`-- ",
                              cur->name);
        }
        if (written < 0) {
            free(prefix);
            return ERROR_FILE_WRITE;
        }

        if (cur->first_child != NULL) {
            cur = cur->first_child;
            depth++;
        } else {
            while (cur != root && cur->next_sibling == NULL) {
                cur = cur->parent;
                depth--;
            }
            if (cur == root) {
                break;
            }
            cur = cur->next_sibling;
        }
    }

    free(prefix);
    return SUCCESS;
}

static status read_line(FILE *in, char **line, bool *eof)
{
    size_t capacity = 128;
    size_t length = 0;
    char *buffer = malloc(capacity);
    if (buffer == NULL) {
        return ERROR_MEMORY_ALLOCATION;
    }

    int ch;
    while ((ch = fgetc(in)) != EOF && ch != '\n') {
        if (length + 2 > capacity) {
            char *tmp = realloc(buffer, capacity * 2);
            if (tmp == NULL) {
                free(buffer);
                return ERROR_MEMORY_ALLOCATION;
            }
            buffer = tmp;
            capacity *= 2;
        }
        buffer[length++] = (char)ch;
    }

    if (ch == EOF && ferror(in)) {
        free(buffer);
        return ERROR_FILE_READ;
    }

    buffer[length] = '\0';
    *eof = (ch == EOF && length == 0);
    *line = buffer;
    return SUCCESS;
}

static status process_stream(FILE *in, FILE *out)
{
    size_t number = 0;

    while (1) {
        char *line = NULL;
        bool eof = false;
        status st = read_line(in, &line, &eof);
        if (st != SUCCESS) {
            return st;
        }
        if (eof) {
            free(line);
            return SUCCESS;
        }

        number++;

        Node *root = NULL;
        st = parse_bracket_expression(line, &root);
        free(line);

        if (st == ERROR_EMPTY_LINE) {
            continue;
        }

        if (st == ERROR_INVALID_SYNTAX) {
            if (fprintf(out, "Expression #%zu: syntax error\n\n", number) < 0) {
                return ERROR_FILE_WRITE;
            }
            continue;
        }

        if (st != SUCCESS) {
            return st;
        }

        if (fprintf(out, "Expression #%zu:\n", number) < 0) {
            tree_free(root);
            return ERROR_FILE_WRITE;
        }

        st = render_tree(out, root);
        tree_free(root);
        if (st != SUCCESS) {
            return st;
        }

        if (fputc('\n', out) == EOF) {
            return ERROR_FILE_WRITE;
        }
    }
}

status process_file(const char *input_path, const char *output_path)
{
    if (input_path == NULL || output_path == NULL ||
        is_same_file(input_path, output_path)) {
        return ERROR_INVALID_ARGUMENT;
    }

    FILE *in = fopen(input_path, "r");
    if (in == NULL) {
        return ERROR_FILE_OPEN;
    }

    FILE *out = fopen(output_path, "w");
    if (out == NULL) {
        fclose(in);
        return ERROR_FILE_OPEN;
    }

    status st = process_stream(in, out);

    fclose(in);
    if (fclose(out) != 0 && st == SUCCESS) {
        st = ERROR_FILE_WRITE;
    }

    return st;
}