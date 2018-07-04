use arch::{CId, ipc_open};

#[derive(Debug, Clone)]
pub struct Channel {
    cid: CId
}

impl Channel {
    pub fn create() -> Channel {
        let cid;
        unsafe {
            cid =ipc_open();
        }

        Channel { cid: cid }
    }

    pub fn from_cid(cid: CId) -> Channel {
        Channel { cid: cid }
    }

    pub fn to_cid(&self) -> CId {
        self.cid
    }
}
