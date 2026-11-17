#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stddef.h>
#include "status.h"
#include "func.h"

status parse_time(const char *text, int *minutes);
status read_talks(FILE *in, talk_t **talks, size_t *n, size_t *halls);

#endif