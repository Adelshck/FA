#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "func.h"
#include "status.h"

static const char *status_message(status code)
{
    switch (code)
    {
    case SUCCESS:
        return "success";
    case ERROR_NULL_POINTER:
        return "null pointer passed";
    case ERROR_INVALID_RADIX_POWER:
        return "r must be in range 1..5";
    case ERROR_ALLOCATION:
        return "memory allocation failed";
    default:
        return "unknown error";
    }
}

int main(void)
{
    const long long numbers[] = {0, 1, 5, 56, 255, 1024, -1, -42, -12345, LLONG_MAX, LLONG_MIN};
    const size_t numbers_count = 11;
    const unsigned int radix_powers[] = {1u, 2u, 3u, 4u, 5u, 6u};
    const size_t radix_count = 6;
    size_t i;
    size_t j;
    char *converted;
    status code;

    for (i = 0; i < numbers_count; i++)
    {
        printf("number = %lld\n", numbers[i]);
        for (j = 0; j < radix_count; j++)
        {
            code = convert_to_base_2r(numbers[i], radix_powers[j], &converted);
            if (code == SUCCESS)
            {
                printf("  r = %u (base %u): %s\n", radix_powers[j], 1u << radix_powers[j], converted);
                free(converted);
            }
            else
            {
                printf("  r = %u: error: %s\n", radix_powers[j], status_message(code));
            }
        }
    }

    code = convert_to_base_2r(10, 2u, NULL);
    printf("null result pointer: %s\n", status_message(code));

    return 0;
}