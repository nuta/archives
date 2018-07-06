#[lang="eh_personality"]
extern fn rust_eh_personality() {
}

#[lang="oom"]
extern fn rust_oom() -> ! {
    loop {}
}

#[lang="start"]
fn lang_start<T>(main: fn() -> T, _argc: isize, _argv: *const *const u8) -> isize {
    main();
    0
}
