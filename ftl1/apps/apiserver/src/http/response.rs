use ftl::channel::Channel;
use ftl::channel::MESSAGE_DATA_LEN_MAX;
use ftl::channel::Message;
use ftl::error::ErrorCode;
use ftl::eventloop::Event;
use ftl::eventloop::EventLoop;
use ftl::handle::Handleable;
use ftl::poll::Readiness;
use ftl::prelude::*;
use ftl::rc::Rc;

use crate::http::Headers;
use crate::http::StatusCode;

pub trait ResponseWriter {
    fn headers_mut(&mut self) -> &mut Headers;
    fn write_status(&mut self, status: StatusCode);
    fn write_body(&mut self, data: impl AsRef<[u8]>);
    fn end(self);
    fn are_headers_sent(&self) -> bool;
}

pub struct ResponseWriterImpl {
    headers: Headers,
    headers_sent: bool,
    eventloop: EventLoop<()>,
    ch: Rc<Channel>,
}

impl ResponseWriterImpl {
    pub fn new(ch: Rc<Channel>) -> Self {
        let mut eventloop = EventLoop::new().unwrap();

        eventloop.add_channel(ch.clone(), ()).unwrap();
        eventloop
            .listen(ch.handle_id(), Readiness::WRITABLE)
            .unwrap();

        Self {
            ch,
            eventloop,
            headers: Headers::new(),
            headers_sent: false,
        }
    }

    pub fn blocking_write(&mut self, data: impl AsRef<[u8]>) {
        for chunk in data.as_ref().chunks(MESSAGE_DATA_LEN_MAX) {
            loop {
                match self.ch.send(Message::Data { data: chunk }) {
                    Ok(()) => {
                        // Successfully sent, move to next chunk.
                        break;
                    }
                    Err(ErrorCode::Backpressure) => {
                        debug!("TCP channel is full, waiting for it to be writable again");

                        // Wait for the channel to be writable again.
                        let (_, event) = self.eventloop.wait().unwrap();
                        assert!(matches!(event, Event::ChannelWritable { .. }));
                    }
                    Err(err) => {
                        warn!("failed to send body: {:?}", err);
                        return;
                    }
                }
            }
        }
    }
}

impl ResponseWriter for ResponseWriterImpl {
    fn headers_mut(&mut self) -> &mut Headers {
        &mut self.headers
    }

    fn are_headers_sent(&self) -> bool {
        self.headers_sent
    }

    fn write_status(&mut self, status: StatusCode) {
        let mut buf = format!("HTTP/1.1 {}\r\n", status.as_u16());
        buf.push_str("Transfer-Encoding: chunked\r\n");
        for (name, value) in self.headers.iter() {
            if name == &"transfer-encoding" {
                warn!("disallowed response header: {name}");
                continue;
            }

            buf.push_str(name);
            buf.push_str(": ");
            buf.push_str(value);
            buf.push_str("\r\n");
        }
        buf.push_str("\r\n");

        self.blocking_write(buf.as_bytes());
        self.headers_sent = true;
    }

    fn write_body(&mut self, data: impl AsRef<[u8]>) {
        if !self.headers_sent {
            self.write_status(StatusCode::OK);
        }

        let chunk_len = data.as_ref().len();
        if chunk_len == 0 {
            // "0" indicates the end of the body, which will be sent later
            // by `end`.
            return;
        }

        self.blocking_write(format!("{:x}\r\n", chunk_len).as_bytes());
        self.blocking_write(data);
        self.blocking_write("\r\n");
    }

    fn end(mut self) {
        self.blocking_write("0\r\n\r\n");
    }
}
