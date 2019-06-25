use std::fs::OpenOptions;
use std::io::prelude::*;
use std::sync::Mutex;
use std::sync::mpsc::{Sender, channel};
use log::{Record, Metadata, Level, LevelFilter};
use termion::style::{Bold, Reset};
use termion::color;
use lazy_static::lazy_static;

struct Logger {
    tx: Mutex<Sender<(log::Level, String, String)>>,
}

impl log::Log for Logger {
    fn enabled(&self, _metadata: &Metadata) -> bool {
        true
    }

    fn log(&self, record: &Record) {
        if self.enabled(record.metadata()) {
            let tx = self.tx.lock().unwrap();
            let level = record.level();
            let target = record.target().to_owned();

            use std::fmt::Write;
            let mut args = String::new();
            write!(args, "{}", record.args()).ok();

            tx.send((level, target, args)).ok();
        }
    }

    fn flush(&self) {
        // Do nothing.
    }
}

lazy_static! {
    static ref GLOBAL_LOGGER: Logger = {
        let mut log_file_path = dirs::home_dir().unwrap();
        log_file_path.push(".latte.log");

        let mut file = OpenOptions::new()
            .create(true)
            .truncate(false)
            .append(true)
            .open(log_file_path)
            .unwrap();

        let (tx, rx) = channel();
        std::thread::spawn(move || {
            loop {
                let (level, target, args) = rx.recv().unwrap();
                let mut level_color = String::new();
                match level {
                    Level::Info  => {
                        use std::fmt::Write;
                        write!(level_color, "{}", color::Fg(color::Blue)).ok();
                    },
                    Level::Error => {
                        use std::fmt::Write;
                        write!(level_color, "{}", color::Fg(color::LightRed)).ok();
                    },
                    Level::Warn => {
                        use std::fmt::Write;
                        write!(level_color, "{}", color::Fg(color::LightMagenta)).ok();
                    },
                    Level::Debug => {
                        use std::fmt::Write;
                        write!(level_color, "{}", color::Fg(color::Green)).ok();
                    },
                    _ => {
                        use std::fmt::Write;
                        write!(level_color, "{}", color::Fg(color::Reset)).ok();
                    },
                }

                use std::fmt::Write;
                let mut buf = String::new();
                writeln!(
                    buf,
                    "{}{}[{}]{} {}{}: {}",
                    Bold,
                    level_color,
                    level,
                    color::Fg(color::LightBlack),
                    target,
                    Reset,
                    args
                ).ok();

                file.write(buf.as_bytes()).ok();
            }
        });

        Logger {
            tx: Mutex::new(tx),
        }
    };
}

pub fn init() {
    log::set_logger(&*GLOBAL_LOGGER).unwrap();
    log::set_max_level(LevelFilter::Trace);
}
