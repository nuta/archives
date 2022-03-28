void supervisor_main();

extern "C" void app_main() {
    supervisor_main();
}
