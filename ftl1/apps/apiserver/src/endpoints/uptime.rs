use ftl::prelude::*;
use ftl::timer;

use crate::http::HeaderName;
use crate::http::ResponseWriter;
use crate::http::StatusCode;

pub fn serve_uptime(resp: &mut impl ResponseWriter) {
    let uptime = timer::now().elapsed_since(*crate::STARTED_AT);
    info!("uptime: {}", uptime.as_nanos());

    let body = format!("{}", uptime.as_millis());

    resp.write_status(StatusCode::OK);
    resp.headers_mut()
        .insert(HeaderName::CONTENT_TYPE, "text/plain")
        .unwrap();
    resp.write_body(body.as_bytes());
}
