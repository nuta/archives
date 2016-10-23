#include <stdarg.h>
#include <arch.h>
#include <mutex.h>
#include "logging.h"


/**
 *  Returns absolute value
 *
 *  @param[in] x  the integer to process
 *  @return  the absolute value of `x`
 *
 */
#define abs(x) ((x < 0)? -x : x)


/**
 *  Prints a string
 *
 *  @param[in] s  the string to print
 *
 */
static void print_str (const char *s) {

    for (int i=0; s[i] != '\0'; i++)
        arch_printchar(s[i]);
}


/**
 *  Prints a integer
 *
 *  @param[in] base  the radix (2-16)
 *  @param[in] v     the integer to print
 *  @param[in] len   the byte size of `v` (1, 2, 4, 8)
 *  @param[in] sign  `true` on signed or `false` on unsigned integer
 *  @param[in] alt   `true` if the alternative form is required
 *  @param[in] pad   `true` if the padding is required
 *  @param[in] sep   `true` if the integer separater (`_`) is required
 *
 */
static void print_int (intmax_t base, intmax_t v, uintmax_t len,
                       bool sign, bool alt, bool pad, bool sep) {

    static const char *nchars = "0123456789abcdef";
    char buf[20];
    uintmax_t i;

    /*
     *  -567
     *  ^
     */
    if (sign && (int) v < 0) {
        arch_printchar('-');
        v = abs((int) v);
    }

    /*
     *  0x00001234
     *  ^^
     */
    if (alt && base == 16) {
        print_str("0x");
    }

    /*
     *  0x00001234
     *    ^^^^
     */
    if (pad) {
        uintmax_t order, _v;
        for (order=1, _v=(uintmax_t) v; _v /= (uintmax_t) base; order++);
        for (uintmax_t j=order; j < len*2; j++) {
            arch_printchar('0');
            if (sep && j == len+1)
                arch_printchar('_');
        }
    }

    /*
     *  0x00001234
     *        ^^^^
     */
    for (int j=0; j < (int) sizeof(buf); j++)
        buf[j] = '\0';
    i = sizeof(buf) - 2;

    do {
        uintmax_t index;
        index  = ((uintmax_t) v % (uintmax_t) base);
        v     /= (uintmax_t) base;
        buf[i--] = nchars[index];
        if (sep && i == len+2)
            buf[i--] = '_';
    } while(v != 0);

    i = 0;
    for (int j=0; buf[i] == '\0' && j < (int) sizeof(buf); j++, i++);
    print_str(&buf[i]);
}


/**
 *  Prints a formatted string
 *
 *  ### Format
 *
 *      %[flag][length][specifier]
 *
 *
 *  ### Specifiers
 *
 *  |  spec. |        description                 |
 *  |--------|------------------------------------|
 *  |   d    | signed decimal                     |
 *  |   u    | unsigned decimal                   |
 *  |   x    | hexadecimal                        |
 *  |   p    | pointer (treated as `uintmax_t`)   |
 *  |   c    | ASCII character                    |
 *  |   s    | ASCII characters terminated by `0` |
 *
 *
 *  ### Length modifiers
 *
 *  | modifier |     length     |
 *  |----------|----------------|
 *  |    hh    |    1 byte      |
 *  |    h     |    2 bytes     |
 *  |    l     |    4 bytes     |
 *  |    ll    |    8 bytes     |
 *  |    j     |   `uintmax_t`  |
 *
 *
 *  ### Flags
 *
 *  | flag |            description            |
 *  |------|-----------------------------------|
 *  |   #  | alternative form                  |
 *  |   0  | pad by `'0'` to fill field length |
 *
 *  @param[in] fmt    a format string
 *  @param[in] vargs  arguemnts
 *
 */
void vprintfmt(const char *fmt, va_list vargs) {

    for (int i=0; fmt[i] != '\0'; i++) {

        if (fmt[i] == '%') {
            bool alt = false;
            bool pad = false;
            char specifier;
            uintmax_t len = sizeof(intmax_t); // 1: char, 2: short, 4: unsigned, ...
            uintmax_t arg = 0; // initialize 0 to supress compiler warning

            for (;;) {
                i++;
                if (fmt[i] == '#') {
                    alt = true;
                } else if (fmt[i] == '0') {
                    pad = true;
                } else if (fmt[i] == '\0') {
                    specifier = '%';
                    break;
                } else {
                    specifier = fmt[i];
                    arg = va_arg(vargs, uintmax_t);
                    break;
                }
            }

            // len
            // TODO
            switch(fmt[i]) {
            case 'h':
            case 'l':
            case 'j':
                specifier = fmt[i];
                break;
            }

            switch(specifier) {
            case '%':
                arch_printchar('%');
                break;
            case 'd':
                print_int(10, arg, len, true,  alt, pad, false);
                break;
            case 'u':
                print_int(10, arg, len, false, alt, pad, false);
                break;
            case 'p':
                alt = true;
                pad = true;
                // fallthrough
            case 'x':
                print_int(16, arg, len, false, alt, pad, false);
                break;
            case 'c':
                arch_printchar((char) arg);
                break;
            case 's':
                print_str((char *) arg);
                break;
            default:
                arch_printchar(fmt[i]);
            }
        } else {
            arch_printchar(fmt[i]);
        }
    }
}


static mutex_t printfmt_lock = MUTEX_INITIALIZER;

void printfmt(const char *format, ...) {
    va_list vargs;

    mutex_lock(&printfmt_lock);

    va_start(vargs, format);
    vprintfmt(format, vargs);
    va_end(vargs);

    arch_printchar('\n');

    mutex_unlock(&printfmt_lock);
}
