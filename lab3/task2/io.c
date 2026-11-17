#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include "io.h"

#define TIME_TOKEN_SIZE 32

status parse_time(const char *text, int *minutes)
{
    if (text == NULL || minutes == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (strlen(text) != 5 || text[2] != ':') {
        return ERROR_INVALID_FORMAT;
    }
    if (!isdigit((unsigned char)text[0]) || !isdigit((unsigned char)text[1]) ||
        !isdigit((unsigned char)text[3]) || !isdigit((unsigned char)text[4])) {
        return ERROR_INVALID_FORMAT;
    }
    int hours = (text[0] - '0') * 10 + (text[1] - '0');
    int mins = (text[3] - '0') * 10 + (text[4] - '0');
    if (hours > 23 || mins > 59) {
        return ERROR_INVALID_TIME;
    }
    *minutes = hours * 60 + mins;
    return ERROR_SUCCESS;
}

static status read_header(FILE *in, size_t *n, size_t *halls)
{
    long long n_value = 0;
    long long k_value = 0;
    if (fscanf(in, "%lld %lld", &n_value, &k_value) != 2) {
        return ERROR_INPUT;
    }
    if (n_value <= 0 || k_value <= 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    if ((unsigned long long)n_value > SIZE_MAX / sizeof(talk_t)) {
        return ERROR_INVALID_ARGUMENT;
    }
    *n = (size_t)n_value;
    *halls = (size_t)k_value;
    return ERROR_SUCCESS;
}

static status read_one_talk(FILE *in, talk_t *talk)
{
    long long id = 0;
    char start_text[TIME_TOKEN_SIZE];
    char finish_text[TIME_TOKEN_SIZE];

    if (fscanf(in, "%lld %31s %31s", &id, start_text, finish_text) != 3) {
        return ERROR_INPUT;
    }
    if (id <= 0 || id > INT_MAX) {
        return ERROR_INVALID_ID;
    }

    int start = 0;
    int finish = 0;
    status st = parse_time(start_text, &start);
    if (st != ERROR_SUCCESS) {
        return st;
    }
    st = parse_time(finish_text, &finish);
    if (st != ERROR_SUCCESS) {
        return st;
    }
    if (start >= finish) {
        return ERROR_INVALID_TIME;
    }

    talk->id = (int)id;
    talk->start = start;
    talk->finish = finish;
    return ERROR_SUCCESS;
}

status read_talks(FILE *in, talk_t **talks, size_t *n, size_t *halls)
{
    if (in == NULL || talks == NULL || n == NULL || halls == NULL) {
        return ERROR_NULL_POINTER;
    }

    size_t count = 0;
    size_t hall_count = 0;
    status st = read_header(in, &count, &hall_count);
    if (st != ERROR_SUCCESS) {
        return st;
    }

    talk_t *array = (talk_t *)malloc(count * sizeof(talk_t));
    if (array == NULL) {
        return ERROR_MEMORY;
    }

    for (size_t i = 0; i < count; i++) {
        st = read_one_talk(in, &array[i]);
        if (st != ERROR_SUCCESS) {
            free(array);
            return st;
        }
    }

    *talks = array;
    *n = count;
    *halls = hall_count;
    return ERROR_SUCCESS;
}