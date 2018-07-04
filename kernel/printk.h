#ifndef __PRINTK_H__
#define __PRINTK_H__

void printk(const char *fmt, ...);

#define DEBUG(fmt, ...) printk(fmt "\n", ## __VA_ARGS__)
#define INFO(fmt, ...) printk(fmt "\n", ## __VA_ARGS__)
#define BUG(fmt, ...) printk("BUG: " fmt "\n", ## __VA_ARGS__)
#define PANIC(fmt, ...) do { \
        printk("PANIC: " fmt "\n", ## __VA_ARGS__); \
        arch_panic(); \
    } while(0)
#define HEXDUMP(buf, len) do {                    \
        printk(#buf ":");                         \
                                                  \
        for (size_t i = 0; i < len; i++) {        \
            if (i % 16 == 0) {                    \
                printk("\n    ");                 \
            }                                     \
            printk("%x ", *((u8_t *) buf + i));   \
        }                                         \
                                                  \
        printk("\n");                             \
    } while (0)

#endif
