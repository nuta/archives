#include <kernel/init.h>
#include <kernel/kmalloc.h>
#include "bootinfo.h"


extern uint8_t __kmalloc_static_chunk;
extern uint8_t __kmalloc_static_chunk_end;
extern uint8_t __resources;

void esp8266_init(struct bootinfo *bootinfo) {

    size_t len = (uintptr_t) &__kmalloc_static_chunk_end -
                 (uintptr_t) &__kmalloc_static_chunk;

    add_kmalloc_chunk(&mal,
                      512 /* XXX */, len / 512);

    init_kernel((struct resources *) &__resources);
}
