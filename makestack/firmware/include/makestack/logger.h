#ifndef __MAKESTACK_LOGGER_H__
#define __MAKESTACK_LOGGER_H__

#include <makestack/types.h>

#define LOGGER_BUF_SIZE ((size_t) 4098)

#define TRACE(fmt, ...) logger("[%s] TRACE: " fmt "\n", __func__, ## __VA_ARGS__)
#define DEBUG(fmt, ...) logger("[%s] DEBUG: " fmt "\n", __func__, ## __VA_ARGS__)
#define WARN(fmt, ...)  logger("[%s] WARN: " fmt "\n", __func__, ## __VA_ARGS__)
#define INFO(fmt, ...)  logger(fmt "\n", ## __VA_ARGS__)

void init_logger();
void logger(const char *format, ...);
void vlogger(const char *format, va_list vargs);
char *read_logger_buffer(size_t *length);

#endif
