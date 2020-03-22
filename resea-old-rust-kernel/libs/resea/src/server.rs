#[macro_export]
macro_rules! serve_forever {
    ($server:expr, [$($interface:ident), *]) => {
        loop {
            let m = $server.ch.recv().expect("failed to recv");
            let reply_to = m.from.clone();
            let r = match m.header.interface_id() {
                $(
                resea::idl::$interface::INTERFACE_ID => resea::idl::$interface::Server::handle($server, m),
                )*
                _ => {
                    println!("{}: unknown message: {:04x}", env!("PROGRAM_NAME"), m.header.msg_id());
                    None
                }
            };

            if let Some(Ok(r)) = r {
                if let Err(err) = reply_to.send(r) {
                    println!("{}: wanrning: failed to reply: {:?}",
                        env!("PROGRAM_NAME"), err);
                }
            }
        }
    };
}