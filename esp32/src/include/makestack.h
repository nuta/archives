#ifndef __MAKESTACK_H__
#define __MAKESTACK_H__

#include <Arduino.h>
#include <Wire.h>
#include <stdarg.h>

void print(const char *fmt, ...);
void println(const char *fmt, ...);
void publish(const char *event, char *value);

#endif
