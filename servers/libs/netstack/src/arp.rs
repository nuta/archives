use alloc::collections::BTreeMap;
use alloc::vec::Vec;
use core::cell::RefCell;
use core::fmt;
use core::mem::size_of;
use endian::EndianExt;
use ipv4::Ipv4Addr;
use netif::NetIf;
use packet::Packet;
use route::Routes;
use Result;

enum Operation {
    ArpRequest = 1,
    ArpResponse = 2,
}

#[derive(Clone, PartialEq, Eq)]
pub struct MacAddr([u8; 6]);

impl MacAddr {
    pub const BROADCAST: MacAddr = MacAddr([0xff, 0xff, 0xff, 0xff, 0xff, 0xff]);

    pub fn new(addr: &[u8; 6]) -> MacAddr {
        MacAddr(*addr)
    }

    pub fn from_slice(addr: &[u8]) -> MacAddr {
        let mut arr: [u8; 6] = [0; 6];
        arr.copy_from_slice(addr);
        MacAddr::new(&arr)
    }

    pub fn as_slice(&self) -> &[u8; 6] {
        &self.0
    }
}

impl fmt::Display for MacAddr {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
            self.0[0], self.0[1], self.0[2], self.0[3], self.0[4], self.0[5]
        )
    }
}

#[repr(C, packed)]
struct ArpPacket {
    hw_type: u16,
    proto_type: u16,
    hw_len: u8,
    proto_len: u8,
    op: u16,
    sender_hw_addr: [u8; 6],
    sender_ip_addr: u32,
    target_hw_addr: [u8; 6],
    target_ip_addr: u32,
}

pub struct CacheEntry {
    mac_addr: Option<MacAddr>,
    queue: RefCell<Vec<(Ipv4Addr, Packet)>>,
}

pub struct Arp {
    cache: RefCell<BTreeMap<Ipv4Addr, CacheEntry>>,
}

impl Arp {
    pub fn new() -> Arp {
        Arp {
            cache: RefCell::new(BTreeMap::new()),
        }
    }

    pub fn lookup(&self, addr: &Ipv4Addr) -> Option<MacAddr> {
        if *addr == Ipv4Addr::BROADCAST {
            return Some(MacAddr::BROADCAST);
        }

        let cache = self.cache.borrow();
        if let Some(entry) = cache.get(addr) {
            if let Some(ref mac_addr) = entry.mac_addr {
                return Some(mac_addr.clone());
            }
        }

        None
    }

    /* Send an ARP request. */
    pub fn resolve(&self, netif: &NetIf, addr: &Ipv4Addr, pkt: Packet) -> Result<(Packet)> {
        // TODO: implement quota to prevent resource starvation attacks.
        {
            if let Some(ref entry) = self.cache.borrow().get(addr) {
                // Enqueue the packet.

                entry.queue.borrow_mut().push((addr.clone(), pkt));
                return self.construct_packet(netif, addr, &Ipv4Addr::BROADCAST);
            }
        }

        let mut queue = Vec::new();
        queue.push((addr.clone(), pkt));
        let entry = CacheEntry {
            mac_addr: None,
            queue: RefCell::new(queue),
        };
        self.cache.borrow_mut().insert(addr.clone(), entry);
        self.construct_packet(netif, addr, &Ipv4Addr::BROADCAST)
    }

    pub fn receive(&self, routes: &Routes, payload: &[u8]) {
        let (op, sender_ipaddr, sender_macaddr) = {
            if payload.len() < size_of::<ArpPacket>() {
                /* Too short packet. */
                return;
            }

            let header = payload.as_ptr() as *const ArpPacket;
            let op = (unsafe { (*header).op }).to_he();
            let sender_ipaddr = Ipv4Addr::from_u32((unsafe { (*header).sender_ip_addr }).to_he());
            let sender_macaddr = MacAddr::from_slice(&unsafe { (*header).sender_hw_addr });
            (op, sender_ipaddr, sender_macaddr)
        };

        println!(
            "ARP: op={}, sender_ipaddr={:} sender_macaddr={:}",
            op, sender_ipaddr, sender_macaddr
        );
        if op == Operation::ArpResponse as u16 {
            if let Some(entry) = self.cache.borrow_mut().get_mut(&sender_ipaddr) {
                /* Send queued packets. */
                while let Some((dst, pkt)) = (*entry).queue.borrow_mut().pop() {
                    if let Some(route) = routes.route_ipv4(&dst) {
                        route.netif.send(0x800, &sender_macaddr, pkt).ok();
                    }
                }
                (*entry).mac_addr = Some(sender_macaddr);
            }
        }
    }

    fn construct_packet(
        &self,
        netif: &NetIf,
        addr: &Ipv4Addr,
        sender: &Ipv4Addr,
    ) -> Result<Packet> {
        let pkt = Packet::new();
        let payload = Self::construct_ipv4_over_ethernet(
            Operation::ArpRequest,
            &MacAddr::new(&[0, 0, 0, 0, 0, 0]),
            addr,
            &MacAddr::new(&[0, 0, 0, 0, 0, 0]),
            sender,
        );
        pkt.append_header(&payload);

        Ok(pkt)
    }

    fn construct_ipv4_over_ethernet(
        op: Operation,
        target: &MacAddr,
        target_ipaddr: &Ipv4Addr,
        sender: &MacAddr,
        sender_ipaddr: &Ipv4Addr,
    ) -> ArpPacket {
        ArpPacket {
            hw_type: 1u16.to_ne(),         // Ethernet
            proto_type: 0x0800u16.to_ne(), // Ipv4
            hw_len: 6,
            proto_len: 4,
            op: (op as u16).to_ne(),
            sender_hw_addr: *sender.as_slice(),
            sender_ip_addr: sender_ipaddr.as_u32().to_ne(),
            target_hw_addr: *target.as_slice(),
            target_ip_addr: target_ipaddr.as_u32().to_ne(),
        }
    }
}
