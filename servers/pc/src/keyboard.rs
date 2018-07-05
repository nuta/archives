//
//  8042 Keyboard Controller
//
use resea::channel::{Channel};
use resea::arch::x64::{IoPort, Irq};

pub type KeyCode = u8;
const KEYBOARD_IO_BASE: u16 = 0x60;
const KEYBOARD_IO_INPUT: u16 = 0;
const KEYBOARD_IRQ: u32 = 1;

pub struct Keyboard {
    ioport: IoPort,
    irq: Irq,
}

impl Keyboard {
    pub fn new() -> Keyboard {
        let ioport = IoPort::new(KEYBOARD_IO_BASE, 5);
        let irq = Irq::new(KEYBOARD_IRQ);

        Keyboard {
            ioport: ioport,
            irq: irq,
        }
    }

    pub fn listen_for_irq(&self, server: &Channel) {
        self.irq.listen(server);
    }

    pub fn get_keycode(&self) -> KeyCode {
        let scancode = unsafe { self.ioport.in8(KEYBOARD_IO_INPUT) };
        scancode
    }
}
