#include <resea.h>
#include <resea/logging.h>

#define abs(x) ((x < 0)? -x : x)


static char *print_str(char *buf, const char *s) {

    for (int i = 0; s[i] != '\0'; i++)
        *buf++ = s[i];

    return buf;
}


static char *print_int(char *buf, umax_t base, umax_t v, umax_t len,
                      bool sign, bool alt, bool pad, bool sep) {

    static const char *nchars = "0123456789abcdef";
    char tmp[20];
    umax_t i;

    /*
     *  -567
     *  ^
     */
    if (sign && (int) v < 0) {
        *buf++ = '-';
        v = abs((int) v);
    }

    /*
     *  0x00001234
     *  ^^
     */
    if (alt && base == 16) {
        buf = print_str(buf, "0x");
    }

    /*
     *  0x00001234
     *    ^^^^
     */
    if (pad) {
        umax_t order, _v;
        for (order=1, _v=(umax_t) v; _v /= (umax_t) base; order++);
        for (umax_t j=order; j < len*2; j++) {
            *buf++ = '0';
            if (sep && j == len+1)
                *buf++ = '_';
        }
    }

    /*
     *  0x00001234
     *        ^^^^
     */
    for (int j=0; j < (int) sizeof(tmp); j++)
        tmp[j] = '\0';
    i = sizeof(tmp) - 2;

    do {
        umax_t index;
        index  = ((umax_t) v % (umax_t) base);
        v     /= (umax_t) base;
        tmp[i--] = nchars[index];
        if (sep && i == len+2)
            tmp[i--] = '_';
    } while(v != 0);

    i = 0;
    for (int j=0; tmp[i] == '\0' && j < (int) sizeof(tmp); j++, i++);
    return print_str(buf, &tmp[i]);
}


#define RESERVE_BUF_SIZE(size) do { \
        if (buf_size - 1 <= size) { \
            *buf = '\0';            \
            return;                 \
        } else {                    \
            buf_size -= size;       \
        }                           \
    } while(0)

void vsprintf(char *buf, size_t buf_size, const char *fmt, va_list vargs) {
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
                RESERVE_BUF_SIZE(1);
                *buf++ = '%';
                break;
            case 'd':
                RESERVE_BUF_SIZE(20);
                buf = print_int(buf, 10, va_arg(vargs, umax_t), len, true,  alt, pad, false);
                break;
            case 'u':
                RESERVE_BUF_SIZE(20);
                buf = print_int(buf, 10, va_arg(vargs, umax_t), len, false, alt, pad, false);
                break;
            case 'p':
                alt = true;
                pad = true;
                // fallthrough
            case 'x':
                RESERVE_BUF_SIZE(20);
                buf = print_int(buf, 16, va_arg(vargs, umax_t), len, false, alt, pad, false);
                break;
            case 'c':
                RESERVE_BUF_SIZE(1);
                *buf++ = va_arg(vargs, int);
                break;
            case 's': {
                char *str = va_arg(vargs, char *);
                size_t str_len = strlen(str);
                RESERVE_BUF_SIZE(str_len);
                buf = print_str(buf, str);
                break;
            }
            default:
                *buf++ = '%';
                *buf++ = fmt[i];
            }
        } else {
            *buf++ = fmt[i];
        }
    }

    *buf = '\0';
}

int printf(const char *fmt, ...) {
    va_list vargs;
    char buf[256];

    va_start(vargs, fmt);
    vsprintf((char *) &buf, sizeof(buf), fmt, vargs);
    call_logging_emit(1, (char *) &buf, strlen(buf));
    va_end(vargs);

    return 0;
}
