#ifndef STATUS_H
#define STATUS_H

typedef enum {
    SUCCESS = 0,
    ERROR_NULL_POINTER,
    ERROR_INVALID_PARAMETER,
    ERROR_OUT_OF_MEMORY,
    ERROR_IO,
    ERROR_FORMAT,
    ERROR_OVERFLOW
} status;

#endif