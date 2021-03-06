#!/usr/bin/env python3
import argparse
from pprint import pprint
from pathlib import Path
import sys
import jinja2
import colorama
from colorama import Fore, Style
from lark import Lark, Transformer

GRAMMAR = """
ID: /[a-zA-Z_][a-zA-Z0-9_]*/
VALUE: /[a-zA-Z0-9_-][a-zA-Z0-9_-]*/
COMMENT: /\/\/.*/

%import common.WS
%ignore WS
%ignore COMMENT

idl: interface
interface: attrs "interface" ID "{" types messages "}"
attrs: "[" [attr ("," attr)*] "]"
attr: ID "(" VALUE ")"

types: type*
type: [attrs] "type" ID "=" VALUE

messages: message*
message: [attrs] ID args "->" args
args: "(" [arg ("," arg)*] ")"
arg: ID ":" ID
"""

class IdlTransformer(Transformer):
    def message(self, child):
        attrs, name, req, res = child
        return {
            "attrs": attrs,
            "name": str(name),
            "req": req,
            "res": res,
        }

    def type(self, child):
        name, alias_of = child
        return {
            "name": str(name),
            "alias_of": str(alias_of),
        }

    def interface(self, child):
        attrs, name, types, messages = child
        return {
            "name": str(name),
            "attrs": attrs,
            "types": types,
            "messages": messages,
        }

    tokens_to_tuple = lambda _, ts: tuple(map(str, ts))

    types = list
    messages = list
    attrs = dict
    attr = tokens_to_tuple
    args = list
    arg = tokens_to_tuple
    defs = list
    idl = lambda _, child: child[0]

def load_idl(idl):
    ast = Lark(GRAMMAR, start="idl").parse(idl)
    return IdlTransformer().transform(ast)

TEMPLATE = """\
//! Generated by `idl.py`.
#![allow(clippy::all)]
#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]
#![allow(unused_variables)]
#![allow(non_camel_case_types)]

use crate::channel::{Channel, CId};
use crate::message::{Msg, Header, Payload, Page};
use crate::syscalls::Result;

pub const INTERFACE_ID: u8 = {{ interface_id }};

// Types
{% for t in types %}\
pub type {{ t.name }} = {{ t.alias_of }};
{% endfor %}\

// Message ID and headers
{% for m in messages %}\
{% if m.attrs["type"] in ["rpc", "upcall"] %}\
pub const {{ m.name | upper }}_REQUEST_ID: u16  = ((INTERFACE_ID as u16) << 8) | {{ m.attrs["id"] }};
pub const {{ m.name | upper }}_RESPONSE_ID: u16 = ((INTERFACE_ID as u16) << 8) | (1 << 7) | {{ m.attrs["id"] }};
pub const {{ m.name | upper }}_REQUEST_HEADER: usize = {{ m.req | header }} | {{ m.name | upper }}_REQUEST_ID as usize;
pub const {{ m.name | upper }}_RESPONSE_HEADER: usize = {{ m.res | header }} | {{ m.name | upper }}_RESPONSE_ID as usize;
{% endif %}\
{% endfor %}\

// Server trait
pub trait Server {
{% for m in messages %}\
{% if m.attrs["type"] == "rpc" %}\
    fn {{ m.name }}(&mut self{{ m.req | args_list }}) -> Result<({{ m.res | rets_list }})>;
{% endif %}\
{% endfor %}\
}

impl Server {
    pub fn handle(&mut self, m: Msg) -> Option<Result<Msg>> {
        match m.header.msg_id() {
{% for m in messages %}\
{% if m.attrs["type"] == "rpc" %}\
            {{ m.name | upper }}_REQUEST_ID => {
                Some(self.{{ m.name }}({{ m.req | cast_to_args }})
                    .map(|args| { {{ m.res | cast_to_msg(m.name, "args") }} }))
            }
{% endif %}\
{% endfor %}\
            _ => ({ internal_println!("unknown method: {}", m.header.method_id()); None })
        }
    }
}

// Client
pub struct Client {
    server: Channel
}

impl Client {
    pub fn from_channel(server: Channel) -> Client {
        Client { server }
    }

{% for m in messages %}\
{% if m.attrs["type"] == "rpc" %}\
    pub fn {{ m.name }}(&self{{ m.req | args_list }}) -> Result<({{ m.res | rets_list }})> {
        let __r = self.server.call({{ m.req | serialize(m.name) }})?;
        Ok(({{ m.res | deserialize("__r") }}))
    }
{% endif %}\
{% endfor %}\
}
"""

def rename_type(name):
    rename_map = {
        "page": "Page",
    }

    return rename_map.get(name, name)

def args_list(args):
    s = ""
    for name, type_ in args:
        s += f", {name}: {rename_type(type_)}"
    return s

def payloads(rets):
    values = []
    for _,type_ in rets:
        values.append(f"{rename_type(type_)}")
    return ", ".join(values)

def rets_list(rets):
    values = []
    for _,type_ in rets:
        values.append(f"{rename_type(type_)}")
    return ", ".join(values)

