use super::serial;

#[no_mangle]
pub unsafe extern "C" fn qemu_machine_boot() {
    serial::init();
    crate::boot::arch_boot();
}
