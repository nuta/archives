#[cfg_attr(not(feature = "hosting"), panic_handler)]
#[cfg_attr(feature = "hosting", allow(dead_code))]
fn panic(info: &core::panic::PanicInfo) -> ! {
    println!("panic: {}", info);

    #[allow(clippy::empty_loop)]
    loop {}
}
