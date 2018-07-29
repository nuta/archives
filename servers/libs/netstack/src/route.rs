use alloc::rc::Rc;
use alloc::vec::Vec;
use arp::MacAddr;
use core::cell::RefCell;
use ip::{IpAddr, Network};
use ipv4::{Ipv4Addr, Netmask};
use netif::NetIf;

#[derive(Clone)]
pub struct Route {
    pub dst: Ipv4Addr,
    pub netmask: Netmask,
    pub src: Ipv4Addr,
    pub netif: Rc<NetIf>,
}

pub struct Routes {
    routes: RefCell<Vec<Route>>,
}

impl Routes {
    pub fn new() -> Routes {
        Routes {
            routes: RefCell::new(Vec::new()),
        }
    }

    pub fn add_ipv4(&self, netif: Rc<NetIf>, dst: Ipv4Addr, netmask: Netmask, src: Ipv4Addr) {
        let route = Route {
            dst: dst,
            netmask: netmask,
            src: src,
            netif: netif,
        };

        self.routes.borrow_mut().push(route);
    }

    pub fn get_by_mac_addr(&self, mac_addr: &MacAddr) -> Option<Route> {
        for route in &*self.routes.borrow() {
            if *mac_addr == route.netif.mac_addr {
                return Some(route.clone());
            }
        }

        None
    }

    pub fn route_ipv4(&self, dst: &Ipv4Addr) -> Option<Route> {
        for route in &*self.routes.borrow() {
            return Some(route.clone());
        }

        None
    }

    pub fn is_ours(&self, addr: &IpAddr) -> bool {
        match addr.network() {
            Network::Ipv4 => {
                if *addr.ipv4_addr() == Ipv4Addr::BROADCAST {
                    return true;
                }

                for route in &*self.routes.borrow() {
                    if *addr.ipv4_addr() == route.src {
                        return true;
                    }
                }
            }
            _ => (),
        };

        return false;
    }
}
