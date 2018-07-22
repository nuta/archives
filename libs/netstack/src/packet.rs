use core::slice;
use core::mem::{transmute, size_of};
use core::option::Option;
use core::cell::{RefCell};
use alloc::vec::Vec;
use transport::Transport;
use ip::{Network, IpAddr};

pub struct Packet {
    header: RefCell<Vec<u8>>,
    data: RefCell<Option<Vec<u8>>>,
}

pub struct PacketInfo {
    pub transport: Transport,
    pub network: Network,
    pub dst: IpAddr,
    pub dst_port: u16,
    pub src_port: u16,
}

impl Packet {
    pub fn new() -> Packet {
        Packet {
            header: RefCell::new(Vec::with_capacity(256)),
            data: RefCell::new(None),
        }
    }

    pub fn set_data_from_slice(&self, data: & [u8]) {
        let mut v = Vec::new();
        v.extend_from_slice(data);
        self.data.replace(Some(v));
    }

    pub fn append_header<T: Sized>(&self, header: &T) {
        let ptr = unsafe { transmute::<&T, *const u8>(header) };
        let len = size_of::<T>();
        let slice = unsafe { slice::from_raw_parts(ptr, len) };

        let mut new_header = Vec::new();
        new_header.extend_from_slice(slice);
        new_header.extend_from_slice(self.header.borrow().as_slice());
        self.header.replace(new_header);
    }

    pub fn data_len(&self) -> usize {
        if let Some(ref data) = *self.data.borrow() { data.len() } else { 0 }
    }

    pub fn total_len(&self) -> usize {
        self.data_len() + self.header.borrow().len()
    }

    pub fn as_vec(&self) -> Vec<u8> {
        let mut buf = Vec::with_capacity(self.total_len());
        buf.extend_from_slice(self.header.borrow().as_slice());
        if let Some(ref data) = *self.data.borrow() {
            buf.extend_from_slice(data.as_slice());
        }

        buf
    }
}

impl PacketInfo {
    pub fn new(transport: Transport, network: Network, dst: IpAddr, dst_port: u16, src_port: u16) -> PacketInfo {
        PacketInfo {
            transport: transport,
            network: network,
            dst: dst,
            dst_port: dst_port,
            src_port: src_port,
        }
    }
}
