#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include "func.h"
#include "status.h"

static const char *status_message(status st) {
    switch (st) {
        case SUCCESS:                 return "success";
        case ERROR_NULL_POINTER:      return "null pointer";
        case ERROR_INVALID_PARAMETER: return "invalid parameter";
        case ERROR_OUT_OF_MEMORY:     return "out of memory";
        case ERROR_IO:                return "I/O error";
        case ERROR_FORMAT:            return "format error";
        case ERROR_OVERFLOW:          return "overflow";
    }
    return "unknown error";
}

static void report(FILE *err, status st) {
    if (st != SUCCESS) {
        fprintf(err, "Error: %s\n", status_message(st));
    }
}

int main(void) {
    FILE *out = fdopen(STDOUT_FILENO, "w");
    FILE *err = fdopen(STDERR_FILENO, "w");
    if (out == NULL || err == NULL) {
        return 1;
    }
    setvbuf(err, NULL, _IONBF, 0);

    report(err, overfprintf(out, "Roman (1994): %Ro\n", 1994));
    report(err, overfprintf(out, "Roman (153): %Ro\n", 153));
    report(err, overfprintf(out, "Zeckendorf (19): %Zr\n", 19u));
    report(err, overfprintf(out, "Zeckendorf (10): %Zr\n", 10u));
    report(err, overfprintf(out, "Zeckendorf (100): %Zr\n", 100u));
    report(err, overfprintf(out, "Cv (255, 16): %Cv\n", 255, 16));
    report(err, overfprintf(out, "CV (255, 16): %CV\n", 255, 16));
    report(err, overfprintf(out, "Cv (-255, 2): %Cv\n", -255, 2));
    report(err, overfprintf(out, "Cv (INT_MIN, 16): %Cv\n", INT_MIN, 16));
    report(err, overfprintf(out, "Cv (100, 99 -> base 10): %Cv\n", 100, 99));
    report(err, overfprintf(out, "to (\"ff\", 16): %to\n", "ff", 16));
    report(err, overfprintf(out, "TO (\"FF\", 16): %TO\n", "FF", 16));
    report(err, overfprintf(out, "to (\"-zz\", 36): %to\n", "-zz", 36));
    report(err, overfprintf(out, "mi (42): %mi\n", 42));
    report(err, overfprintf(out, "mu (3000000000): %mu\n", 3000000000u));
    report(err, overfprintf(out, "md (1.5): %md\n", 1.5));
    report(err, overfprintf(out, "mf (1.5f): %mf\n", 1.5f));
    report(err, overfprintf(out, "standard: %5d|%-5d|%05.1f|%s|%c|%ld|%zu|%%\n",
                            42, 42, 3.14159, "text", 'x', 123456789L, (size_t)7));

    char buffer[1024];
    status st = oversprintf(buffer, "Roman: %Ro, base 2: %Cv, float: %.2f", 2026, 10, 2, 2.71828);
    if (st == SUCCESS) {
        fprintf(out, "oversprintf: %s\n", buffer);
    } else {
        report(err, st);
    }

    st = overfprintf(out, "Invalid roman: %Ro", 0);
    fprintf(out, "\nStatus for Ro(0): %s\n", status_message(st));

    st = overfprintf(out, "Invalid digit: %to", "FF", 16);
    fprintf(out, "\nStatus for to(\"FF\", 16): %s\n", status_message(st));

    st = overfprintf(NULL, "x");
    fprintf(out, "Status for NULL stream: %s\n", status_message(st));

    fflush(out);
    fflush(err);
    return 0;
}