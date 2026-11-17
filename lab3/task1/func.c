#include "func.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static unsigned long long bit_add(unsigned long long a, unsigned long long b)
{
    unsigned long long carry;

    while (b)
    {
        carry = (a & b) << 1;
        a ^= b;
        b = carry;
    }
    return a;
}

static const char *ptr_add_const(const char *base, size_t offset)
{
    return (const char *)(uintptr_t)bit_add((unsigned long long)(uintptr_t)base,
                                            (unsigned long long)offset);
}

static char *ptr_add(char *base, size_t offset)
{
    return (char *)(uintptr_t)bit_add((unsigned long long)(uintptr_t)base,
                                      (unsigned long long)offset);
}

status convert_to_base_2r(long long number, unsigned int r, char **result)
{
    const char alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
    unsigned long long magnitude;
    unsigned long long temp;
    unsigned int mask;
    size_t digits;
    size_t length;
    size_t pos;
    int negative;
    char *buffer;

    if (result == NULL)
    {
        return ERROR_NULL_POINTER;
    }
    *result = NULL;

    if (r < 1u || r > 5u)
    {
        return ERROR_INVALID_RADIX_POWER;
    }

    negative = number < 0;
    magnitude = (unsigned long long)number;
    if (negative)
    {
        magnitude = bit_add(~magnitude, 1ULL);
    }

    mask = ~(~0u << r);

    digits = 0;
    temp = magnitude;
    do
    {
        digits = (size_t)bit_add((unsigned long long)digits, 1ULL);
        temp >>= r;
    } while (temp);

    length = (size_t)bit_add((unsigned long long)digits, negative ? 2ULL : 1ULL);

    buffer = (char *)malloc(length);
    if (buffer == NULL)
    {
        return ERROR_ALLOCATION;
    }

    pos = (size_t)bit_add((unsigned long long)length, ~0ULL);
    *ptr_add(buffer, pos) = '\0';

    temp = magnitude;
    do
    {
        pos = (size_t)bit_add((unsigned long long)pos, ~0ULL);
        *ptr_add(buffer, pos) = *ptr_add_const(alphabet, (size_t)(temp & mask));
        temp >>= r;
    } while (temp);

    if (negative)
    {
        pos = (size_t)bit_add((unsigned long long)pos, ~0ULL);
        *ptr_add(buffer, pos) = '-';
    }

    *result = buffer;
    return SUCCESS;
}