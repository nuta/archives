#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use {Channel, Result as ServerResult, OoL, ErrorCode, CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
use interfaces::discovery;
use syscalls::{ipc_open, ipc_call, ipc_send, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = 1000;
pub const BENCHMARK_SERVICE: u16 = 1000;

pub struct Benchmark {
    cid: CId
}

impl Benchmark {
    pub fn from_cid(cid: CId) -> Benchmark {
        Benchmark {
            cid: cid
        }
    }

    pub fn from_channel(ch: &Channel) -> Benchmark {
        Benchmark {
            cid: ch.to_cid()
        }
    }

    pub fn connect() -> Benchmark {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover(BENCHMARK_SERVICE).unwrap();
        Benchmark {
            cid: ch.to_cid()
        }
    }

    // Stubs
}

pub trait Server {
}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        match header.msg_type() {
            _ => {
                ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
        }
    }
}