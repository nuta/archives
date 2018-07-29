use self::super::CId;
use syscalls::{ipc_open, ipc_transfer};

#[derive(Debug, Clone)]
pub struct Channel {
    cid: CId,
}

impl Channel {
    pub fn create() -> Channel {
        let cid = unsafe { ipc_open() };
        Channel { cid: cid }
    }

    pub fn from_cid(cid: CId) -> Channel {
        Channel { cid: cid }
    }

    pub fn to_cid(&self) -> CId {
        self.cid
    }

    pub fn transfer_to(&self, dest: &Channel) {
        unsafe {
            ipc_transfer(self.cid, dest.cid);
        }
    }
}
