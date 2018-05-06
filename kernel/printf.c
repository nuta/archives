#include <kernel/types.h>

#define abs(x) ((x < 0)? -x : x)


static void print_str(const char *s) {

    for (int i=0; s[i] != '\0'; i++)
        arch_putchar(s[i]);
}


static void print_int(umax_t base, umax_t v, umax_t len,
                      bool sign, bool alt, bool pad, bool sep) {

    static const char *nchars = "0123456789abcdef";
    char buf[20];
    umax_t i;

    /*
     *  -567
     *  ^
     */
    if (sign && (int) v < 0) {
        arch_putchar('-');
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
        umax_t order, _v;
        for (order=1, _v=(umax_t) v; _v /= (umax_t) base; order++);
        for (umax_t j=order; j < len*2; j++) {
            arch_putchar('0');
            if (sep && j == len+1)
                arch_putchar('_');
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
        umax_t index;
        index  = ((umax_t) v % (umax_t) base);
        v     /= (umax_t) base;
        buf[i--] = nchars[index];
        if (sep && i == len+2)
            buf[i--] = '_';
    } while(v != 0);

    i = 0;
    for (int j=0; buf[i] == '\0' && j < (int) sizeof(buf); j++, i++);
    print_str(&buf[i]);
}


void vprintf(const char *fmt, va_list vargs) {
    for (int i=0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%') {
            bool alt = false;
            bool pad = false;
            char specifier;
            umax_t len = sizeof(umax_t); // 1: char, 2: short, 4: unsigned, ...

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
                arch_putchar('%');
                break;
            case 'd':
                print_int(10, va_arg(vargs, umax_t), len, true,  alt, pad, false);
                break;
            case 'u':
                print_int(10, va_arg(vargs, umax_t), len, false, alt, pad, false);
                break;
            case 'p':
                alt = true;
                pad = true;
                // fallthrough
            case 'x':
                print_int(16, va_arg(vargs, umax_t), len, false, alt, pad, false);
                break;
            case 'c':
                arch_putchar(va_arg(vargs, int));
                break;
            case 's':
                print_str(va_arg(vargs, char *));
                break;
            default:
                arch_putchar('%');
                arch_putchar(fmt[i]);
            }
        } else {
            arch_putchar(fmt[i]);
        }
    }
}


int printf(const char *fmt, ...) {
    va_list vargs;

    va_start(vargs, fmt);
    vprintf(fmt, vargs);
    va_end(vargs);

    // FIXME
    return -1;
}
