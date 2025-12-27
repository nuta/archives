use crate::http::Body;
use crate::http::Request;
use crate::http::ResponseWriter;
use crate::http::StatusCode;

const MAX_BODY_SIZE: usize = 256 * 1024;

pub fn serve_echo(req: &Request, resp: &mut impl ResponseWriter) {
    match &req.body {
        Body::Full(body) => {
            if body.len() > MAX_BODY_SIZE {
                resp.write_status(StatusCode::CONTENT_TOO_LARGE);
                return;
            }

            resp.write_status(StatusCode::OK);
            resp.write_body(body);
        }
    }
}
