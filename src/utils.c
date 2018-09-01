/// @file
/// @brief Utility functons.
#include "utils.h"
#include "malloc.h"

/// Converts an ASCII string into an integer. Supports hexadecimal string
/// (prefixed by 0x) and decimal string.
int ena_str2int(const char *str) {
    char *s = (char *) str;
    int base = 10;
    size_t i;
    if (str[0] != '\0' && str[1] != '\0' && str[0] == '0') {
        switch (str[1]) {
            case 'x':
                /* 0x123, 0xabc, ... */
                base = 16;
                i = 2; // Remove `0x' prefix.
                break;
            default:
                // Invalid string.
                return 0;
        }
    } else {
        i = 0;
    }

    int r = 0;
    while (str[i] != '\0') {
        char ch = s[i];
        int value;
        if (ena_isalpha(ch)) {
            value = (ena_isupper(ch) ? ch - 'A' : ch - 'a') + 10;
        } else {
            value = ch - '0';
        }

        r = r * base + value;
        i++;
    }

    return r;
}

void *ena_memcpy(void *dst, const void *src, size_t len) {
    uint8_t *dst_p = dst;
    uint8_t *src_p = (uint8_t *) src;

    while (len > 0) {
        *dst_p = *src_p;
        dst_p++;
        src_p++;
        len--;
    }

    return dst;
}

int ena_memcmp(void *ptr1, const void *ptr2, size_t len) {
    uint8_t *ptr2_p = (uint8_t *) ptr2;
    uint8_t *ptr1_p = (uint8_t *) ptr1;

    while (len > 0) {
        int tmp = *ptr2_p - *ptr1_p;
        if (tmp) {
            return tmp;
        }

        ptr1_p++;
        ptr2_p++;
        len--;
    }

    return 0;
}

int ena_strcmp(const char *s1, const char *s2) {
    while (*s1 != '\0' && *s2 != '\0') {
        int tmp = *s1 - *s2;
        if (tmp) {
            return tmp;
        }

        s1++;
        s2++;
    }

    return *s1 - *s2;
}

int ena_strncmp(const char *s1, const char *s2, size_t n) {
    while (*s1 != '\0' && *s2 != '\0') {
        int tmp = *s1 - *s2;
        if (tmp) {
            return tmp;
        }

        n--;
        if (n == 0) {
            break;
        }

        s1++;
        s2++;
    }

    return *s1 - *s2;
}

size_t ena_strlen(const char *str) {
    size_t len = 0;
    while (*str != '\0') {
        str++;
        len++;
    }

    return len;
}

char *ena_strdup(const char *str) {
    size_t len = ena_strlen(str);
    char *new_str = ena_malloc(len + 1);
    ena_memcpy(new_str, str, len);
    new_str[len] = '\0';
    return new_str;
}

char *ena_strndup(const char *str, size_t len) {
    size_t str_len = ena_strlen(str);
    size_t actual_len = (str_len < len) ? str_len : len;
    char *new_str = ena_malloc(actual_len + 1);
    ena_memcpy(new_str, str, actual_len);
    new_str[len] = '\0';
    return new_str;
}


#ifdef __x86_64__
uintptr_t arch_get_stack_bottom(void) {
    uintptr_t bottom;
    __asm__ __volatile__("movq %%rsp, %0" : "=m"(bottom));
    return bottom;
}

void arch_load_regs(uintptr_t *regs) {
    __asm__ __volatile__(
        "movq %%rax, %0 \n"
        "movq %%rbx, %1 \n"
        "movq %%rcx, %2 \n"
        "movq %%rdx, %3 \n"
        "movq %%rdi, %4 \n"
        "movq %%rsi, %5 \n"
        "movq %%r8,  %6 \n"
        "movq %%r9,  %7 \n"
        "movq %%r10, %8 \n"
        "movq %%r11, %9 \n"
        "movq %%r12, %10 \n"
        "movq %%r13, %11 \n"
        "movq %%r14, %12 \n"
        "movq %%r15, %13 \n"
    :
    "=m"(regs[0]),
    "=m"(regs[1]),
    "=m"(regs[2]),
    "=m"(regs[3]),
    "=m"(regs[4]),
    "=m"(regs[5]),
    "=m"(regs[6]),
    "=m"(regs[7]),
    "=m"(regs[8]),
    "=m"(regs[9]),
    "=m"(regs[10]),
    "=m"(regs[11]),
    "=m"(regs[12]),
    "=m"(regs[13])
    );
}
#elif __EMSCRIPTEN__
uintptr_t arch_get_stack_bottom(void) {
    return 0;
}

void arch_load_regs(uintptr_t *regs) {
}
#else
#error "unsupported arch"
#endif
