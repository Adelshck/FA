#include <stdio.h>

#include "func.h"
#include "status.h"

static const char *status_message(status st)
{
    switch (st) {
    case SUCCESS:
        return "Success";
    case ERROR_INVALID_ARGUMENT:
        return "Invalid argument (input and output paths must differ)";
    case ERROR_MEMORY_ALLOCATION:
        return "Memory allocation failed";
    case ERROR_FILE_OPEN:
        return "Cannot open file";
    case ERROR_FILE_READ:
        return "File read error";
    case ERROR_FILE_WRITE:
        return "File write error";
    case ERROR_INVALID_SYNTAX:
        return "Invalid syntax";
    case ERROR_EMPTY_LINE:
        return "Empty line";
    }
    return "Unknown error";
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    status st = process_file(argv[1], argv[2]);
    if (st != SUCCESS) {
        fprintf(stderr, "Error: %s\n", status_message(st));
        return 1;
    }

    printf("Trees written to %s\n", argv[2]);
    return 0;
}