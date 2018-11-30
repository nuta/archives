#include <stdio.h>
#include <stdarg.h>
#include <esp_system.h>
#include "jerryscript-port.h"

void jerry_port_log (jerry_log_level_t level, const char *format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

// Handles assertion failures.
void jerry_port_fatal(jerry_fatal_code_t code) {
  printf("jerryscript: fatal error %d\n", code);
  esp_restart();
}

double jerry_port_get_current_time (void) {
  return (double) 0.0;
}


bool jerry_port_get_time_zone (jerry_time_zone_t *tz) {
  tz->offset = 0;
  tz->daylight_saving_time = 0;
  return true;
}
