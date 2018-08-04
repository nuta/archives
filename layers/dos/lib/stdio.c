#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <dos.h>

int getchar(void) {
    uint64_t rax = SYSFUNC_READ_STDIN, rbx = 0, rcx = 0, rdx = 0;
    sysfunc(&rax, &rbx, &rcx, &rdx);
    return rax & 0xff;
}

void putchar(char ch) {
    uint64_t rax = SYSFUNC_WRITE_STDOUT, rbx = 0, rcx = 0, rdx = ch;
    sysfunc(&rax, &rbx, &rcx, &rdx);
}

static int print_str(const char *s) {
    int ret = 0;
    while (*s != '\0') {
        putchar(*s);
        s++;
        ret++;
    }

    return ret;
}


static int print_uint(unsigned long long n, int base, char pad, int width) {
    int ret = 0;
    char tmp[32];
    int i = sizeof(tmp) - 2;
    do {
        tmp[i] = "0123456789abcdef"[n % base];
        n /= base;
        width--;
        i--;
    } while(n > 0 && i > 0);

    while (width-- > 0) {
        putchar(pad);
        ret++;
    }

    tmp[sizeof(tmp) - 1] = '\0';
    return ret + print_str(&tmp[i + 1]);
}

static unsigned long long va_arg_uint(va_list vargs, int num_size) {
    switch (num_size) {
        case 3: return va_arg(vargs, unsigned long long);
        case 2: return va_arg(vargs, unsigned long);
        default: return va_arg(vargs, unsigned);
    }
}

int vprintf(const char *fmt, va_list vargs) {
    int ret = 0;
    int num_size;
    int width;
    char pad;
    int in_fmt = 0;

    for (int i=0; fmt[i] != '\0'; i++) {
        if (in_fmt) {
            switch(fmt[i]) {
                case 'l':
                    num_size++;
                case '%':
                    putchar('%');
                    ret++;
                    in_fmt = 0;
                    break;
                case '0':
                    pad = '0';
                    break;
                case '#':
                    ret += print_str("0x");
                    break;
                case 'x':
                    ret += print_uint(va_arg_uint(vargs, num_size), 16, pad, width);
                    in_fmt = 0;
                    break;
                case 'd': {
                    unsigned long long n = va_arg_uint(vargs, width);
                    if ((int) n < 0) {
                        putchar('-');
                        n = - ((int) n);
                    }

                    ret += print_uint(n, 10, pad, width);
                    in_fmt = 0;
                    break;
                }
                case 'u':
                    ret += print_uint(va_arg_uint(vargs, num_size), 10, pad, width);
                    in_fmt = 0;
                    break;
                case 'c':
                    putchar(va_arg(vargs, int));
                    ret++;
                    in_fmt = 0;
                    break;
                case 's':
                    ret += print_str(va_arg(vargs, char *));
                    in_fmt = 0;
                    break;
                default:
                    putchar('%');
                    putchar(fmt[i]);
                    ret += 2;
                    in_fmt = 0;
            }
        } else if (fmt[i] == '%' && fmt[i + 1] == '\0') {
            putchar('%');
            ret++;
        } else if (fmt[i] == '%') {
            in_fmt = 1;
            num_size = 1;
            width = 0; // no padding
            pad = ' ';
        } else {
            putchar(fmt[i]);
            ret++;
        }
    }
}

int printf(const char *fmt, ...) {
    va_list vargs;

    va_start(vargs, fmt);
    int n = vprintf(fmt, vargs);
    va_end(vargs);
    return n;
}
