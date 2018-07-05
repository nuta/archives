use resea::arch::x64::{IoPort};

#[derive(Debug)]
pub struct DateTime {
    pub year: u16,
    pub month: u8,
    pub day: u8,
    pub hour: u8,
    pub min: u8,
    pub sec: u8
}

const RTC_IO_BASE: u16 = 0x70;
const RTC_IO_W: u16 = 0x00;
const RTC_IO_R: u16 = 0x01;
const RTC_REG_SEC: u8 = 0x00;
const RTC_REG_MIN: u8 = 0x02;
const RTC_REG_HOUR: u8 = 0x04;
const RTC_REG_DAY: u8 = 0x07;
const RTC_REG_MONTH: u8 = 0x08;
const RTC_REG_YEAR: u8 = 0x09;
const RTC_REG_CENTURY: u8 = 0x32;
const RTC_REG_STATUS_A: u8 = 0x0a;
const RTC_REG_STATUS_B: u8 = 0x0b;

fn from_bcd(v: u8) -> u8 {
    ((v >> 4) * 10) + (v & 0xf)
}

pub struct Rtc {
    ioport: IoPort,
}

impl Rtc {
    pub fn new() -> Rtc {
        Rtc {
            ioport: IoPort::new(RTC_IO_BASE, 2),
        }
    }

    unsafe fn read_cmos(&self, reg: u8) -> u8 {
        self.ioport.out8(RTC_IO_W, reg);
        self.ioport.in8(RTC_IO_R)
    }

    unsafe fn wait_for_rtc(&self) {
        while self.read_cmos(RTC_REG_STATUS_A) & 0x80 == 0x80 {}
    }

    pub fn read(&self) -> DateTime {
        let mut year: u16;
        let mut century: u8;
        let mut month: u8;
        let mut day: u8;
        let mut hour: u8;
        let mut min: u8;
        let mut sec: u8;
        let status_b: u8;

        unsafe {
            self.wait_for_rtc();
            year = self.read_cmos(RTC_REG_YEAR) as u16;
            century = self.read_cmos(RTC_REG_CENTURY);
            month = self.read_cmos(RTC_REG_MONTH);
            day = self.read_cmos(RTC_REG_DAY);
            hour = self.read_cmos(RTC_REG_HOUR);
            min = self.read_cmos(RTC_REG_MIN);
            sec = self.read_cmos(RTC_REG_SEC);
            status_b = self.read_cmos(RTC_REG_STATUS_B);
        }

        if status_b & 0x04 == 0 {
            // Values are stored in BCD format.
            year = from_bcd(year as u8) as u16;
            century = from_bcd(century);
            month = from_bcd(month);
            day = from_bcd(day);
            hour = from_bcd(hour);
            min = from_bcd(min);
            sec = from_bcd(sec);
        }

        year = (century as u16 * 100) + year;
        DateTime {
            year: year,
            month: month,
            day: day,
            hour: hour,
            min: min,
            sec: sec
        }
    }
}
