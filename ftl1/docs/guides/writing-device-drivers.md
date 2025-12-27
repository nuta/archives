# Writing Device Drivers

This guide covers how to write device drivers for FTL by examining the virtio-net driver as a reference implementation.

## Device Discovery

To find the device, declare an environ item with `EnvType::Device`. The system will provide a matched device in the environ:

```rust
pub const SPEC: AppSpec = AppSpec {
    name: "your_driver",
    start: |vsyscall| ftl::start(vsyscall, main),
    env: &[
        EnvItem {
            name: "device", // [!code ++]
            ty: EnvType::Device { // [!code ++]
                matcher: DeviceMatcher::DeviceTree { // [!code ++]
                    compatible: "virtio,mmio", // [!code ++]
                }, // [!code ++]
            }, // [!code ++]
        },
    ],
    exports: &[],
};

#[derive(serde::Deserialize)]
struct Environ {
    device: DeviceDesc, // [!code ++]
}
```

The `DeviceDesc` contains a list of device matches. Each match contains the device information to access it using following APIs.

## Memory-Mapped I/O (MMIO)

### Defining MMIO Registers

First, define registers you want to access:

```rust
use ftl_driver::iospace::{DeviceReg, LittleEndian, ReadOnly};

const MAGIC_REG: DeviceReg<LittleEndian, ReadOnly, u32>
    = DeviceReg::new(0 /* offset */);
```

The `DeviceReg<E, A, T>` is a type to define a register. It takes the following parameters:

- `E`: Endianness. Little endian or big endian.
- `A`: Access control. Read only, write only, read write.
- `T`: Type. `u8`, `u16`, `u32`, or `u64`.
- `offset`: The byte offset from the start of the MMIO region.

### Accessing MMIO Registers

Use `IoSpace` to map MMIO regions:

```rust
// Iterate over all matched devices.
for m in &device.matches {
    // Map the device's MMIO region.
    let iospace = IoSpace::map(m.iospace_desc)?;

    // Read a device register.
    let value = iospace.read32(MAGIC_REG);
    if value == 0x1234_5678 {
        info!("found a device!");
    }
}
```

## Direct Memory Access (DMA)

FTL provides two DMA types: persistent (so-called coherent DMA) and oneshot (so-called streaming DMA).

### Persistent DMA

For long-lived buffers (e.g., device command queues) shared with the device:

```rust
use ftl_driver::persistent_dma::PersistentDma;

let dma_buffer: PersistentDma<MyStruct> = PersistentDma::alloc(m.dma)?;

// The device-visible address of the buffer.
let baddr = dma_buffer.baddr();

// Reading from the buffer.
{
    let guard = dma_buffer.read_guard();
    let data = guard.field;
}

// Writing to the buffer.
{
    let mut guard = dma_buffer.write_guard();
    guard.field = value;
}

```

### Oneshot DMA

For temporary transfers (e.g., network packets) to map temporaily to the device:

```rust
use ftl_driver::oneshot_dma::{OneshotDma, Direction};

struct Driver {
    oneshot_dma: OneshotDma<Vec<u8>>,
}

impl Driver {
    fn probe(m: &DeviceMatch) -> Result<Self, Error> {
        let oneshot_dma: OneshotDma<Vec<u8>> = OneshotDma::new(m.dma)?;
        Ok(Self { oneshot_dma })
    }

    fn send_packet(&mut self, data: Vec<u8>) {
        let baddr = self.oneshot_dma.map(data, Direction::ToDevice).unwrap();

        // `data` is now moved to the device and is accessible
        // at `baddr` in device's address space.

        // Do driver-specific work to tell the device about the packet.
        add_to_packet_queue(baddr);
    }

    // Called when the device has processed the TX packet.
    fn packet_sent(&mut self, baddr: BusAddr) {
        // Unmap the buffer and reclaim the ownership.
        let data: Vec<u8> = self
            .oneshot_dma
            .unmap(baddr, Direction::ToDevice)
            .unwrap();
    }
}
```

## Receiving Interrupts

You can acquire an interrupt using `Interrupt::acquire`:

```rust
let interrupt = Interrupt::acquire(m.irq)?;
ctx.listeners.add_interrupt(interrupt, ());
```

You can receive interrupts in the `Application` event loop:

```rust
enum Context {
    Interrupt,
}

fn main(env: Environ) {
    let mut eventloop = EventLoop::new().unwrap();
    eventloop.add_interrupt(interrupt, Context::Interrupt).unwrap();

    loop {
        let (ctx, event) = eventloop.wait().unwrap();
        match (ctx, event) {
            (Context::Interrupt, Event::Interrupt { irq }) => {
                handle_interrupt();
            }
        }
    }
}
```

## Connecting to Services

How to connect to other OS components depends on the device type.

### Ethernet Drivers

Ethernet drivers need to connect to the TCP/IP server (`apps/tcpip`) by `open`ing a channel to it with `ethernet:<mac>` URI.

TCP/IP server returns an OpenReply message with a new channel to send RX packets the driver received, and receive TX packets to forward to the device.
