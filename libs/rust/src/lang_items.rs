#[lang="eh_personality"]
extern fn rust_eh_personality() {
}

#[lang="panic_fmt"]
fn rust_begin_panic() -> ! {
    loop {}
}

#[lang="oom"]
extern fn rust_oom() -> ! {
    loop {}
}
