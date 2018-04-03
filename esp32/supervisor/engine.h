#pragma once
#include <jerryscript.h>

class Engine {
   private:
    bool initialized;
    void run_preload();

   public:
    Engine();
    ~Engine();
    void run(const char *script, size_t script_length);
    void update_config(const char *key, const char *value);
    void execute_command(const char *key, const char *value);
};

extern Engine *app_engine;
