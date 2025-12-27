use ftl::collections::vec_deque::VecDeque;
use ftl::prelude::vec::Vec;
use ftl::prelude::*;
use smoltcp::phy::DeviceCapabilities;
use smoltcp::time::Instant;

/// Represents a right to receive a RX packet.
pub struct RxTokenImpl(Vec<u8>);

impl smoltcp::phy::RxToken for RxTokenImpl {
    /// Smoltcp wants to receive a packet.
    fn consume<R, F>(self, f: F) -> R
    where
        F: FnOnce(&[u8]) -> R,
    {
        // Simply pass the buffer to smoltcp.
        f(&self.0)
    }
}

pub struct TxTokenImpl<'a>(&'a mut Device);

impl<'a> smoltcp::phy::TxToken for TxTokenImpl<'a> {
    /// Smoltcp wants to transmit a packet of `len` bytes.
    fn consume<R, F>(self, len: usize, f: F) -> R
    where
        F: FnOnce(&mut [u8]) -> R,
    {
        assert!(len <= self.0.tx_buf.len());

        // Let smoltcp fill a packet to transmit.
        let ret = f(&mut self.0.tx_buf[..len]);

        (self.0.transmit)(&self.0.tx_buf[..len]);
        ret
    }
}

pub type TransmitFn = dyn Fn(&[u8]);

/// A network device implementation for smoltcp.
pub struct Device {
    transmit: Box<TransmitFn>,
    tx_buf: Vec<u8>,
    rx_queue: VecDeque<Vec<u8>>,
}

impl Device {
    pub fn new(transmit: Box<TransmitFn>) -> Device {
        Device {
            transmit,
            tx_buf: vec![0; 1514],
            rx_queue: VecDeque::new(),
        }
    }

    pub fn receive_packet(&mut self, packet: &[u8]) {
        self.rx_queue.push_back(packet.to_vec());
    }
}

impl smoltcp::phy::Device for Device {
    type RxToken<'a> = RxTokenImpl;
    type TxToken<'a> = TxTokenImpl<'a>;

    fn capabilities(&self) -> DeviceCapabilities {
        let mut caps = DeviceCapabilities::default();
        caps.medium = smoltcp::phy::Medium::Ethernet;
        caps.max_transmission_unit = 1514;
        caps
    }

    fn receive(&mut self, _timestamp: Instant) -> Option<(Self::RxToken<'_>, Self::TxToken<'_>)> {
        self.rx_queue
            .pop_front()
            .map(|pkt| (RxTokenImpl(pkt), TxTokenImpl(self)))
    }

    fn transmit(&mut self, _timestamp: Instant) -> Option<Self::TxToken<'_>> {
        Some(TxTokenImpl(self))
    }
}
