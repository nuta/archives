use {Result, Error};
use resea::interfaces::net_device::NetDevice;
use arp::{MacAddr};
use packet::Packet;
use endian::EndianExt;

#[repr(C, packed)]
pub struct EthernetHeader {
    dst: [u8; 6],
    src: [u8; 6],
    proto: u16,
}

pub enum NetIfType {
    Ethernet,
}

pub struct NetIf {
    #[allow(dead_code)]
    type_: NetIfType,
    device: NetDevice,
    pub mac_addr: MacAddr,
}

impl NetIf {
    pub fn new(type_: NetIfType, device: NetDevice, mac_addr: MacAddr) -> NetIf {
        NetIf {
            type_: type_,
            device: device,
            mac_addr: mac_addr,
        }
    }

    pub fn send(&self, proto: u16, dst: &MacAddr, pkt: Packet) -> Result<()> {
        let checksum = Self::compute_checksum(&pkt);
        let header = Self::construct_header(dst, &self.mac_addr, proto);
        pkt.append_header(&header);
        match self.device.send(pkt.as_vec().as_slice()) {
            Ok(_) => Ok(()),
            Err(_) => Err(Error::DeviceError),
        }
    }

    fn construct_header(dst: &MacAddr, src: &MacAddr, proto: u16) -> EthernetHeader {
        EthernetHeader {
            dst: *dst.as_slice(),
            src: *src.as_slice(),
            proto: proto.to_ne(),
        }
    }

    fn compute_checksum(pkt: &Packet) -> u16 {
        0
    }
}
