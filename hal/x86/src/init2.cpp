extern "C" void x86_init(void *binfo);

void x86_init_entrypoint(void *binfo) {
    x86_init(binfo);
}
