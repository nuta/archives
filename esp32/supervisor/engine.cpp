#include "engine.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <jerryscript-ext/handler.h>
#include <jerryscript.h>
#include <stdio.h>
#include <string.h>
#include "builtins.h"
#include "preload.h"
#include "utils.h"


static int get_global_integer_var(const char *var_name) {
    jerry_value_t global = jerry_get_global_object();
    jerry_value_t prop = jerry_create_string((const jerry_char_t *) var_name);
    jerry_value_t val = jerry_get_property(global, prop);

    int ret = jerry_get_number_value(val);

    jerry_release_value(val);
    jerry_release_value(prop);
    return ret;
}


static void call_global_function(const char *func_name, jerry_value_t *args, int argv) {
    jerry_value_t global = jerry_get_global_object();
    jerry_value_t prop =
        jerry_create_string((const jerry_char_t *) func_name);
    jerry_value_t func = jerry_get_property(global, prop);
    jerry_release_value(prop);

    jerry_value_t ret = jerry_call_function(func, global, args, argv);

    for (int i = 0; i < argv; i++) {
        jerry_release_value(args[i]);
    }

    jerry_release_value(ret);
    jerry_release_value(func);
    jerry_release_value(global);
}


void interval_task(void *param) {
    while (1) {
        int delay = max(get_global_integer_var("__loop_interval"), 50);
        vTaskDelay(delay / portTICK_PERIOD_MS);

        jerry_value_t args[] = { jerry_create_number(delay) };
        int argc = sizeof(args) / sizeof(jerry_value_t);
        call_global_function("__loop", args, argc);
    }
}


static void print_error_message(const char *title, jerry_value_t error) {
    jerry_value_t prop = jerry_create_string((const jerry_char_t *) "name");
    jerry_value_t name_val = jerry_get_property(error, prop);
    jerry_release_value(prop);
    prop = jerry_create_string((const jerry_char_t *) "message");
    jerry_value_t message_val = jerry_get_property(error, prop);
    jerry_release_value(prop);

    size_t name_size = jerry_get_string_size(name_val);
    char *name = (char *) malloc(name_size + 1);
    jerry_string_to_char_buffer(name_val, (jerry_char_t *) name, name_size);
    name[name_size] = '\0';

    size_t message_size = jerry_get_string_size(message_val);
    char *message = (char *) malloc(message_size + 1);
    jerry_string_to_char_buffer(message_val, (jerry_char_t *) message, message_size);
    message[message_size] = '\0';

    printf("%s: %s: %s\n", title, name, message);

    free(message);
    free(name);
    jerry_release_value(name_val);
    jerry_release_value(message_val);
}


Engine::Engine() {
    printf("initializing JavaScript engine...\n");
    jerry_init(JERRY_INIT_EMPTY);
    xTaskCreate(&interval_task, "interval", 8192, NULL, 5, NULL);
}


void Engine::run(const char *script, size_t script_length) {
    // Register builtin functions.
    for (auto global_func = builtins; global_func->name; global_func++) {
        jerryx_handler_register_global((const jerry_char_t *)global_func->name,
                                       global_func->handler);
    }

    // Load preload.ts
    int ret = jerry_eval((jerry_char_t *) preload, sizeof(preload), true);
    if (jerry_value_has_error_flag(ret)) {
        print_error_message("preload", ret);
        return;
    }
    jerry_release_value(ret);

    // Load app code
    ret = jerry_eval((jerry_char_t *) script, script_length, true);
    if (jerry_value_has_error_flag(ret)) {
        print_error_message("app", ret);
        return;
    }
    jerry_release_value(ret);
}


void Engine::update_config(const char *key, const char *value) {
    jerry_value_t args[] = {
        jerry_create_string((const jerry_char_t *) key),
        jerry_create_string((const jerry_char_t *) value)
    };
    int argc = sizeof(args) / sizeof(jerry_value_t);
    call_global_function("__update_config", args, argc);
}


void Engine::execute_command(const char *key, const char *value) {
    jerry_value_t args[] = {
        jerry_create_string((const jerry_char_t *) key),
        jerry_create_string((const jerry_char_t *) value)
    };
    int argc = sizeof(args) / sizeof(jerry_value_t);
    call_global_function("__execute_command", args, argc);
}


Engine::~Engine() {
    jerry_cleanup();
}
