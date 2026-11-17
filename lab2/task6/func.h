#ifndef FUNC_H
#define FUNC_H

#include <stdio.h>
#include "status.h"

status oversscanf(const char *restrict str, const char *restrict format, int *restrict scanned_count, ...);
status overfscanf(FILE *restrict stream, const char *restrict format, int *restrict scanned_count, ...);

#endif