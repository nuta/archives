#![no_std]
#![feature(start)]

#[macro_use]
extern crate resea;
use resea::arch::x64::{IoPort};

#[derive(Debug)]
struct DateTime {
    year: u16,
    month: u8,
    day: u8,
    hour: u8,
    min: u8,
    sec: u8
}

const IOPORT_RTC_W: u16 = 0x70;
const IOPORT_RTC_R: u16 = 0x71;
const RTC_REG_SEC: u8 = 0x00;
const RTC_REG_MIN: u8 = 0x02;
const RTC_REG_HOUR: u8 = 0x04;
const RTC_REG_DAY: u8 = 0x07;
const RTC_REG_MONTH: u8 = 0x08;
const RTC_REG_YEAR: u8 = 0x09;
const RTC_REG_CENTURY: u8 = 0x32;
const RTC_REG_STATUS_A: u8 = 0x0a;
const RTC_REG_STATUS_B: u8 = 0x0b;

unsafe fn read_cmos(reg: u8) -> u8 {
    let wio = IoPort::new(IOPORT_RTC_W);
    let rio = IoPort::new(IOPORT_RTC_R);

    wio.out8(reg);
    rio.in8()
}

unsafe fn wait_for_rtc() {
    while read_cmos(RTC_REG_STATUS_A) & 0x80 == 0x80 {}
}

fn from_bcd(v: u8) -> u8 {
    ((v >> 4) * 10) + (v & 0xf)
}

fn read_datetime() -> DateTime {
    let mut year: u16;
    let mut century: u8;
    let mut month: u8;
    let mut day: u8;
    let mut hour: u8;
    let mut min: u8;
    let mut sec: u8;
    let status_b: u8;

    unsafe {
        wait_for_rtc();
        year = read_cmos(RTC_REG_YEAR) as u16;
        century = read_cmos(RTC_REG_CENTURY);
        month = read_cmos(RTC_REG_MONTH);
        day = read_cmos(RTC_REG_DAY);
        hour = read_cmos(RTC_REG_HOUR);
        min = read_cmos(RTC_REG_MIN);
        sec = read_cmos(RTC_REG_SEC);
        status_b = read_cmos(RTC_REG_STATUS_B);
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


#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    println!("rtc: starting rtc driver...");

    let date = read_datetime();
    println!("date: {:?}", date);

    loop {}
    0
}
