#include <kernel/init.h>
#include <kernel/kmalloc.h>
#include "hypercall.h"


extern uint8_t __bss_start;
extern uint8_t __bss_end;
extern int __kmalloc_static_chunk;
extern int __kmalloc_static_chunk_end;

struct resources __resources;
struct hypercalls *hypercalls;

void esp8266_init(struct hypercalls *_hypercalls) {

    for (uint8_t *p = &__bss_start; p <= &__bss_end; p++) {
        *p = 0;
    }

    hypercalls = _hypercalls;
    add_kmalloc_chunk(&__kmalloc_static_chunk,
                      (uintptr_t) &__kmalloc_static_chunk_end - (uintptr_t) &__kmalloc_static_chunk,
                      true);

    add_kmalloc_chunk((void *) 0x3fff6000 /* used by firmware on updating */, 0x2000, true);
    add_kmalloc_chunk((void *) 0x3fff8000 /* used by firmware on updating */, 0x2000, false);

    init_kernel((struct resources *) &__resources);
}