def cast_payload(p):
    if len(p) == 0:
        return f"Payload::Inline(0)"
    else:
        name, type_ = p
        return f"{name}.into()"

serializer_env = jinja2.Environment()
serializer_env.filters["cast_payload"] = cast_payload
serializer = serializer_env.from_string("""\
    Msg {
        header: Header::from_usize({{ name | upper }}_REQUEST_HEADER),
        from: Channel::from_cid(CId::new(0)), /* overwrote by `Channell::send` */
        p0: {{ payloads[0] | cast_payload }},
        p1: {{ payloads[1] | cast_payload }},
        p2: {{ payloads[2] | cast_payload }},
        p3: {{ payloads[3] | cast_payload }},
        p4: {{ payloads[4] | cast_payload }},
    }
""")

def serialize(payloads, name):
    return serializer.render(**locals())

def deserialize(payloads, m):
    values = []
    for i, (name,type_) in enumerate(payloads):
        values.append(f"({m}.p{i}).into()")
    return ", ".join(values)

def cast_to_args(payloads):
    args = []
    for i, (_, type_) in enumerate(payloads):
        args.append(f"m.p{i}.into()")
    return ", ".join(args)

def cast_to_msg(payloads, method_name, args):
    s = "Msg { from: Channel::from_cid(CId::new(0)), "
    s += f"header: Header::from_usize(((1 as usize) << 26) /* FIXME: send */ | {method_name.upper()}_RESPONSE_HEADER),"
    for i in range(0, 5):
        if i == 0 and len(payloads) == 1:
            s += f"p{i}: {args}.into(), "
        elif i < len(payloads):
            name, type_ = payloads[i]
            s += f"p{i}: {args}.{i}.into(), "
        else:
            s += f"p{i}: Payload::Inline(0), "
    s += "}"
    return s

TYPES = {}
def load_types(types):
    global TYPES
    for t in types:
        TYPES[t["name"]] = t["alias_of"]

INLINE_TYPES = ["usize", "isize", "u8", "char"]
def resolve_payload_type(t):
    while True:
        if t not in TYPES:
            break
        t = TYPES[t]

    if t in INLINE_TYPES:
        return "inline"

    if t == "page":
        return "page"

    raise InvalidIdlException(f"invalid type: `{t}'")

def header(payloads):
    # TODO: page
    types = 0
    for i, (_, type_) in enumerate(payloads):
        payload_type = resolve_payload_type(type_)
        if payload_type == "inline":
            type_id = 0
        elif payload_type == "page":
            type_id = 1
        else:
            raise InvalidIdlException("unsupported payload type")

        types |= type_id << (2 * i)

    header = types << 16
    return hex(header)

class InvalidIdlException(Exception):
    pass

def validate_idl(idl):
    used_method_ids = []

    if "id" not in idl["attrs"]:
        raise InvalidIdlException("The interface ID must be specified.")

    MSG_ATTRS = ["id", "type"]
    MSG_TYPES = ["rpc", "upcall"]
    for m in idl["messages"]:
        if "id" not in m["attrs"]:
            raise InvalidIdlException("The interface ID must be specified.")
        if m["attrs"]["type"] not in MSG_TYPES:
            raise InvalidIdlException("The message type must be one of: {MSG_TYPES}")
        for attr_name in m["attrs"].keys():
            if attr_name not in MSG_ATTRS:
                raise InvalidIdlException("Unknown message attribute: `{attr_name}'")

        method_id = int(m["attrs"]["id"])
        if method_id in used_method_ids:
            raise InvalidIdlException(f"The duplicated method ID: {method_id} (name={m['name']})")
        if method_id >= 128:
            raise InvalidIdlException(f"Too large method ID: {method_id} (name={m['name']})")
        used_method_ids.append(method_id)


def generate_stub(idl):
    validate_idl(idl)
    load_types(idl["types"])

    env = jinja2.Environment()
    env.filters["args_list"] = args_list
    env.filters["rets_list"] = rets_list
    env.filters["serialize"] = serialize
    env.filters["deserialize"] = deserialize
    env.filters["header"] = header
    env.filters["cast_to_args"] = cast_to_args
    env.filters["cast_to_msg"] = cast_to_msg

    return env.from_string(TEMPLATE).render(
        interface_id=idl["attrs"]["id"],
        interface_name=idl["name"],
        types=idl["types"],
        messages=idl["messages"],
    )

def main():
    parser = argparse.ArgumentParser(description="Resea Interface stub generator.")
    parser.add_argument("-o", dest="output", required=True, help="The output file.")
    parser.add_argument("idl_file", help="A idl file.")
    args = parser.parse_args()

    try:
        idl = load_idl(open(args.idl_file).read())
        stub = generate_stub(idl)
    except InvalidIdlException as e:
        sys.exit(f"{Style.BRIGHT}idl.py: {Fore.RED}error:{Fore.RESET} {args.idl_file}: {e}")

    with open(args.output, "w") as f:
        f.write(stub)

if __name__ == "__main__":
    colorama.init(autoreset=True)
    main()