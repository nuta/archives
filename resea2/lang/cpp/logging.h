#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <types.h>

__EXTERN_C__

#define BUFFER_SIZE 2048

size_t get_buffered_log(char **s);
void vprintfmt(const char *format, va_list vargs);
void printfmt(const char *format, ...);
void printfmt_nonl(const char *format, ...);
void init_logging();

#ifdef BUILD_DEBUG
#define DEBUG(fmt, ...)  printfmt("D> " fmt, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

#define BUG(fmt, ...)    printfmt("B> " fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)   printfmt("I> " fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)   printfmt("W> " fmt, ##__VA_ARGS__)

#define NOT_YET_IMPLEMENTED() WARN("%s:%d: NYI", __FILE__, __LINE__)

__EXTERN_C_END__

#endif
