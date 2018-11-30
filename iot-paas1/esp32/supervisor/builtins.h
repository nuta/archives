#pragma once
#include "engine.h"

struct global_func {
    const char *name;
    jerry_value_t (*handler)(const jerry_value_t func,
                             const jerry_value_t this_,
                             const jerry_value_t *args,
                             const jerry_length_t argc);
};

extern struct global_func builtins[];
