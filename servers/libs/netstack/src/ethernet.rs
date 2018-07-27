use core::mem::{size_of};
use endian::EndianExt;
use {Result, Error};
use ip::Network;
use arp::MacAddr;

#[repr(C, packed)]
pub struct EthernetHeader {
    dst: [u8; 6],
    src: [u8; 6],
    proto: u16,
}

pub fn receive(frame: &[u8]) -> Result<(Network, MacAddr, MacAddr, usize)> {
    let (proto, src, dst, offset) = {
        if frame.len() < size_of::<EthernetHeader>() {
            /* Too short frame. */
            return Err(Error::MalformedPacket);
        }

        let header = frame.as_ptr() as *const EthernetHeader;
        let proto = (unsafe { (*header).proto }).to_he();
        let dst = MacAddr::from_slice(&unsafe { (*header).dst });
        let src = MacAddr::from_slice(&unsafe { (*header).src });
        let offset = size_of::<EthernetHeader>();
        (proto, src, dst, offset)
    };

    match proto {
        0x806 => Ok((Network::Arp, src, dst, offset)),
        0x800 => Ok((Network::Ipv4, src, dst, offset)),
        _ => Err(Error::UnknownProtocol),
    }
}
