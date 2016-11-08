#include <kernel/init.h>
#include <kernel/kmalloc.h>
#include "finfo.h"


extern uint8_t __bss_start;
extern uint8_t __bss_end;
extern int __kmalloc_static_chunk;
extern int __kmalloc_static_chunk_end;

struct resources __resources;
struct firmware_info *finfo;

void esp8266_init(struct firmware_info *_finfo) {

    for (uint8_t *p = &__bss_start; p <= &__bss_end; p++) {
        *p = 0;
    }

    finfo = _finfo;
    add_kmalloc_chunk(&__kmalloc_static_chunk,
                      (uintptr_t) &__kmalloc_static_chunk_end - (uintptr_t) &__kmalloc_static_chunk);
    add_kmalloc_chunk((void *) 0x3fff7000 /* used by firmware on updating */, 0x2000);

    init_kernel((struct resources *) &__resources);
}
