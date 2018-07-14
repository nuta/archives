#include <stdlib.h>
#include <kernel/types.h>

paddr_t phypages_start;
size_t phypages_num;

int main(int argc, char **argv) {
    phypages_num = 32 * 1024;
    phypages_start = (paddr_t) malloc(phypages_num * PAGE_SIZE);

    cpuvars[posix_get_processor_id()] = malloc(sizeof(struct cpuvar));
    kernel_init();
    return 0;
}
