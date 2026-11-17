#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "func.h"

#define SPEC_SIZE 32

typedef struct {
    int (*get)(void *ctx);
    int (*unget)(int c, void *ctx);
    bool (*scan_std)(void *ctx, const char *spec, void *dest, bool suppress);
    void *ctx;
} Reader;

typedef struct {
    const char *ptr;
} StringContext;

static int string_get_char(void *ctx) {
    StringContext *sc = (StringContext *)ctx;
    if (*sc->ptr == '\0') return EOF;
    return (unsigned char)*(sc->ptr++);
}

static int string_unget_char(int c, void *ctx) {
    StringContext *sc = (StringContext *)ctx;
    if (c != EOF) sc->ptr--;
    return c;
}

static bool string_scan_std(void *ctx, const char *spec, void *dest, bool suppress) {
    StringContext *sc = (StringContext *)ctx;
    char fmt[SPEC_SIZE + 3];
    snprintf(fmt, sizeof(fmt), "%s%%n", spec);
    int consumed = -1;
    if (suppress) {
        sscanf(sc->ptr, fmt, &consumed);
    } else {
        sscanf(sc->ptr, fmt, dest, &consumed);
    }
    if (consumed < 0) return false;
    sc->ptr += consumed;
    return true;
}

static int file_get_char(void *ctx) {
    return fgetc((FILE *)ctx);
}

static int file_unget_char(int c, void *ctx) {
    return ungetc(c, (FILE *)ctx);
}

static bool file_scan_std(void *ctx, const char *spec, void *dest, bool suppress) {
    FILE *stream = (FILE *)ctx;
    char fmt[SPEC_SIZE + 3];
    snprintf(fmt, sizeof(fmt), "%s%%n", spec);
    int consumed = -1;
    if (suppress) {
        fscanf(stream, fmt, &consumed);
    } else {
        fscanf(stream, fmt, dest, &consumed);
    }
    return consumed >= 0;
}

static int skip_spaces(const Reader *r) {
    int c;
    while ((c = r->get(r->ctx)) != EOF && isspace(c)) {
    }
    return c;
}

static int roman_value(char c) {
    switch (c) {
        case 'I': return 1;
        case 'V': return 5;
        case 'X': return 10;
        case 'L': return 50;
        case 'C': return 100;
        case 'D': return 500;
        case 'M': return 1000;
        default: return 0;
    }
}

static bool parse_roman(const Reader *r, int *result) {
    int c = skip_spaces(r);
    if (c == EOF) return false;
    r->unget(c, r->ctx);

    char buf[32];
    int len = 0;
    while (len < 31) {
        c = r->get(r->ctx);
        if (c == EOF) break;
        char uc = (char)toupper(c);
        if (roman_value(uc) == 0) {
            r->unget(c, r->ctx);
            break;
        }
        buf[len++] = uc;
    }
    if (len == 0) return false;

    int total = 0;
    for (int i = 0; i < len; i++) {
        int val = roman_value(buf[i]);
        if (i + 1 < len && val < roman_value(buf[i + 1])) {
            total -= val;
        } else {
            total += val;
        }
    }
    *result = total;
    return true;
}

static bool parse_zeckendorf(const Reader *r, unsigned int *result) {
    int c = skip_spaces(r);
    if (c == EOF) return false;
    r->unget(c, r->ctx);

    unsigned int val = 0;
    unsigned int fib_prev = 1;
    unsigned int fib_curr = 2;
    bool prev_one = false;

    while ((c = r->get(r->ctx)) == '0' || c == '1') {
        if (c == '1') {
            if (prev_one) {
                *result = val;
                return true;
            }
            val += fib_prev;
            prev_one = true;
        } else {
            prev_one = false;
        }
        unsigned int next_fib = fib_prev + fib_curr;
        fib_prev = fib_curr;
        fib_curr = next_fib;
    }
    if (c != EOF) r->unget(c, r->ctx);
    return false;
}

static bool parse_custom_base(const Reader *r, int base, bool uppercase, int *result) {
    int c = skip_spaces(r);
    if (c == EOF) return false;

    bool negative = false;
    if (c == '-') {
        negative = true;
        c = r->get(r->ctx);
    } else if (c == '+') {
        c = r->get(r->ctx);
    }
    if (c == EOF) return false;
    r->unget(c, r->ctx);

    unsigned int val = 0;
    bool read_any = false;

    while ((c = r->get(r->ctx)) != EOF) {
        int digit = -1;
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (uppercase && c >= 'A' && c <= 'Z') {
            digit = c - 'A' + 10;
        } else if (!uppercase && c >= 'a' && c <= 'z') {
            digit = c - 'a' + 10;
        }
        if (digit < 0 || digit >= base) {
            r->unget(c, r->ctx);
            break;
        }
        read_any = true;
        val = val * (unsigned int)base + (unsigned int)digit;
    }

    if (!read_any) return false;

    *result = negative ? (int)(0u - val) : (int)val;
    return true;
}

