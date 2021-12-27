#![no_std]
#![no_main]
#![feature(asm)]
#![feature(abi_efiapi)]

#[macro_use]
extern crate log;
extern crate alloc;

// Keep this line to ensure the `mem*` functions are linked in.
extern crate rlibc;

use uefi::prelude::*;

#[entry]
fn efi_main(_image: Handle, st: SystemTable<Boot>) -> Status {
    uefi_services::init(&st).expect_success("failed to initialize uefi services");

    info!("");
    info!("");
    info!("");
    info!("_  __                   _                 _   _ _____ _____ ___ ");
    info!("| |/ /__ _ ______ _ _ __(_)   ___  _ __   | | | | ____|  ___|_ _|");
    info!("| ' // _` |_  / _` | '__| |  / _ \\| '_ \\  | | | |  _| | |_   | | ");
    info!("| . \\ (_| |/ / (_| | |  | | | (_) | | | | | |_| | |___|  _|  | | ");
    info!("|_|\\_\\__,_/___\\__,_|_|  |_|  \\___/|_| |_|  \\___/|_____|_|   |___|");
    info!("");
    info!("");
    info!("");

    st.boot_services().stall(5000000);
    Status::SUCCESS
}
