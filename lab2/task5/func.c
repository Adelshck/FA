#include "func.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

enum { SPEC_CAP = 64 };

typedef struct {
    FILE *stream;
    char *buffer;
    size_t offset;
    int is_string;
} destination_t;

typedef enum {
    ARG_INT,
    ARG_LONG,
    ARG_LLONG,
    ARG_INTMAX,
    ARG_SIZE,
    ARG_PTRDIFF,
    ARG_DOUBLE,
    ARG_LDOUBLE,
    ARG_STR,
    ARG_PTR
} arg_kind_t;

typedef union {
    int i;
    long l;
    long long ll;
    intmax_t j;
    size_t z;
    ptrdiff_t t;
    double d;
    long double ld;
    const char *s;
    void *p;
} arg_value_t;

static status write_char(destination_t *dest, char c) {
    if (dest == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (dest->is_string) {
        if (dest->buffer == NULL) {
            return ERROR_NULL_POINTER;
        }
        dest->buffer[dest->offset] = c;
        dest->offset++;
        dest->buffer[dest->offset] = '\0';
    } else {
        if (dest->stream == NULL) {
            return ERROR_NULL_POINTER;
        }
        if (fputc(c, dest->stream) == EOF) {
            return ERROR_IO;
        }
    }
    return SUCCESS;
}

static status write_string(destination_t *dest, const char *s) {
    if (dest == NULL || s == NULL) {
        return ERROR_NULL_POINTER;
    }
    for (size_t i = 0; s[i] != '\0'; i++) {
        status st = write_char(dest, s[i]);
        if (st != SUCCESS) {
            return st;
        }
    }
    return SUCCESS;
}

static status format_roman(destination_t *dest, int val) {
    if (dest == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (val <= 0 || val > 3999) {
        return ERROR_INVALID_PARAMETER;
    }

    const int values[] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
    const char *symbols[] = {"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};

    int rem = val;
    for (size_t i = 0; i < 13; i++) {
        while (rem >= values[i]) {
            status st = write_string(dest, symbols[i]);
            if (st != SUCCESS) {
                return st;
            }
            rem -= values[i];
        }
    }
    return SUCCESS;
}

static status format_zeckendorf(destination_t *dest, unsigned int val) {
    if (dest == NULL) {
        return ERROR_NULL_POINTER;
    }

    if (val == 0) {
        return write_string(dest, "1");
    }

    unsigned long long fib[64];
    size_t count = 0;
    unsigned long long a = 1;
    unsigned long long b = 2;
    while (a <= val) {
        fib[count++] = a;
        unsigned long long t = a + b;
        a = b;
        b = t;
    }

    char bits[64];
    unsigned long long rest = val;
    for (size_t k = count; k > 0; k--) {
        size_t i = k - 1;
        if (rest >= fib[i]) {
            bits[i] = '1';
            rest -= fib[i];
        } else {
            bits[i] = '0';
        }
    }

    for (size_t i = 0; i < count; i++) {
        status st = write_char(dest, bits[i]);
        if (st != SUCCESS) {
            return st;
        }
    }
    return write_char(dest, '1');
}

static status format_base_custom(destination_t *dest, int val, int base, int uppercase) {
    if (dest == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (base < 2 || base > 36) {
        base = 10;
    }

    if (val == 0) {
        return write_char(dest, '0');
    }

    unsigned int uval;
    if (val < 0) {
        status st = write_char(dest, '-');
        if (st != SUCCESS) {
            return st;
        }
        uval = 0u - (unsigned int)val;
    } else {
        uval = (unsigned int)val;
    }

    char buf[40];
    size_t idx = 0;
    const char *digits = uppercase ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   : "0123456789abcdefghijklmnopqrstuvwxyz";

    while (uval > 0) {
        buf[idx++] = digits[uval % (unsigned int)base];
        uval /= (unsigned int)base;
    }

    for (size_t i = 0; i < idx / 2; i++) {
        char temp = buf[i];
        buf[i] = buf[idx - 1 - i];
        buf[idx - 1 - i] = temp;
    }
    buf[idx] = '\0';

    return write_string(dest, buf);
}

static status parse_to_decimal(destination_t *dest, const char *str, int base, int uppercase) {
    if (dest == NULL || str == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (base < 2 || base > 36) {
        base = 10;
    }

    int negative = 0;
    size_t i = 0;
    if (str[0] == '-') {
        negative = 1;
        i = 1;
    } else if (str[0] == '+') {
        i = 1;
    }

    if (str[i] == '\0') {
        return ERROR_FORMAT;
    }

    unsigned long long limit = negative ? (unsigned long long)LLONG_MAX + 1ULL
                                        : (unsigned long long)LLONG_MAX;
    unsigned long long result = 0;

    for (; str[i] != '\0'; i++) {
        char c = str[i];
        int digit = -1;
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (!uppercase && c >= 'a' && c <= 'z') {
            digit = c - 'a' + 10;
        } else if (uppercase && c >= 'A' && c <= 'Z') {
            digit = c - 'A' + 10;
        }

        if (digit < 0 || digit >= base) {
            return ERROR_FORMAT;
        }

        if (result > (limit - (unsigned long long)digit) / (unsigned long long)base) {
            return ERROR_OVERFLOW;
        }
        result = result * (unsigned long long)base + (unsigned long long)digit;
    }

    char buf[32];
    int len;
    if (negative) {
        len = snprintf(buf, sizeof(buf), "-%llu", result);
    } else {
        len = snprintf(buf, sizeof(buf), "%llu", result);
    }
    if (len < 0 || (size_t)len >= sizeof(buf)) {
        return ERROR_FORMAT;
    }

    return write_string(dest, buf);
}

static status format_memory_dump(destination_t *dest, const void *ptr, size_t size) {
    if (dest == NULL || ptr == NULL) {
        return ERROR_NULL_POINTER;
    }

    const unsigned char *bytes = (const unsigned char *)ptr;
    for (size_t i = 0; i < size; i++) {
        unsigned char b = bytes[i];
        char byte_str[9];
        for (int bit = 7; bit >= 0; bit--) {
            byte_str[7 - bit] = ((b >> bit) & 1) ? '1' : '0';
        }
        byte_str[8] = '\0';

        status st = write_string(dest, byte_str);
        if (st != SUCCESS) {
            return st;
        }

        if (i + 1 < size) {
            st = write_char(dest, ' ');
            if (st != SUCCESS) {
                return st;
            }
        }
    }
    return SUCCESS;
}

static status handle_custom(destination_t *dest, const char *f, size_t *pos, va_list *ap, int *handled) {
    *handled = 1;
    char a = f[*pos];
    char b = f[*pos + 1];
    status st;

    if (a == 'R' && b == 'o') {
        int v = va_arg(*ap, int);
        st = format_roman(dest, v);
    } else if (a == 'Z' && b == 'r') {
        unsigned int v = va_arg(*ap, unsigned int);
        st = format_zeckendorf(dest, v);
    } else if (a == 'C' && (b == 'v' || b == 'V')) {
        int v = va_arg(*ap, int);
        int base = va_arg(*ap, int);
        st = format_base_custom(dest, v, base, b == 'V');
    } else if (a == 't' && b == 'o') {
        const char *s = va_arg(*ap, const char *);
        int base = va_arg(*ap, int);
        st = parse_to_decimal(dest, s, base, 0);
    } else if (a == 'T' && b == 'O') {
        const char *s = va_arg(*ap, const char *);
        int base = va_arg(*ap, int);
        st = parse_to_decimal(dest, s, base, 1);
    } else if (a == 'm' && b == 'i') {
        int v = va_arg(*ap, int);
        st = format_memory_dump(dest, &v, sizeof(v));
    } else if (a == 'm' && b == 'u') {
        unsigned int v = va_arg(*ap, unsigned int);
        st = format_memory_dump(dest, &v, sizeof(v));
    } else if (a == 'm' && b == 'd') {
        double v = va_arg(*ap, double);
        st = format_memory_dump(dest, &v, sizeof(v));
    } else if (a == 'm' && b == 'f') {
        float v = (float)va_arg(*ap, double);
        st = format_memory_dump(dest, &v, sizeof(v));
    } else {
        *handled = 0;
        return SUCCESS;
    }

    *pos += 2;
    return st;
}

static status spec_push(char *spec, size_t *n, char c) {
    if (*n + 1 >= SPEC_CAP) {
        return ERROR_FORMAT;
    }
    spec[*n] = c;
    (*n)++;
    return SUCCESS;
}

static status spec_push_int(char *spec, size_t *n, int v) {
    char tmp[16];
    int len = snprintf(tmp, sizeof(tmp), "%d", v);
    if (len < 0 || (size_t)len >= sizeof(tmp)) {
        return ERROR_FORMAT;
    }
    for (int k = 0; k < len; k++) {
        status st = spec_push(spec, n, tmp[k]);
        if (st != SUCCESS) {
            return st;
        }
    }
    return SUCCESS;
}

static int print_arg(char *out, size_t size, const char *spec, arg_kind_t kind, const arg_value_t *v) {
    switch (kind) {
        case ARG_INT:     return snprintf(out, size, spec, v->i);
        case ARG_LONG:    return snprintf(out, size, spec, v->l);
        case ARG_LLONG:   return snprintf(out, size, spec, v->ll);
        case ARG_INTMAX:  return snprintf(out, size, spec, v->j);
        case ARG_SIZE:    return snprintf(out, size, spec, v->z);
        case ARG_PTRDIFF: return snprintf(out, size, spec, v->t);
        case ARG_DOUBLE:  return snprintf(out, size, spec, v->d);
        case ARG_LDOUBLE: return snprintf(out, size, spec, v->ld);
        case ARG_STR:     return snprintf(out, size, spec, v->s);
        case ARG_PTR:     return snprintf(out, size, spec, v->p);
    }
    return -1;
}

static status handle_standard(destination_t *dest, const char *f, size_t *pos, va_list *ap) {
    size_t i = *pos;
    char spec[SPEC_CAP];
    size_t n = 0;
    status st;

    spec[n++] = '%';

    while (f[i] != '\0' && strchr("-+ #0", f[i]) != NULL) {
        st = spec_push(spec, &n, f[i]);
        if (st != SUCCESS) {
            return st;
        }
        i++;
    }

    if (f[i] == '*') {
        int w = va_arg(*ap, int);
        st = spec_push_int(spec, &n, w);
        if (st != SUCCESS) {
            return st;
        }
        i++;
    } else {
        while (isdigit((unsigned char)f[i])) {
            st = spec_push(spec, &n, f[i]);
            if (st != SUCCESS) {
                return st;
            }
            i++;
        }
    }

    if (f[i] == '.') {
        i++;
        if (f[i] == '*') {
            int p = va_arg(*ap, int);
            i++;
            if (p >= 0) {
                st = spec_push(spec, &n, '.');
                if (st != SUCCESS) {
                    return st;
                }
                st = spec_push_int(spec, &n, p);
                if (st != SUCCESS) {
                    return st;
                }
            }
        } else {
            st = spec_push(spec, &n, '.');
            if (st != SUCCESS) {
                return st;
            }
            while (isdigit((unsigned char)f[i])) {
                st = spec_push(spec, &n, f[i]);
                if (st != SUCCESS) {
                    return st;
                }
                i++;
            }
        }
    }

    char len = 0;
    if (f[i] == 'h') {
        i++;
        if (f[i] == 'h') {
            len = 'H';
            i++;
        } else {
            len = 'h';
        }
    } else if (f[i] == 'l') {
        i++;
        if (f[i] == 'l') {
            len = 'q';
            i++;
        } else {
            len = 'l';
        }
    } else if (f[i] != '\0' && strchr("jztL", f[i]) != NULL) {
        len = f[i];
        i++;
    }

    const char *len_str = "";
    switch (len) {
        case 'H': len_str = "hh"; break;
        case 'h': len_str = "h"; break;
        case 'l': len_str = "l"; break;
        case 'q': len_str = "ll"; break;
        case 'j': len_str = "j"; break;
        case 'z': len_str = "z"; break;
        case 't': len_str = "t"; break;
        case 'L': len_str = "L"; break;
        default: break;
    }
    for (size_t k = 0; len_str[k] != '\0'; k++) {
        st = spec_push(spec, &n, len_str[k]);
        if (st != SUCCESS) {
            return st;
        }
    }

    char conv = f[i];
    if (conv == '\0') {
        return ERROR_FORMAT;
    }
    i++;
    st = spec_push(spec, &n, conv);
    if (st != SUCCESS) {
        return st;
    }
    spec[n] = '\0';

    arg_kind_t kind;
    arg_value_t v;

    switch (conv) {
        case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
            switch (len) {
                case 'l': kind = ARG_LONG;    v.l  = va_arg(*ap, long);      break;
                case 'q': kind = ARG_LLONG;   v.ll = va_arg(*ap, long long); break;
                case 'j': kind = ARG_INTMAX;  v.j  = va_arg(*ap, intmax_t);  break;
                case 'z': kind = ARG_SIZE;    v.z  = va_arg(*ap, size_t);    break;
                case 't': kind = ARG_PTRDIFF; v.t  = va_arg(*ap, ptrdiff_t); break;
                case 'L': return ERROR_FORMAT;
                default:  kind = ARG_INT;     v.i  = va_arg(*ap, int);       break;
            }
            break;
        case 'c':
            if (len != 0) {
                return ERROR_FORMAT;
            }
            kind = ARG_INT;
            v.i = va_arg(*ap, int);
            break;
        case 's':
            if (len != 0) {
                return ERROR_FORMAT;
            }
            kind = ARG_STR;
            v.s = va_arg(*ap, const char *);
            if (v.s == NULL) {
                v.s = "(null)";
            }
            break;
        case 'p':
            if (len != 0) {
                return ERROR_FORMAT;
            }
            kind = ARG_PTR;
            v.p = va_arg(*ap, void *);
            break;
        case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': case 'a': case 'A':
            if (len == 'L') {
                kind = ARG_LDOUBLE;
                v.ld = va_arg(*ap, long double);
            } else if (len == 0) {
                kind = ARG_DOUBLE;
                v.d = va_arg(*ap, double);
            } else {
                return ERROR_FORMAT;
            }
            break;
        default:
            return ERROR_FORMAT;
    }

    int need = print_arg(NULL, 0, spec, kind, &v);
    if (need < 0) {
        return ERROR_FORMAT;
    }

    char *out = (char *)malloc((size_t)need + 1);
    if (out == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    int written = print_arg(out, (size_t)need + 1, spec, kind, &v);
    if (written < 0 || written != need) {
        free(out);
        return ERROR_FORMAT;
    }

    for (int k = 0; k < need; k++) {
        st = write_char(dest, out[k]);
        if (st != SUCCESS) {
            free(out);
            return st;
        }
    }

    free(out);
    *pos = i;
    return SUCCESS;
}

static status process_format(destination_t *dest, const char *format, va_list *ap) {
    if (dest == NULL || format == NULL || ap == NULL) {
        return ERROR_NULL_POINTER;
    }

    size_t i = 0;
    while (format[i] != '\0') {
        if (format[i] != '%') {
            status st = write_char(dest, format[i]);
            if (st != SUCCESS) {
                return st;
            }
            i++;
            continue;
        }

        i++;
        if (format[i] == '\0') {
            return ERROR_FORMAT;
        }

        if (format[i] == '%') {
            status st = write_char(dest, '%');
            if (st != SUCCESS) {
                return st;
            }
            i++;
            continue;
        }

        int handled = 0;
        status st = handle_custom(dest, format, &i, ap, &handled);
        if (st != SUCCESS) {
            return st;
        }
        if (handled) {
            continue;
        }

        st = handle_standard(dest, format, &i, ap);
        if (st != SUCCESS) {
            return st;
        }
    }

    return SUCCESS;
}

status overfprintf(FILE *stream, const char *format, ...) {
    if (stream == NULL || format == NULL) {
        return ERROR_NULL_POINTER;
    }

    destination_t dest;
    dest.stream = stream;
    dest.buffer = NULL;
    dest.offset = 0;
    dest.is_string = 0;

    va_list ap;
    va_start(ap, format);
    status st = process_format(&dest, format, &ap);
    va_end(ap);

    return st;
}

status oversprintf(char *str, const char *format, ...) {
    if (str == NULL || format == NULL) {
        return ERROR_NULL_POINTER;
    }

    destination_t dest;
    dest.stream = NULL;
    dest.buffer = str;
    dest.offset = 0;
    dest.is_string = 1;

    str[0] = '\0';

    va_list ap;
    va_start(ap, format);
    status st = process_format(&dest, format, &ap);
    va_end(ap);

    return st;
}