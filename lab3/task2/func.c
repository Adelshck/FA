#include <stdlib.h>
#include "func.h"

static int compare_by_id(const void *a, const void *b)
{
    const talk_t *x = (const talk_t *)a;
    const talk_t *y = (const talk_t *)b;
    return (x->id > y->id) - (x->id < y->id);
}

static int compare_by_finish(const void *a, const void *b)
{
    const talk_t *x = (const talk_t *)a;
    const talk_t *y = (const talk_t *)b;
    if (x->finish != y->finish) {
        return (x->finish > y->finish) - (x->finish < y->finish);
    }
    return (x->start > y->start) - (x->start < y->start);
}

static status validate_talks(const talk_t *talks, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (talks[i].id <= 0) {
            return ERROR_INVALID_ID;
        }
        if (talks[i].start < 0 || talks[i].finish >= MINUTES_IN_DAY ||
            talks[i].start >= talks[i].finish) {
            return ERROR_INVALID_TIME;
        }
    }
    return ERROR_SUCCESS;
}

static status check_unique_ids(talk_t *sorted_by_id, size_t n)
{
    qsort(sorted_by_id, n, sizeof(talk_t), compare_by_id);
    for (size_t i = 1; i < n; i++) {
        if (sorted_by_id[i].id == sorted_by_id[i - 1].id) {
            return ERROR_DUPLICATE_ID;
        }
    }
    return ERROR_SUCCESS;
}

static size_t schedule_greedy(const talk_t *sorted_by_finish, size_t n, int *hall_end, size_t halls)
{
    size_t count = 0;
    for (size_t i = 0; i < n; i++) {
        size_t best = halls;
        for (size_t j = 0; j < halls; j++) {
            if (hall_end[j] <= sorted_by_finish[i].start &&
                (best == halls || hall_end[j] > hall_end[best])) {
                best = j;
            }
        }
        if (best != halls) {
            hall_end[best] = sorted_by_finish[i].finish;
            count++;
        }
    }
    return count;
}

status max_talks_count(const talk_t *talks, size_t n, size_t halls, size_t *result)
{
    if (talks == NULL || result == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (n == 0 || halls == 0) {
        return ERROR_INVALID_ARGUMENT;
    }

    status st = validate_talks(talks, n);
    if (st != ERROR_SUCCESS) {
        return st;
    }

    talk_t *copy = (talk_t *)malloc(n * sizeof(talk_t));
    if (copy == NULL) {
        return ERROR_MEMORY;
    }
    for (size_t i = 0; i < n; i++) {
        copy[i] = talks[i];
    }

    st = check_unique_ids(copy, n);
    if (st != ERROR_SUCCESS) {
        free(copy);
        return st;
    }

    qsort(copy, n, sizeof(talk_t), compare_by_finish);

    size_t effective_halls = halls < n ? halls : n;
    int *hall_end = (int *)calloc(effective_halls, sizeof(int));
    if (hall_end == NULL) {
        free(copy);
        return ERROR_MEMORY;
    }

    *result = schedule_greedy(copy, n, hall_end, effective_halls);

    free(hall_end);
    free(copy);
    return ERROR_SUCCESS;
}