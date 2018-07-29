use alloc::rc::Rc;
use alloc::vec::Vec;
use arp::MacAddr;
use core::cell::Cell;
use core::mem;
use core::slice;
use endian::EndianExt;
use ip::IpAddr;
use ipv4::{Ipv4Addr, Netmask};
use netif::NetIf;
use route::Route;
use socket::Socket;
use {Error, Result};

#[derive(Clone, Copy)]
pub enum DhcpClientState {
    BeforeDiscover,
    WaitingForOffer,
    WaitingForAck,
    Done,
}

pub struct DhcpClient {
    state: Cell<DhcpClientState>,
    sock: Rc<Socket>,
    route: Route,
    xid: DhcpTransactionId,
}

impl DhcpClient {
    pub fn new(xid: DhcpTransactionId, sock: Rc<Socket>, netif: Rc<NetIf>) -> DhcpClient {
        let route = Route {
            dst: (Ipv4Addr::BROADCAST).clone(),
            netmask: Netmask::from_u32(0),
            src: (Ipv4Addr::UNSPECIFIED).clone(),
            netif: netif,
        };

        DhcpClient {
            xid: xid,
            state: Cell::new(DhcpClientState::BeforeDiscover),
            sock: sock,
            route: route,
        }
    }

    pub fn start(&self) -> Result<()> {
        self.discover()
    }

    pub fn input(&self, payload: &[u8]) -> Result<(Ipv4Addr, Netmask, Ipv4Addr)> {
        if let Some(msg) = DhcpMessage::from_payload(payload) {
            let mut msg_type = DhcpMessageType::Unknown as u8;
            let mut netmask = None;
            let mut gateway = None;
            for opt in msg.options.iter() {
                match opt.opttype {
                    OPT_TYPE => {
                        if opt.data.len() == 1 {
                            msg_type = opt.data[0];
                        };
                    }
                    OPT_SUBNET_MASK => {
                        if opt.data.len() == 4 {
                            let value = (unsafe {
                                *mem::transmute::<*const u8, *const u32>(opt.data.as_ptr())
                            }).to_he();
                            netmask = Some(Netmask::from_u32(value));
                        };
                    }
                    OPT_ROUTER => {
                        if opt.data.len() == 4 {
                            let value = (unsafe {
                                *mem::transmute::<*const u8, *const u32>(opt.data.as_ptr())
                            }).to_he();
                            gateway = Some(Ipv4Addr::from_u32(value));
                        };
                    }
                    _ => (),
                }
            }

            println!("DHCP: type={}", msg_type);
            match self.state.get() {
                DhcpClientState::WaitingForOffer if msg_type == DhcpMessageType::Offer as u8 => {
                    if let Some(addr) = msg.your_ipaddr {
                        self.request(&addr).ok();
                    }
                }
                DhcpClientState::WaitingForAck if msg_type == DhcpMessageType::Ack as u8 => {
                    if let Some(addr) = msg.your_ipaddr {
                        if netmask.is_none() {
                            print!("DHCP: netmask is not provided.");
                            return Err(Error::InvalidResponse);
                        }

                        if gateway.is_none() {
                            print!("DHCP: router is not provided.");
                            return Err(Error::InvalidResponse);
                        }

                        self.state.replace(DhcpClientState::Done);
                        return Ok((addr, netmask.unwrap(), gateway.unwrap()));
                    }
                }
                _ => (), /* Invalid. */
            }

            Err(Error::Unfinished)
        } else {
            Err(Error::MalformedPacket)
        }
    }

    fn discover(&self) -> Result<()> {
        let msg = DhcpMessage::new(
            self.xid,
            &self.route.netif.mac_addr,
            &[
                DhcpOption::new(OPT_TYPE, &[DhcpMessageType::Discover as u8]),
                DhcpOption::new(OPT_PARAMETER_LIST, &DHCP_PARAMS),
            ],
        );

        println!("DHCP: sending discover");
        self.state.replace(DhcpClientState::WaitingForOffer);
        self.sock.do_sendto(
            Some(&self.route),
            IpAddr::from_ipv4_addr(Ipv4Addr::BROADCAST.clone()),
            67,
            msg.as_slice(),
        )
    }

    fn request(&self, requested_addr: &Ipv4Addr) -> Result<()> {
        let addr_arr = unsafe { mem::transmute::<u32, [u8; 4]>(requested_addr.as_u32().to_ne()) };

        let msg = DhcpMessage::new(
            self.xid,
            &self.route.netif.mac_addr,
            &[
                DhcpOption::new(OPT_TYPE, &[DhcpMessageType::Request as u8]),
                DhcpOption::new(DHCP_REQUESTED_IP_ADDR, &addr_arr),
                DhcpOption::new(OPT_PARAMETER_LIST, &DHCP_PARAMS),
            ],
        );

        println!("DHCP: sending request");
        self.state.replace(DhcpClientState::WaitingForAck);
        self.sock.do_sendto(
            Some(&self.route),
            IpAddr::from_ipv4_addr(Ipv4Addr::BROADCAST.clone()),
            67,
            msg.as_slice(),
        )
    }
}

