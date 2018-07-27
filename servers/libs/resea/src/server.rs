#[macro_export]
macro_rules! register_as {
    ($server:expr, [$($service:ident), *]) => {
        let discovery = resea::interfaces::discovery::Discovery::from_cid(1);
        $(
            discovery.register($service::SERVICE_ID, $server.ch.clone()).expect("discovery.register error");
        )*
    }
}

#[macro_export]
macro_rules! serve_forever {
    ($server:expr, [$($service:ident), *]) => {
        use resea::{ErrorCode};
        use resea::arch::x64::prelude::{CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
        use resea::syscalls::{ipc_recv, ipc_replyrecv};

        let mut header: Header;
        let mut from: CId = 0;
        let mut a0: Payload = 0;
        let mut a1: Payload = 0;
        let mut a2: Payload = 0;
        let mut a3: Payload = 0;
        let mut r0: Payload = 0;
        let mut r1: Payload = 0;
        let mut r2: Payload = 0;
        let mut r3: Payload = 0;

        unsafe {
            header = ipc_recv($server.ch.to_cid(), &mut from,
                &mut a0 as *mut Payload, &mut a1 as *mut Payload, &mut a2 as *mut Payload, &mut a3 as *mut Payload);
        }

        loop {
            let from_ch = Channel::from_cid(from);

            match header.service_type() {
                $(
                    $service::SERVICE_ID => {
                       let (r_header, r_r0, r_r1, r_r2, r_r3) = $service::Server::handle($server, from_ch, header, a0, a1, a2, a3);
                       header = r_header;
                       r0 = r_r0;
                       r1 = r_r1;
                       r2 = r_r2;
                       r3 = r_r3;
                    },
                )*
                _ => { header = (ErrorCode::NotImplemented as u64) << ERROR_OFFSET; }, /* Unknown message. */
            }

            if header.error_type() == ErrorCode::DontReply as u8 {
                unsafe {
                    header = ipc_recv($server.ch.to_cid(), &mut from,
                        &mut a0 as *mut Payload, &mut a1 as *mut Payload,
                        &mut a2 as *mut Payload, &mut a3 as *mut Payload);
                }
            } else {
                unsafe {
                    header = ipc_replyrecv(&mut from, header, r0, r1, r2, r3,
                        &mut a0 as *mut Payload, &mut a1 as *mut Payload,
                        &mut a2 as *mut Payload, &mut a3 as *mut Payload);
                }
            }
        }
    };
}
