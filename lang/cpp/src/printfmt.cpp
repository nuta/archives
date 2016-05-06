#include <resea.h>
#include <hal.h>


void hal_printchar(char);

/**
 *  Returns absolute value
 *
 *  @param[in] x  the integer to process
 *  @return  the absolute value of `x`
 *
 */
#define abs(x) ((x < 0)? -x : x)


/**
 *  Checks if character is alphabetic
 *
 *  @param[in] ch  the character to be checked
 *  @return  `true` if `ch` is alphabetic letter, `false` otherwise
 *
 */
static bool isalphabet (const char ch) {

    return (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'));
}


/**
 *  Prints a string
 *
 *  @param[in] s  the string to print
 *
 */
void print_str (const char *s) {

    for (int i=0; s[i] != '\0'; i++)
        hal_printchar(s[i]);
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
    uintmax_t index;
    uintmax_t i;

    /*
     *  -567
     *  ^
     */
    if (sign && (int) v < 0) {
        hal_printchar('-');
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
            hal_printchar('0');
            if (sep && j == len+1)
                hal_printchar('_');
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


/** Prints a formatted string
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
 *  |   b    | binary number                      |
 *  |   p    | pointer (treated as `uintmax_t`)     |
 *  |   x    | hexadecimal                        |
 *  |   X    | hexadecimal with separator         |
 *  |   c    | ASCII character                    |
 *  |   s    | ASCII characters terminated by `0` |
 *  |   V    | same as `uintmax_t`                  |
 *
 *
 *  ### Length modifiers
 *
 *  | modifier |    length    |
 *  |----------|--------------|
 *  |    hh    |    1 byte    |
 *  |    h     |    2 bytes   |
 *  |    l     |    4 bytes   |
 *  |    ll    |    8 bytes   |
 *  |    j     |   `uintmax_t`  |
 *  |    z     |   `size_t`     |
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
static void vprintfmt (const char *fmt, va_list vargs) {

    for (int i=0; fmt[i] != '\0'; i++) {

        if (fmt[i] == '%') {
            bool alt = false;
            bool pad = false;
            char specifier;
            uintmax_t len = sizeof(intmax_t); // 1: char, 2: short, 4: unsigned, ...
            long long arg;

            for (;;) {
                i++;
                if (isalphabet(fmt[i]))   {
                    specifier = fmt[i];
                    break;
                } else if (fmt[i] == '#') {
                    alt = true;
                } else if (fmt[i] == '0') {
                    pad = true;
                } else if (fmt[i] == '\0') {
                    hal_printchar('%');
                    return;
                }
            }

            // len
            switch(fmt[i]) {
            case 'h':
                if (fmt[i+1] == 'h') {
                    len = 1;
                    i += 2;
                } else {
                    len = 2;
                    i++;
                }
                specifier = fmt[i];
                break;
            case 'l':
                if (fmt[i+1] == 'l') {
                    len = 8;
                    i += 2;
                } else {
                    len = 4;
                    i++;
                }
                specifier = fmt[i];
                break;
            case 'j':
                len = sizeof(uintmax_t);
                i++;
                specifier = fmt[i];
                break;
            case 'V':
                len = sizeof(uintmax_t);
                specifier = 'V';
                break;
            case 'z':
                len = sizeof(size_t);
                i++;
                specifier = fmt[i];
                break;
            }

            // get arg
            if (specifier != 'c' && specifier != 's') {

                switch(len) {
                case 8:
                    arg = va_arg(vargs, long long);
                    break;
                case 4:
                    arg = (int) va_arg(vargs, int);
                    break;
                case 2:
                case 1:
                    arg = va_arg(vargs, int);
                    break;
                }
            }

            switch(specifier) {
            case '%':
                hal_printchar('%');
                break;
            case 'V':
                print_int(10, arg, len, true,  alt, pad, false);
                break;
            case 'd':
                print_int(10, arg, len, true,  alt, pad, false);
                break;
            case 'u':
                print_int(10, arg, len, false, alt, pad, false);
                break;
            case 'b':
                print_int(2,  arg, len, false, alt, pad, false);
                break;
            case 'p':
                alt = true;
                pad = true;                          /* fallthrough */
            case 'x':
                print_int(16, arg, len, false, alt, pad, false);
                break;
            case 'X':
                print_int(16, arg, len, false, alt, pad, true);
                break;
            case 'c':
                hal_printchar(va_arg(vargs, int));
                break;
            case 's':
                print_str(va_arg(vargs, char *));
                break;
            default:
                hal_printchar(fmt[i]);
            }
        } else {
            hal_printchar(fmt[i]);
        }
    }
}


static mutex_t lock = MUTEX_UNLOCKED;

void printfmt(const char *format, ...) {
    va_list vargs;

    lock_mutex(&lock);

    va_start(vargs, format);
    vprintfmt(format, vargs);
    va_end(vargs);

    hal_printchar('\n');

    unlock_mutex(&lock);
}
