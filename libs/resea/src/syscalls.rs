use self::super::{CId, Header, Payload};

extern "C" {
    pub fn ipc_open() -> CId;
    pub fn ipc_close(cid: CId);
    pub fn ipc_transfer(ch: CId, dest: CId);

    pub fn ipc_call(ch: CId, header: Header, a0: Payload, a1: Payload,
                    a2: Payload, a3: Payload, r0: *mut Payload, r1: *mut Payload,
                    r2: *mut Payload, r3: *mut Payload) -> Header;
    pub fn ipc_send(ch: CId, header: Header, a0: Payload, a1: Payload,
                    a2: Payload, a3: Payload) -> Header;
    pub fn ipc_recv(ch: CId, from: *mut CId, a0: *mut Payload, a1: *mut Payload,
                    a2: *mut Payload, a3: *mut Payload) -> Header;
    pub fn ipc_replyrecv(client: *mut CId, header: Header, r0: Payload, r1: Payload,
                         r2: Payload, r3: Payload, a0: *mut Payload, a1: *mut Payload,
                         a2: *mut Payload, a3: *mut Payload) -> Header;
}
