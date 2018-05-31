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

    #[inline]
    pub unsafe fn out16(&self, offset: u16, value: u8) {
        asm!("outw %ax, %dx" :: "{dx}"(self.port + offset), "{ax}"(value));
    }

    #[inline]
    pub unsafe fn in16(&self, offset: u16) -> u8 {
        let value: u8;
        asm!("inw %dx, %ax" : "={ax}"(value) : "{dx}"(self.port + offset));
        value
    }

    #[inline]
    pub unsafe fn out32(&self, offset: u16, value: u32) {
        asm!("outl %eax, %dx" :: "{dx}"(self.port + offset), "{eax}"(value));
    }

    #[inline]
    pub unsafe fn in32(&self, offset: u16) -> u32 {
        let value: u32;
        asm!("inl %dx, %eax" : "={eax}"(value) : "{dx}"(self.port + offset));
        value
    }
}
