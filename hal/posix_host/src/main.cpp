#include <stdio.h>
#include <resea.h>

extern "C" void start_apps(void);
extern "C" void kernel_startup(void);


void hal_startup(void) {

}


void posix_host_startup(void) {

    setvbuf(stdout, nullptr, _IONBF, 0);
    kernel_startup();
    start_apps();
}


int main(void) {

    posix_host_startup();
    puts("posix_host: [CRIT] start_apps() returned");
    return 1;
}

