#include <stdint.h>
#include <stdbool.h>
#include "strfmt.h"

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
 *  @param[in] s  The string to print.
 *
 */
static void print_str(const char *s, void (*callback)(void *arg, char c),
                      void *callback_arg) {

    for (int i=0; s[i] != '\0'; i++)
        callback(callback_arg, s[i]);
}


/**
 *  Prints an escaped string
 *
 *  @param[in] s  The string to print.
 *
 */
static void print_escaped_str(const char *s, void (*callback)(void *arg, char c),
                              void *callback_arg) {

    for (int i=0; s[i] != '\0'; i++) {
        switch (s[i]) {
        case '\n': print_str("\n", callback, callback_arg); break;
        default:   callback(callback_arg, s[i]);
        }
    }
}


/**
 *  Prints a integer
 *
 *  @param[in] base          The radix (2-16).
 *  @param[in] v             The integer to print.
 *  @param[in] len           The byte size of `v`(1, 2, 4, 8)/.
 *  @param[in] sign          `true` on signed or `false` on unsigned integer.
 *  @param[in] alt           `true` if the alternative form is required.
 *  @param[in] pad           `true` if the padding is required.
 *  @param[in] sep           `true` if the integer separater (`_`) is required.
 *  @param[in] callback      The callback function.
 *  @param[in] callback_arg  The argument for the callback.
 *
 */
static void print_int (intmax_t base, intmax_t v, uintmax_t len,
                       bool sign, bool alt, bool pad, bool sep,
                       void (*callback)(void *arg, char c), void *callback_arg) {

    static const char *nchars = "0123456789abcdef";
    char buf[20];
    uintmax_t i;

    /*
     *  -567
     *  ^
     */
    if (sign && (int) v < 0) {
        callback(callback_arg, '-');
        v = abs((int) v);
    }

    /*
     *  0x00001234
     *  ^^
     */
    if (alt && base == 16) {
        print_str("0x", callback, callback_arg);
    }

    /*
     *  0x00001234
     *    ^^^^
     */
    if (pad) {
        uintmax_t order, _v;
        for (order=1, _v=(uintmax_t) v; _v /= (uintmax_t) base; order++);
        for (uintmax_t j=order; j < len*2; j++) {
            callback(callback_arg, '0');
            if (sep && j == len+1)
                callback(callback_arg, '_');
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
    print_str(&buf[i], callback, callback_arg);
}


static void print_double(double value, void (*callback)(void *arg, char c),
                         void *callback_arg) {
    int decimal_place = 4;
    union {double d; uint64_t u;} u = {value};

    int higher = u.u >> 48; // including sign bit
    if (higher == 0x7ff0) {
        strfmt("Inf", callback, callback_arg);
        return;
    }

    if (higher == 0xfff0) {
        strfmt("-Inf", callback, callback_arg);
        return;
    }

    if (higher == 0xffff) {
        strfmt("NaN", callback, callback_arg);
        return;
    }

    if (value < 0) {
        strfmt("-", callback, callback_arg);
        value = -value;
    }

    // Rounded off to the (decimal_place - 1) decimal place.
    double x = 0.5;
    for (int i = 0; i < decimal_place; i++)
        x *= 0.1;
    value += x;

    // Print the integer part.
    long integer = (long) value;
    value -= integer;
    strfmt("%d", callback, callback_arg, integer);

    // Print the decimal part.
    if (decimal_place > 0) {
        strfmt(".", callback, callback_arg);
        for (int i = 0; i < decimal_place; i++) {
            value *= 10.;
            long integer = (long) value;
            value -= integer;
            strfmt("%d", callback, callback_arg, integer);
        }
    }
}


/**
 *  Parses a formatted string
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
 *  ### Flags
 *
 *  | flag |            description            |
 *  |------|-----------------------------------|
 *  |   #  | alternative form                  |
 *  |   0  | pad by `'0'` to fill field length |
 *
 *  @param[in] fmt           The format string.
 *  @param[in] vargs         The arguments.
 *  @param[in] callback      The callback function.
 *  @param[in] callback_arg  The arguemnt for the callback.
 *
 */
void vstrfmt(const char *fmt, va_list vargs, void (*callback)(void *arg, char c),
             void *callback_arg) {

    for (int i=0; fmt[i] != '\0'; i++) {

        if (fmt[i] == '%') {
            bool alt = false;
            bool pad = false;
            char specifier;
            uintmax_t len = sizeof(intmax_t); // 1: char, 2: short, 4: unsigned, ...

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
                    break;
                }
            }

            switch(specifier) {
            case '%':
                callback(callback_arg, '%');
                break;
            case 'f':
                print_double(va_arg(vargs, double), callback, callback_arg);
                break;
            case 'd':
                print_int(10, va_arg(vargs, uintmax_t), len, true,  alt, pad, false,
                          callback, callback_arg);
                break;
            case 'u':
                print_int(10, va_arg(vargs, uintmax_t), len, false, alt, pad, false,
                          callback, callback_arg);
                break;
            case 'p':
                alt = true;
                pad = true;
                // fallthrough
            case 'x':
                print_int(16, va_arg(vargs, uintmax_t), len, false, alt, pad, false,
                          callback, callback_arg);
                break;
            case 'c':
                callback(callback_arg, va_arg(vargs, int));
                break;
            case 's':
                print_str(va_arg(vargs, char *), callback, callback_arg);
                break;
            case 'q':
                print_escaped_str(va_arg(vargs, char *), callback, callback_arg);
                break;
            default:
                callback(callback_arg, '%');
                callback(callback_arg, fmt[i]);
            }
        } else {
            callback(callback_arg, fmt[i]);
        }
    }
}


void strfmt(const char *fmt, void (*callback)(void *arg, char c),
            void *callback_arg, ...) {

    va_list vargs;

    va_start(vargs, callback_arg);
    vstrfmt(fmt, vargs, callback, callback_arg);
    va_end(vargs);
}