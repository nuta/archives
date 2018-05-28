#[macro_export]
macro_rules! print {
    ($($arg:tt)*) => {
        Logging::from_cid(1).emit(format!($($arg)*).as_bytes()).ok();
    };
}

#[macro_export]
macro_rules! println {
    ($fmt:expr) => { print!(concat!($fmt, "\n")); };
    ($fmt:expr, $($arg:tt)*) => { print!(concat!($fmt, "\n"), $($arg)*); };
}
