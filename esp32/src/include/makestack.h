#ifndef __MAKESTACK_H__
#define __MAKESTACK_H__

#include <Arduino.h>
#include <Wire.h>
#include <stdarg.h>

typedef void (*command_handler_t)(String arg);

void print(const char *fmt, ...);
void println(const char *fmt, ...);
void publish(const char *event, char *value);
void command(const char *name, command_handler_t handler);

#endif
