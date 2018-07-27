use core::fmt;
use core::slice;
use core::mem;
use {Result, Error};
use packet::Packet;
use transport::Transport;
use ip::{IpAddr};
use endian::EndianExt;

const DEFAULT_TTL: u8 = 32;

#[repr(C, packed)]
struct Ipv4Header {
    version: u8,
    ecn: u8,
    length: u16,
    frag_id: u16,
    frag_offset: u16,
    ttl: u8,
    proto: u8,
    checksum: u16,
    src: u32,
    dst: u32,
}

/* Returns in network byte order (assuming that values in `header' is in network byte order). */
fn compute_checksum(header: &Ipv4Header) -> u16 {
    let u16slice = unsafe {
        slice::from_raw_parts(
            header as *const _ as *const u16,
            mem::size_of::<Ipv4Header>() / mem::size_of::<u16>()
        )
    };

    let mut checksum: u32 = 0;
    for &v in u16slice {
        checksum += v as u32;
    }

    /* Add carry. */
    checksum = (checksum >> 16) + (checksum & 0xffff);
    checksum = (checksum >> 16) + (checksum & 0xffff);
    !checksum as u16
}

#[derive(Clone, Ord, PartialOrd, Eq, PartialEq)]
pub struct Ipv4Addr(u32);

impl Ipv4Addr {
    pub const UNSPECIFIED: Ipv4Addr = Ipv4Addr(0);
    pub const BROADCAST: Ipv4Addr = Ipv4Addr(0xffffffff);

    pub fn new(addr: &[u8]) -> Ipv4Addr {
        Ipv4Addr (
            (addr[0] as u32) << 24 |
            (addr[1] as u32) << 16 |
            (addr[2] as u32) << 8  |
            (addr[3] as u32)
        )
    }

    pub fn from_u32(addr: u32) -> Ipv4Addr {
        Ipv4Addr (addr)
    }

    pub fn as_u32(&self) -> u32 {
        self.0
    }
}

impl fmt::Display for Ipv4Addr {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}.{}.{}.{}",
            (self.0 >> 24) & 0xff,
            (self.0 >> 16) & 0xff,
            (self.0 >> 8) & 0xff,
            self.0 & 0xff
        )
    }
}

#[derive(Clone, PartialEq, Eq)]
pub struct Netmask(u32);

impl Netmask {
    pub fn from_u32(netmask: u32) -> Netmask {
        Netmask(netmask)
    }
}

impl fmt::Display for Netmask {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}.{}.{}.{}",
            (self.0 >> 24) & 0xff,
            (self.0 >> 16) & 0xff,
            (self.0 >> 8) & 0xff,
            self.0 & 0xff
        )
    }
}

pub fn construct(pkt: &mut Packet, proto: Transport, dst: &Ipv4Addr, src: &Ipv4Addr) -> Result<()> {
    let mut header = construct_header(dst, src, mem::size_of::<Ipv4Header>() + pkt.total_len(), proto);
    header.checksum = compute_checksum(&header);
    pkt.append_header(&header);
    Ok(())
}

pub fn receive(payload: &[u8]) -> Result<(Transport, IpAddr, IpAddr, usize)> {
    let (proto, src, dst, offset) = {
        if payload.len() < mem::size_of::<Ipv4Header>() {
            /* Too short packet. */
            return Err(Error::MalformedPacket);
        }

        let header = payload.as_ptr() as *const Ipv4Header;
        let proto = unsafe { (*header).proto };
        let src = IpAddr::from_ipv4_addr(Ipv4Addr::from_u32((unsafe { (*header).src }).to_he()));
        let dst = IpAddr::from_ipv4_addr(Ipv4Addr::from_u32((unsafe { (*header).dst }).to_he()));
        let len = unsafe { (*header).length }.to_he();
        let offset = mem::size_of::<Ipv4Header>();

        if payload.len() - offset < (len as usize) - mem::size_of::<Ipv4Header>() {
            /* Bogus length */
            return Err(Error::MalformedPacket);
        }

        (proto, dst, src, offset)
    };

    match proto {
        17 => Ok((Transport::Udp, dst, src, offset)),
        _ => Err(Error::UnknownProtocol),
    }
}

fn construct_header(dst: &Ipv4Addr, src: &Ipv4Addr, len: usize, proto: Transport) -> Ipv4Header {
    Ipv4Header {
        version: 0x45,
        ecn: 0,
        length: (len as u16).to_ne(),
        frag_id: 0,
        frag_offset: 0,
        ttl: DEFAULT_TTL,
        proto: proto as u8,
        checksum: 0,
        src: src.as_u32().to_ne(),
        dst: dst.as_u32().to_ne(),
    }
}
