#include <resea.h>
#include <resea/logging.h>


static char *print_str(char *buf, const char *s) {

    while (*s != '\0') {
        *buf++ = *s++;
    }

    return buf;
}


static char *print_uint(char *buf, umax_t n, int base, char pad, int width) {
    char tmp[32];
    int i = sizeof(tmp) - 2;
    do {
        tmp[i] = "0123456789abcdef"[n % base];
        n /= base;
        width--;
        i--;
    } while(n > 0 && i > 0);

    while (width-- > 0) {
        *buf++ = pad;
    }

    tmp[sizeof(tmp) - 1] = '\0';
    return print_str(buf, &tmp[i + 1]);
}


static umax_t va_arg_uint(va_list vargs, int num_size) {
    switch (num_size) {
        case 3: return va_arg(vargs, unsigned long long);
        case 2: return va_arg(vargs, unsigned long);
        default: return va_arg(vargs, unsigned);
    }
}

#define RESERVE_BUF_SIZE(size) do {  \
        int _size = size;            \
        if (buf_size - 1 <= _size) { \
            *buf = '\0';             \
            return;                  \
        } else {                     \
            buf_size -= _size;       \
        }                            \
    } while(0)

static void vsprintf(char *buf, size_t buf_size, const char *fmt, va_list vargs) {
    int num_size;
    int width;
    char pad;
    bool in_fmt = false;

    for (int i=0; fmt[i] != '\0'; i++) {
        if (in_fmt) {
            switch(fmt[i]) {
                case 'l':
                    num_size++;
                case '%':
                    RESERVE_BUF_SIZE(1);
                    *buf++ = '%';
                    in_fmt = false;
                    break;
                case '0':
                    pad = '0';
                    break;
                case '#':
                    RESERVE_BUF_SIZE(2);
                    buf = print_str(buf, "0x");
                    break;
                case 'p':
                    num_size = 3;
                    pad = '0';
                    width = sizeof(uptr_t) * 2;
                    // fallthrough
                case 'x':
                    RESERVE_BUF_SIZE(32);
                    buf = print_uint(buf, va_arg_uint(vargs, num_size), 16, pad, width);
                    in_fmt = false;
                    break;
                case 'd': {
                    RESERVE_BUF_SIZE(32);
                    umax_t n = va_arg_uint(vargs, width);
                    if ((int) n < 0) {
                        *buf++ = '-';
                        n = - ((int) n);
                    }

                    buf = print_uint(buf, n, 10, pad, width);
                    in_fmt = false;
                    break;
                }
                case 'u':
                    RESERVE_BUF_SIZE(32);
                    buf = print_uint(buf, va_arg_uint(vargs, num_size), 10, pad, width);
                    in_fmt = false;
                    break;
                case 'c':
                    RESERVE_BUF_SIZE(1);
                    *buf++ = va_arg(vargs, int);
                    in_fmt = false;
                    break;
                case 's': {
                    char *str = va_arg(vargs, char *);
                    RESERVE_BUF_SIZE(strlen(str));
                    buf = print_str(buf, str);
                    in_fmt = false;
                    break;
                }
                default:
                    RESERVE_BUF_SIZE(2);
                    *buf++ = '%';
                    *buf++ = fmt[i];
                    in_fmt = false;
            }
        } else if (fmt[i] == '%' && fmt[i + 1] == '\0') {
            RESERVE_BUF_SIZE(1);
            *buf++ = '%';
        } else if (fmt[i] == '%') {
            in_fmt = true;
            num_size = 1;
            width = 0; // no padding
            pad = ' ';
        } else {
            RESERVE_BUF_SIZE(1);
            *buf++ = fmt[i];
        }
    }

    RESERVE_BUF_SIZE(1);
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
