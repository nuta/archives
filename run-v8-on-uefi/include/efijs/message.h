#pragma once

#ifdef __cplusplus__
extern "C" {
#endif

#include <stdio.h>

#define EFIJS_DEBUG(fmt, ...) printf("DEBUG: " fmt, ##__VA_ARGS__)
#define EFIJS_BUG(fmt, ...)   printf("BUG: " fmt, ##__VA_ARGS__)
#define EFIJS_INFO(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#define EFIJS_WARN(fmt, ...)  printf("WARN: " fmt, ##__VA_ARGS__)
#define EFIJS_IGNORED_NYI() printf("NYI %s() in %s line %d", __func__, __FILE__, __LINE__)
#define EFIJS_NYI() do { printf("panic: NYI %s() in %s line %d", __func__, __FILE__, __LINE__); while (1); } while(0)
#define EFIJS_PANIC(fmt, ...) do {                  \
        printf("PANIC: " fmt, ##__VA_ARGS__); \
        while (1);                            \
    } while (0)

#ifdef __cplusplus__
}
#endif
