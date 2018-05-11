#ifdef PRINTK_TEST
#   include <stdarg.h>
#   include <stdint.h>
#   include <stdio.h>
#   include <stdbool.h>
#   define arch_putchar putchar
typedef intmax_t imax_t;
typedef uintmax_t umax_t;
typedef uintptr_t uptr_t;
#else
#   include <kernel/types.h>
#endif

static void print_str(const char *s) {

    while (*s != '\0') {
        arch_putchar(*s);
        s++;
    }
}


static void print_uint(umax_t n, int base, char pad, int width) {
    char tmp[32];
    int i = sizeof(tmp) - 2;
    do {
        tmp[i] = "0123456789abcdef"[n % base];
        n /= base;
        width--;
        i--;
    } while(n > 0 && i > 0);

    while (width-- > 0) {
        arch_putchar(pad);
    }

    tmp[sizeof(tmp) - 1] = '\0';
    print_str(&tmp[i + 1]);
}

static umax_t va_arg_uint(va_list vargs, int num_size) {
    switch (num_size) {
        case 3: return va_arg(vargs, unsigned long long);
        case 2: return va_arg(vargs, unsigned long);
        default: return va_arg(vargs, unsigned);
    }
}


void vprintk(const char *fmt, va_list vargs) {
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
                    arch_putchar('%');
                    in_fmt = false;
                    break;
                case '0':
                    pad = '0';
                    break;
                case '#':
                    print_str("0x");
                    break;
                case 'p':
                    num_size = 3;
                    pad = '0';
                    width = sizeof(uptr_t) * 2;
                    // fallthrough
                case 'x':
                    print_uint(va_arg_uint(vargs, num_size), 16, pad, width);
                    in_fmt = false;
                    break;
                case 'd': {
                    umax_t n = va_arg_uint(vargs, width);
                    if ((int) n < 0) {
                        arch_putchar('-');
                        n = - ((int) n);
                    }

                    print_uint(n, 10, pad, width);
                    in_fmt = false;
                    break;
                }
                case 'u':
                    print_uint(va_arg_uint(vargs, num_size), 10, pad, width);
                    in_fmt = false;
                    break;
                case 'c':
                    arch_putchar(va_arg(vargs, int));
                    in_fmt = false;
                    break;
                case 's':
                    print_str(va_arg(vargs, char *));
                    in_fmt = false;
                    break;
                default:
                    arch_putchar('%');
                    arch_putchar(fmt[i]);
                    in_fmt = false;
            }
        } else if (fmt[i] == '%' && fmt[i + 1] == '\0') {
            arch_putchar('%');
        } else if (fmt[i] == '%') {
            in_fmt = true;
            num_size = 1;
            width = 0; // no padding
            pad = ' ';
        } else {
            arch_putchar(fmt[i]);
        }
    }
}


void printk(const char *fmt, ...) {
    va_list vargs;

    va_start(vargs, fmt);
    vprintk(fmt, vargs);
    va_end(vargs);
}

#ifdef PRINTK_TEST
int main(void) {
    printk("%c  %p  %d  %s\n", '#', 0x00aabbcc11223344, -123, "Hello!");
}
#endif
