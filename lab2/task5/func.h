#ifndef FUNC_H
#define FUNC_H

#include <stdio.h>
#include <stdarg.h>
#include "status.h"

status overfprintf(FILE *stream, const char *format, ...);
status oversprintf(char *str, const char *format, ...);

#endif