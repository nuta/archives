pub struct IoPort {
    port: u16
}

impl IoPort {
    pub fn new(port: u16) -> IoPort {
        IoPort { port: port }
    }

    #[inline]
    pub unsafe fn out8(&self, value: u8) {
        asm!("outb %al, %dx" :: "{dx}"(self.port), "{ax}"(value));
    }

    #[inline]
    pub unsafe fn in8(&self) -> u8 {
        let value: u8;
        asm!("inb %dx, %al" : "={ax}"(value) : "{dx}"(self.port));
        value
    }
}
