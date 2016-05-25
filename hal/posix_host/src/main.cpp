#include <stdio.h>
#include "posix_host.h"
#include <resea.h>

void start_apps(void);
void kernel_startup(void);


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

