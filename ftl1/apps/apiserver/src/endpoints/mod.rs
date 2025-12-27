mod echo;
mod uptime;

use ftl::prelude::*;

use crate::http::HeaderName;
use crate::http::Method;
use crate::http::Request;
use crate::http::ResponseWriter;
use crate::http::StatusCode;

static INDEX_HTML: &[u8] = include_bytes!("../../shell/index.html");
static INDEX_CSS: &[u8] = include_bytes!("../../shell/main.css");
static INDEX_JS: &[u8] = include_bytes!("../../shell/main.js");

fn serve_static(content_type: &str, body: &[u8], resp: &mut impl ResponseWriter) {
    let headers = resp.headers_mut();
    headers
        .insert(HeaderName::CONTENT_TYPE, content_type)
        .unwrap();

    resp.write_status(StatusCode::OK);
    resp.write_body(body);
}

pub fn route(req: &Request, resp: &mut impl ResponseWriter) {
    match (&req.method, req.path.as_str()) {
        (Method::Get, "/") => serve_static("text/html", INDEX_HTML, resp),
        (Method::Get, "/main.css") => serve_static("text/css", INDEX_CSS, resp),
        (Method::Get, "/main.js") => serve_static("text/javascript", INDEX_JS, resp),
        (Method::Get, "/uptime") => uptime::serve_uptime(resp),
        (Method::Get | Method::Post, "/echo") => echo::serve_echo(req, resp),
        _ => {
            error(resp, StatusCode::new(404).unwrap(), "Route not found");
        }
    }
}

pub fn error(resp: &mut impl ResponseWriter, status: StatusCode, message: &str) {
    if resp.are_headers_sent() {
        // It's too late to send an error response.
        warn!(
            "HTTP error response already sent, cannot send error: {}",
            message
        );
        return;
    }

    let content_length = format!("{}", message.len());
    let headers = resp.headers_mut();
    headers
        .insert(HeaderName::CONTENT_TYPE, "text/plain")
        .unwrap();
    headers
        .insert(HeaderName::CONTENT_LENGTH, content_length.as_str())
        .unwrap();

    resp.write_status(status);
    resp.write_body(message);
}