static status read_spec(const char **fmt, char *spec, size_t size, bool *suppress) {
    size_t n = 0;
    spec[n++] = '%';
    if (**fmt == '*') {
        *suppress = true;
        spec[n++] = '*';
        (*fmt)++;
    }
    while (isdigit((unsigned char)**fmt)) {
        if (n + 2 >= size) return ERROR_FORMAT;
        spec[n++] = *(*fmt)++;
    }
    while (**fmt != '\0' && strchr("hljztL", **fmt) != NULL) {
        if (n + 2 >= size) return ERROR_FORMAT;
        spec[n++] = *(*fmt)++;
    }
    if (**fmt == '\0' || strchr("diuoxXcsfFeEgGaAp", **fmt) == NULL) return ERROR_FORMAT;
    if (n + 2 > size) return ERROR_FORMAT;
    spec[n++] = *(*fmt)++;
    spec[n] = '\0';
    return SUCCESS;
}

static bool match_char(const Reader *r, char expected) {
    int c = r->get(r->ctx);
    if (c != (unsigned char)expected) {
        if (c != EOF) r->unget(c, r->ctx);
        return false;
    }
    return true;
}

static status scan_core(const Reader *r, const char *fmt, int *count, va_list args) {
    *count = 0;

    while (*fmt != '\0') {
        if (isspace((unsigned char)*fmt)) {
            while (*fmt != '\0' && isspace((unsigned char)*fmt)) fmt++;
            int c = skip_spaces(r);
            if (c != EOF) r->unget(c, r->ctx);
            continue;
        }

        if (*fmt != '%') {
            if (!match_char(r, *fmt)) return ERROR_CONVERSION;
            fmt++;
            continue;
        }

        fmt++;

        if (*fmt == '%') {
            if (!match_char(r, '%')) return ERROR_CONVERSION;
            fmt++;
            continue;
        }

        if (fmt[0] == 'R' && fmt[1] == 'o') {
            fmt += 2;
            int *dest = va_arg(args, int *);
            if (!dest) return ERROR_NULL_POINTER;
            if (!parse_roman(r, dest)) return ERROR_CONVERSION;
            (*count)++;
            continue;
        }

        if (fmt[0] == 'Z' && fmt[1] == 'r') {
            fmt += 2;
            unsigned int *dest = va_arg(args, unsigned int *);
            if (!dest) return ERROR_NULL_POINTER;
            if (!parse_zeckendorf(r, dest)) return ERROR_CONVERSION;
            (*count)++;
            continue;
        }

        if (fmt[0] == 'C' && (fmt[1] == 'v' || fmt[1] == 'V')) {
            bool uppercase = (fmt[1] == 'V');
            fmt += 2;
            int *dest = va_arg(args, int *);
            int base = va_arg(args, int);
            if (!dest) return ERROR_NULL_POINTER;
            if (base < 2 || base > 36) base = 10;
            if (!parse_custom_base(r, base, uppercase, dest)) return ERROR_CONVERSION;
            (*count)++;
            continue;
        }

        char spec[SPEC_SIZE];
        bool suppress = false;
        status st = read_spec(&fmt, spec, sizeof(spec), &suppress);
        if (st != SUCCESS) return st;

        void *dest = NULL;
        if (!suppress) {
            dest = va_arg(args, void *);
            if (!dest) return ERROR_NULL_POINTER;
        }
        if (!r->scan_std(r->ctx, spec, dest, suppress)) return ERROR_CONVERSION;
        if (!suppress) (*count)++;
    }

    return SUCCESS;
}

status oversscanf(const char *restrict str, const char *restrict format, int *restrict scanned_count, ...) {
    if (!str || !format || !scanned_count) return ERROR_NULL_POINTER;

    StringContext sc = { str };
    Reader reader = { string_get_char, string_unget_char, string_scan_std, &sc };

    va_list args;
    va_start(args, scanned_count);
    status st = scan_core(&reader, format, scanned_count, args);
    va_end(args);
    return st;
}

status overfscanf(FILE *restrict stream, const char *restrict format, int *restrict scanned_count, ...) {
    if (!stream || !format || !scanned_count) return ERROR_NULL_POINTER;

    Reader reader = { file_get_char, file_unget_char, file_scan_std, stream };

    va_list args;
    va_start(args, scanned_count);
    status st = scan_core(&reader, format, scanned_count, args);
    va_end(args);
    return st;
}