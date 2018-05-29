use interfaces::{io};

pub struct IoPort {
    port: u16
}

impl IoPort {
    pub fn new(base: u16, len: u16) -> IoPort {
        // TODO: error handling
        io::Io::from_cid(1).ioalloc(base as u32, len as usize).ok();

        IoPort { port: base }
    }

    #[inline]
    pub unsafe fn out8(&self, offset: u16, value: u8) {
        asm!("outb %al, %dx" :: "{dx}"(self.port + offset), "{ax}"(value));
    }

    #[inline]
    pub unsafe fn in8(&self, offset: u16) -> u8 {
        let value: u8;
        asm!("inb %dx, %al" : "={ax}"(value) : "{dx}"(self.port + offset));
        value
    }
}
