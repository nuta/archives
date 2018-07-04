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
    pub fn ipc_send(ch: CId, header: Header, a0: Payload, a1: Payload,
                    a2: Payload, a3: Payload) -> Header;
    pub fn ipc_recv(ch: CId, from: *mut CId, a0: *mut Payload, a1: *mut Payload,
                    a2: *mut Payload, a3: *mut Payload) -> Header;
    pub fn ipc_replyrecv(server: CId, client: *mut CId, header: Header, r0: Payload, r1: Payload,
                         r2: Payload, r3: Payload, a0: *mut Payload, a1: *mut Payload,
                         a2: *mut Payload, a3: *mut Payload) -> Header;
}
