use core::mem;
use endian::EndianExt;
use packet::Packet;
use {Error, Result};

#[repr(C, packed)]
pub struct UdpHeader {
    src_port: u16,
    dst_port: u16,
    length: u16,
    checksum: u16,
}

pub fn construct(pkt: &mut Packet, dst_port: u16, src_port: u16) -> Result<()> {
    let header = construct_header(
        dst_port,
        src_port,
        mem::size_of::<UdpHeader>() + pkt.data_len(),
    );
    pkt.append_header(&header);
    Ok(())
}

pub fn receive(payload: &[u8]) -> Result<(u16, u16, usize, usize)> {
    if payload.len() < mem::size_of::<UdpHeader>() {
        /* Too short packet. */
        return Err(Error::MalformedPacket);
    }

    let header = payload.as_ptr() as *const UdpHeader;
    let src_port = unsafe { (*header).src_port }.to_he();
    let dst_port = unsafe { (*header).dst_port }.to_he();
    let len = unsafe { (*header).length }.to_he();
    let offset = mem::size_of::<UdpHeader>();

    let payload_len = (len as usize) - mem::size_of::<UdpHeader>();
    if payload.len() - offset < payload_len {
        /* Bogus length */
        return Err(Error::MalformedPacket);
    }

    Ok((src_port, dst_port, offset, payload_len))
}

pub fn construct_header(dst_port: u16, src_port: u16, len: usize) -> UdpHeader {
    UdpHeader {
        src_port: src_port.to_ne(),
        dst_port: dst_port.to_ne(),
        length: (len as u16).to_ne(),
        checksum: 0,
    }
}
