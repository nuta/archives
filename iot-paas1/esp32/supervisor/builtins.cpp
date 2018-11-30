#include "builtins.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include "smms.h"
#include "logger.h"
#include "utils.h"

#define BUILTIN_FUNCTION(func_name)                          \
    static jerry_value_t func_name(                          \
        const jerry_value_t func, const jerry_value_t this_, \
        const jerry_value_t *args, const jerry_length_t argc)

#define VERIFY_ARG_TYPE(i, verify_type)              \
    do {                                             \
        if (i + 1 > argc) {                          \
            WARN("%s: too few args", __func__);     \
            return jerry_create_undefined();         \
        }                                            \
                                                     \
        if (!verify_type(args[i])) {                 \
            WARN("%s: %d: invalid arg type",         \
                   __func__, i);                     \
            return jerry_create_undefined();         \
        }                                            \
    } while (0)

#define ENSURE_INTEGER_ARG(i) VERIFY_ARG_TYPE(i, jerry_value_is_number)
#define ENSURE_BOOLEAN_ARG(i) VERIFY_ARG_TYPE(i, jerry_value_is_boolean)
#define ENSURE_STRING_ARG(i) VERIFY_ARG_TYPE(i, jerry_value_is_string)
#define ENSURE_ARRAY_ARG(i) VERIFY_ARG_TYPE(i, jerry_value_is_array)
#define GET_INTEGER_ARG(i) (int(jerry_get_number_value(args[i])))
#define GET_BOOLEAN_ARG(i) (jerry_value_to_boolean(args[i]))

BUILTIN_FUNCTION(print) {
    ENSURE_STRING_ARG(0);

    jerry_value_t str_val = jerry_value_to_string(args[0]);
    size_t length = jerry_get_string_size(str_val);
    char *str = (char *) malloc(length + 1);
    size_t c = jerry_string_to_char_buffer(str_val, (jerry_char_t *)str, length);
    str[c] = '\0';

    INFO("%s", str);
    smms->append_log(str);

    free(str);
    jerry_release_value(str_val);
    return jerry_create_undefined();
}

BUILTIN_FUNCTION(busywait) {
    ENSURE_INTEGER_ARG(0);

    int usec = GET_INTEGER_ARG(0);
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

bool i2c_initialized = false;
BUILTIN_FUNCTION(i2cInit) {
    if (!i2c_initialized) {
	    i2c_config_t conf;
	    conf.mode = I2C_MODE_MASTER;
	    conf.sda_io_num = (gpio_num_t) 21;
	    conf.scl_io_num = (gpio_num_t) 22;
	    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	    conf.master.clk_speed = 400000;
	    i2c_param_config(I2C_NUM_0, &conf);
	    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
        i2c_initialized = true;
    }

    return jerry_create_undefined();
}

BUILTIN_FUNCTION(i2cRead) {
    return jerry_create_undefined();
}

BUILTIN_FUNCTION(i2cWrite) {
    ENSURE_INTEGER_ARG(0);
    ENSURE_ARRAY_ARG(1);

    int addr = GET_INTEGER_ARG(0);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, 0);

    size_t length = jerry_get_array_length(args[1]);
    for (size_t i = 0; i < length; i++) {
        jerry_value_t val = jerry_get_property_by_index(args[1], i);
        uint8_t byte = uint8_t(jerry_get_number_value(val));
    	i2c_master_write_byte(cmd, byte, 0);
        jerry_release_value(val);
    }

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 20 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

    return jerry_create_undefined();
}


struct global_func builtins[] = {
    {"__print", print},
    {"__busywait", busywait},
    {"__setMode", setMode},
    {"__digitalRead", digitalRead},
    {"__digitalWrite", digitalWrite},
    {"__i2cInit", i2cInit},
    {"__i2cRead", i2cRead},
    {"__i2cWrite", i2cWrite},
    {nullptr, nullptr}
};
