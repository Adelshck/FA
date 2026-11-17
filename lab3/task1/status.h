#ifndef STATUS_H
#define STATUS_H

typedef enum
{
    SUCCESS = 0,
    ERROR_NULL_POINTER,
    ERROR_INVALID_RADIX_POWER,
    ERROR_ALLOCATION
} status;

#endif