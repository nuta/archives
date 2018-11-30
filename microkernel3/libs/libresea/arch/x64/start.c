void main(void);

void start(void) {
    main();

    /* TODO: graceful termination */
    __asm__ __volatile__("ud2");
}
