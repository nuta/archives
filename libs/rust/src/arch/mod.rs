global_asm!(include_str!("../../../resea/arch/x64/syscall.S"));

#[cfg(target_arch="x86_64")]
pub mod x64;

pub type CId = i64;
pub type Header = u64;
pub type Payload = u64;

extern "C" {
    pub fn ipc_open() -> CId;
    pub fn ipc_close(cid: CId);

    pub fn ipc_call(ch: CId, header: Header, a0: Payload, a1: Payload,
                    a2: Payload, a3: Payload, r0: *mut Payload, r1: *mut Payload,
                    r2: *mut Payload, r3: *mut Payload) -> Header;

    /*
    pub fn Header ipc_send(ch: Cid, Header type, a0: Payload, a1: Payload,
                           a2: Payload, a3: Payload);
    pub fn Header ipc_recv(ch: Cid, CId *mut from, *mut a0: Payload, *mut a1: Payload,
                           *mut a2: Payload, *mut a3: Payload);
    pub fn Header ipc_replyrecv(CId client, Header type, r0: Payload, r1: Payload,
                                r2: Payload, r3: Payload, *mut a0: Payload, *mut a1: Payload,
                                *mut a2: Payload, *mut a3: Payload);
    */
}
