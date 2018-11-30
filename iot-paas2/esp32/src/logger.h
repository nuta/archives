#pragma once

enum logging_level {
    LOG_CRITICAL,
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
};

void logger(int logger, const char *prefix, const char *format, ...)
    __attribute__((format(printf, 3, 4)));

#define LOG_PREFIX __func__
#define ERROR(format, ...) logger(LOG_INFO, LOG_PREFIX, format, ##__VA_ARGS__)
#define WARN(format, ...) logger(LOG_INFO, LOG_PREFIX, format, ##__VA_ARGS__)
#define INFO(format, ...) logger(LOG_INFO, LOG_PREFIX, format, ##__VA_ARGS__)
#define DEBUG(format, ...) logger(LOG_INFO, LOG_PREFIX, format, ##__VA_ARGS__)
