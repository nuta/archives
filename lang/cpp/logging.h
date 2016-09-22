#ifndef __LOGGING_H__
#define __LOGGING_H__

void printfmt(const char *format, ...);

#define BUG(fmt, ...)    printfmt("B> " fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)   printfmt("I> " fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...)  printfmt("D> " fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)   printfmt("W> " fmt, ##__VA_ARGS__)

#define NOT_YET_IMPLEMENTED() WARN("%s:%d: NYI", __FILE__, __LINE__)

#endif
