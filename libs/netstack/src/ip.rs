use ipv4::Ipv4Addr;

#[derive(Debug, Clone)]
pub enum Network {
    Ipv4 = 0x0800,
    Arp = 0x0806,
}

pub struct IpAddr {
    network: Network,
    ipv4_addr: Ipv4Addr,
}

impl IpAddr {
    pub fn from_ipv4_addr(addr: Ipv4Addr) -> IpAddr {
        IpAddr {
            network: Network::Ipv4,
            ipv4_addr: addr,
        }
    }

    pub fn network(&self) -> &Network {
        &self.network
    }

    pub fn ipv4_addr(&self) -> &Ipv4Addr {
        &self.ipv4_addr
    }
}
