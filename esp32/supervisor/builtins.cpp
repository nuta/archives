#include "builtins.h"
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <driver/gpio.h>
#include "smms.h"
#include "utils.h"

#define BUILTIN_FUNCTION(func_name)                          \
    static jerry_value_t func_name(                          \
        const jerry_value_t func, const jerry_value_t this_, \
        const jerry_value_t *args, const jerry_length_t argc)

#define VERIFY_ARG_TYPE(i, verify_type)              \
    do {                                             \
        if (i + 1 > argc) {                          \
            printf("%s: too few args\n", __func__);  \
            return jerry_create_undefined();         \
        }                                            \
                                                     \
        if (!verify_type(args[i])) {                 \
            printf("%s: %d: invalid arg type\n",     \
                   __func__, i);                     \
            return jerry_create_undefined();         \
        }                                            \
    } while (0)

#define ENSURE_INTEGER_ARG(i) VERIFY_ARG_TYPE(i, jerry_value_is_number)
#define ENSURE_BOOLEAN_ARG(i) VERIFY_ARG_TYPE(i, jerry_value_is_boolean)
#define ENSURE_STRING_ARG(i) VERIFY_ARG_TYPE(i, jerry_value_is_string)
#define GET_INTEGER_ARG(i) (int(jerry_get_number_value(args[i])))
#define GET_BOOLEAN_ARG(i) (jerry_value_to_boolean(args[i]))

BUILTIN_FUNCTION(print) {
    ENSURE_STRING_ARG(0);

    jerry_value_t str_val = jerry_value_to_string(args[0]);
    size_t length = jerry_get_string_size(str_val);
    char *str = (char *) malloc(length + 1);
    size_t c = jerry_string_to_char_buffer(str_val, (jerry_char_t *)str, length);
    str[c] = '\0';

    printf("log: %s\n", str);
    smms->append_log(str);

    free(str);
    jerry_release_value(str_val);
    return jerry_create_undefined();
}

BUILTIN_FUNCTION(busywait) {
    ENSURE_INTEGER_ARG(0);

    int usec = jerry_get_number_value(GET_INTEGER_ARG(0));
    vTaskDelay(max(usec / 1000, 1));
    return jerry_create_undefined();
}

BUILTIN_FUNCTION(setMode) {
    ENSURE_INTEGER_ARG(0);
    ENSURE_INTEGER_ARG(1);

    gpio_num_t pin = (gpio_num_t)GET_INTEGER_ARG(0);
    gpio_mode_t direction =
        (GET_INTEGER_ARG(1) != 0) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT;

    gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, direction);

    return jerry_create_undefined();
}

BUILTIN_FUNCTION(digitalRead) {
    ENSURE_INTEGER_ARG(0);

    gpio_num_t pin = (gpio_num_t)GET_INTEGER_ARG(0);
    return jerry_create_boolean(gpio_get_level(pin));
}

BUILTIN_FUNCTION(digitalWrite) {
    ENSURE_INTEGER_ARG(0);
    ENSURE_BOOLEAN_ARG(1);

    gpio_num_t pin = (gpio_num_t) GET_INTEGER_ARG(0);
    int level = GET_BOOLEAN_ARG(1) ? 1 : 0;
    gpio_set_level(pin, level);

    return jerry_create_undefined();
}

BUILTIN_FUNCTION(i2cRead) { return jerry_create_undefined(); }

BUILTIN_FUNCTION(i2cWrite) { return jerry_create_undefined(); }

struct global_func builtins[] = {{"__print", print},
                                 {"__busywait", busywait},
                                 {"__setMode", setMode},
                                 {"__digitalRead", digitalRead},
                                 {"__digitalWrite", digitalWrite},
                                 {"__i2cRead", i2cRead},
                                 {"__i2cWrite", i2cWrite},
                                 {nullptr, nullptr}};
