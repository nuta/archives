#pragma once
#include <stdint.h>

#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))

static inline uint32_t from_be32(uint8_t *p) {
    return (p[0] << 24) |
           (p[1] << 16) |
           (p[2] << 8)  |
           p[3];
}

static inline void to_be16(uint8_t *p, uint16_t v) {
    p[0] = (v >> 8) & 0xff;
    p[1] = v & 0xff;
}

static inline void to_be32(uint8_t *p, uint32_t v) {
    p[0] = (v >> 24) & 0xff;
    p[1] = (v >> 16) & 0xff;
    p[2] = (v >> 8) & 0xff;
    p[3] = v & 0xff;
}

static inline void hexdump(uint8_t *p, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (i > 0 && i % 16 == 0) {
            printf("\n");
        }

        printf("%02x ", p[i]);
    }

    printf("\n");
}
