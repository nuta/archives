#[derive(Debug)]
pub enum Uri<'a> {
    TcpListen { host: &'a str, port: u16 },
    Device { mac: [u8; 6] },
}

#[derive(Debug)]
pub enum ParseError {
    MissingScheme,
    UnknownScheme,
    InvalidPort,
    MissingPort,
    InvalidUri,
    InvalidMac,
}

impl<'a> Uri<'a> {
    pub fn parse(uri: &'a [u8]) -> Result<Uri<'a>, ParseError> {
        let uri_str = core::str::from_utf8(uri).map_err(|_| ParseError::InvalidUri)?;
        match uri_str.split_once(':') {
            Some((scheme, rest)) => {
                match scheme {
                    "tcp-listen" => {
                        let (host, port) = rest.split_once(':').ok_or(ParseError::MissingPort)?;
                        let port = port.parse::<u16>().map_err(|_| ParseError::InvalidPort)?;
                        Ok(Uri::TcpListen { host, port })
                    }
                    "ethernet" => {
                        let mut mac = [0u8; 6];
                        let mut idx = 0;
                        for part in rest.split(':') {
                            if idx >= mac.len() {
                                return Err(ParseError::InvalidMac);
                            }

                            if part.len() != 2 {
                                return Err(ParseError::InvalidMac);
                            }

                            let value =
                                u8::from_str_radix(part, 16).map_err(|_| ParseError::InvalidMac)?;
                            mac[idx] = value;
                            idx += 1;
                        }

                        if idx != mac.len() {
                            return Err(ParseError::InvalidMac);
                        }

                        Ok(Uri::Device { mac })
                    }
                    _ => Err(ParseError::UnknownScheme),
                }
            }
            None => Err(ParseError::MissingScheme),
        }
    }
}
