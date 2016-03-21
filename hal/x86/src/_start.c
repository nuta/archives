extern void x86_init_entrypoint(void) __attribute__ ((section(".boot2")));

void x86_init();
void x86_init_entrypoint(void) {
    x86_init();
}
