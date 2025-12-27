use crate::environ::VSyscallPage;

#[derive(Debug)]
pub struct EnvItem {
    pub name: &'static str,
    pub ty: EnvType,
}

#[derive(Debug)]
pub enum DeviceMatcher {
    DeviceTree { compatible: &'static str },
}

#[derive(Debug)]
pub enum EnvType {
    Service { name: &'static str },
    Device { matcher: DeviceMatcher },
}

#[derive(Debug)]
pub enum ExportItem {
    Service { name: &'static str },
}

#[derive(Debug)]
pub struct AppSpec {
    pub name: &'static str,
    pub start: fn(*const VSyscallPage) -> !,
    pub env: &'static [EnvItem],
    pub exports: &'static [ExportItem],
}
