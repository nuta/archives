#![no_std]

extern crate alloc;

use ftl::eventloop::Event;
use ftl::eventloop::EventLoop;
use ftl::interrupt::Interrupt;
use ftl::prelude::*;
use ftl::spec::AppSpec;
use ftl::spec::DeviceMatcher;
use ftl::spec::EnvItem;
use ftl::spec::EnvType;
use ftl_driver::device::DeviceDesc;
use ftl_utils::byte_size::ByteSize;

use crate::virtio_blk::VirtioBlk;

mod virtio_blk;

pub const SPEC: AppSpec = AppSpec {
    name: "virtio_blk",
    start: |vsyscall| ftl::start(vsyscall, main),
    env: &[EnvItem {
        name: "device",
        ty: EnvType::Device {
            matcher: DeviceMatcher::DeviceTree {
                compatible: "virtio,mmio",
            },
        },
    }],
    exports: &[],
};

#[derive(serde::Deserialize)]
struct Environ {
    device: DeviceDesc,
}

fn probe(desc: &DeviceDesc) -> (VirtioBlk, Interrupt) {
    for m in &desc.matches {
        match VirtioBlk::probe(m.iospace, m.dma) {
            Ok(virtio_blk) => {
                let interrupt = Interrupt::acquire(m.irq).unwrap();
                return (virtio_blk, interrupt);
            }
            Err(virtio_blk::Error::OtherDeviceType(_)) => {
                continue;
            }
            Err(e) => {
                panic!("failed to probe virtio_net: {:?}", e);
            }
        }
    }

    panic!("probe failed");
}

#[derive(Debug)]
enum Context {
    Interrupt,
}

fn main(env: Environ) {
    let (mut device, interrupt) = probe(&env.device);

    let disk_size = device.disk_size().try_into().expect("disk size too large");
    info!("found a disk: {}", ByteSize(disk_size));

    info!("writing 0xa5 to sector 0");
    device.write(0, &[0xa5; 512]).unwrap();

    let mut eventloop = EventLoop::new().unwrap();
    eventloop
        .add_interrupt(interrupt, Context::Interrupt)
        .unwrap();

    loop {
        let (ctx, event) = eventloop.wait().unwrap();
        match (ctx, event) {
            (Context::Interrupt, Event::InterruptFired { interrupt }) => {
                if let Err(e) = device.handle_irq() {
                    panic!("failed to handle interrupt: {:?}", e);
                }

                if let Err(e) = interrupt.acknowledge() {
                    panic!("failed to acknowledge interrupt: {:?}", e);
                }
            }
            (ctx, event) => {
                warn!("unexpected event: {:?} with context: {:?}", event, ctx);
            }
        }
    }
}