const DHCP_MAGIC: u32 = 0x63825363;

/* subnet mask, router, DNS, domain name */
const DHCP_PARAMS: [u8; 4] = [1, 3, 6, 5];

enum DhcpMessageType {
    Unknown = 0,
    Discover = 1,
    Offer = 2,
    Request = 3,
    Ack = 5,
}

struct DhcpMessage<'a> {
    data: Vec<u8>,
    options: Vec<DhcpOption<'a>>,
    your_ipaddr: Option<Ipv4Addr>,
}

impl<'a> DhcpMessage<'a> {
    pub fn new(
        xid: DhcpTransactionId,
        macaddr: &MacAddr,
        options: &[DhcpOption<'a>],
    ) -> DhcpMessage<'a> {
        let header = DhcpHeader {
            op: 1,    /* Boot Request */
            htype: 1, /* Ethernet */
            hlen: 6,
            hops: 0,
            transaction_id: (xid as u32).to_ne(),
            seconds: 0,
            flags: 0,
            client_ipaddr: 0,
            your_ipaddr: 0,
            next_server_ipaddr: 0,
            relay_agent_ipaddr: 0,
            client_macaddr: *macaddr.as_slice(),
            padding: [0; 202],
            magic: DHCP_MAGIC.to_ne(),
        };

        let mut data = Vec::new();
        data.extend_from_slice(unsafe { struct_to_slice::<DhcpHeader>(&header) });

        for opt in options {
            data.extend_from_slice(&[opt.opttype as u8, opt.data.len() as u8]);
            data.extend_from_slice(opt.data);
        }

        data.extend_from_slice(&[OPT_END, 0]);

        DhcpMessage {
            data: data,
            options: options.to_vec(),
            your_ipaddr: None,
        }
    }

    pub fn from_payload(payload: &'a [u8]) -> Option<DhcpMessage<'a>> {
        if payload.len() < mem::size_of::<DhcpHeader>() {
            return None;
        }

        let header = unsafe { mem::transmute::<*const u8, *const DhcpHeader>(payload.as_ptr()) };
        if unsafe { (*header).magic.to_he() } != DHCP_MAGIC {
            return None;
        }

        let your_ipaddr = Some(Ipv4Addr::from_u32(unsafe { (*header).your_ipaddr }.to_ne()));

        let mut data = Vec::new();
        data.extend_from_slice(&payload);

        let mut options = Vec::new();
        let mut p = &payload[mem::size_of::<DhcpHeader>()..];
        while p.len() >= 2 {
            let opt_type = p[0] as u8;
            let opt_len = p[1] as usize;
            if p.len() < 2 + opt_len {
                // Bogus length.
                continue;
            }

            let opt_data = &p[2..(2 + opt_len)];
            p = &p[(2 + opt_len)..];
            options.push(DhcpOption::new(opt_type, opt_data))
        }

        let msg = DhcpMessage {
            data: data,
            your_ipaddr: your_ipaddr,
            options: options,
        };

        Some(msg)
    }

    pub fn as_slice(&'a self) -> &'a [u8] {
        self.data.as_slice()
    }
}

unsafe fn struct_to_slice<'a, T>(st: &'a T) -> &'a [u8] {
    slice::from_raw_parts(st as *const _ as *const u8, mem::size_of::<DhcpHeader>())
}

const OPT_END: u8 = 255;
const OPT_TYPE: u8 = 53;
const OPT_PARAMETER_LIST: u8 = 55;
const OPT_SUBNET_MASK: u8 = 1;
const OPT_ROUTER: u8 = 3;
const DHCP_REQUESTED_IP_ADDR: u8 = 50;

#[derive(Clone)]
struct DhcpOption<'a> {
    opttype: u8,
    data: &'a [u8],
}

impl<'a> DhcpOption<'a> {
    pub fn new(opttype: u8, data: &'a [u8]) -> DhcpOption<'a> {
        DhcpOption {
            opttype: opttype,
            data: data,
        }
    }
}

pub type DhcpTransactionId = u32;

#[repr(C, packed)]
pub struct DhcpHeader {
    op: u8,
    htype: u8,
    hlen: u8,
    hops: u8,
    transaction_id: u32,
    seconds: u16,
    flags: u16,
    client_ipaddr: u32,
    your_ipaddr: u32,
    next_server_ipaddr: u32,
    relay_agent_ipaddr: u32,
    client_macaddr: [u8; 6],
    padding: [u8; 202],
    magic: u32, /* 0x63825363 */
}
