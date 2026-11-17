#ifndef FUNC_H
#define FUNC_H

#include <stdio.h>
#include "status.h"
#include "tree.h"

status parse_bracket_expression(const char *expression, Node **root);
status render_tree(FILE *out, const Node *root);
status process_file(const char *input_path, const char *output_path);

#endif