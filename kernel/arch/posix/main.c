#include <kernel/types.h>

int main(int argc, char **argv) {

    cpuvars[posix_get_processor_id()] = malloc(sizeof(struct cpuvar));
    kernel_init();
    return 0;
}
