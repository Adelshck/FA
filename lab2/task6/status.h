#ifndef STATUS_H
#define STATUS_H

typedef enum {
    SUCCESS = 0,
    ERROR_NULL_POINTER,
    ERROR_IO,
    ERROR_FORMAT,
    ERROR_CONVERSION
} status;

#endif