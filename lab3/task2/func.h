#ifndef FUNC_H
#define FUNC_H

#include <stddef.h>
#include "status.h"

#define MINUTES_IN_DAY 1440

typedef struct {
    int id;
    int start;
    int finish;
} talk_t;

status max_talks_count(const talk_t *talks, size_t n, size_t halls, size_t *result);

#endif