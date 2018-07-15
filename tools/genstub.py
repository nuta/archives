#!/usr/bin/env python3
import argparse
from glob import glob
import os
from string import Template
import sys
import jinja2
import idl

def write_file(filepath, body):
    os.makedirs(os.path.dirname(filepath), exist_ok=True)

    with open(filepath, "w") as f:
        f.write(body)

INLINE_PAYLOAD = 0
OOL_PAYLOAD = 1
CHANNEL_PAYLOAD = 2

def get_type_id_by_name(types, name):
    name = types.get(name, name)
    return {
        "channel": CHANNEL_PAYLOAD,
        "string": OOL_PAYLOAD,
        "buffer": OOL_PAYLOAD,
        "usize": INLINE_PAYLOAD,
        "umax": INLINE_PAYLOAD,
        "imax": INLINE_PAYLOAD,
        "error": INLINE_PAYLOAD,
        "i8": INLINE_PAYLOAD,
        "i16": INLINE_PAYLOAD,
        "i32": INLINE_PAYLOAD,
        "i64": INLINE_PAYLOAD,
        "u8": INLINE_PAYLOAD,
        "u16": INLINE_PAYLOAD,
        "u32": INLINE_PAYLOAD,
        "u64": INLINE_PAYLOAD,
        "uptr": INLINE_PAYLOAD
    }[name]

C_TEMPLATE = """\
#ifndef __RESEA_STUB_{{ interface_name | upper }}_H__
#define __RESEA_STUB_{{ interface_name | upper }}_H__

#define {{ interface_name | upper }}_SERVICE ({{interface_id }}U)

{%- for call in calls %}
#define {{ interface_name | upper }}_{{ call["name"] | upper }}_MSG          (({{ interface_name | upper }}_SERVICE << 8) | {{ call["attrs"]["id"] }}UL)
#define {{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_MSG    (({{ interface_name | upper }}_SERVICE << 8) | ({{ call["attrs"]["id"] }}UL + 1))
#define {{ interface_name | upper }}_{{ call["name"] | upper }}_HEADER       (({{ interface_name | upper }}_{{ call["name"] | upper }}_MSG << 16UL) | (({{ call["args"] | header }}) << 8))
#define {{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_HEADER (({{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_MSG << 16UL) | (({{ call["rets"] | header }}) << 8))
static inline header_t call_{{ interface_name }}_{{ call["name"] }}(channel_t __server{{ call | c_args }}) {
    payload_t __tmp;

    return ipc_call(__server, {{ interface_name | upper }}_{{ call["name"] | upper }}_HEADER{{ call | c_params }});
}
{% endfor %}

#endif

"""

def generate_c_file(types, interface):
    def rename_type(type_):
        return type_ + "_t"

    def header(args):
        value = 0
        for i, arg in enumerate(args):
            type_id = get_type_id_by_name(types, arg["type"])
            value |= type_id << (i * 2)
        return str(value)

    def c_args(call):
        s = ""
        for arg in call["args"]:
            s += f", {rename_type(arg['type'])} {arg['name']}"
        for ret in call["rets"]:
            s += f", {rename_type(ret['type'])}* {ret['name']}"
        return s

    def c_params(call):
        s = ""
        for arg in call["args"]:
            s += f", (payload_t) {arg['name']}"

        # Append unused message payloads.
        for _ in range(len(call["args"]), 4):
            s += ", 0"

        for ret in call["rets"]:
            s += f", (payload_t *) {ret['name']}"

        # Handle unused message payloads.
        for _ in range(len(call["rets"]), 4):
            s += ", &__tmp"

        return s

    env = jinja2.Environment()
    env.filters["header"] = header
    env.filters["c_args"] = c_args
    env.filters["c_params"] = c_params

    return env.from_string(C_TEMPLATE).render(
        interface_name=interface["name"],
        interface_id=interface["attrs"]["id"],
        calls=interface["calls"]
    )

def generate_c_stubs(out_dir, idl_files):
    for idl_file in idl_files:
        try:
            defs = idl.parse(idl_file)
        except Exception as e:
            sys.exit(f"failed to parse {idl_file}:" + str(e))

        types = defs["types"]
        for interface in defs["interfaces"]:
            stub = generate_c_file(types, interface)
            write_file(os.path.join(out_dir, f"resea/{interface['name']}.h"), stub)

RUST_TEMPLATE = """\
#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use server::{ServerResult};
use channel::{Channel};
use interfaces::discovery;
use arch::{CId, Payload, OoL, Header, HeaderTrait, ErrorCode, ERROR_OFFSET};
use arch::{ipc_open, ipc_call, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = {{ interface_id }};
pub const {{interface_name | upper }}_SERVICE: u16 = {{ interface_id }};
{%- for call in calls %}
pub const {{ interface_name | upper }}_{{ call["name"] | upper }}_MSG: u16 = ({{ interface_id | upper }}_SERVICE  << 8) | {{ call["attrs"]["id"] }}u16;
pub const {{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_MSG: u16 = ({{ interface_id | upper }}_SERVICE  << 8) | {{ call["attrs"]["id"] }}u16 + 1;
pub const {{ interface_name | upper }}_{{ call["name"] | upper }}_HEADER: u64 = (({{ interface_name | upper }}_{{ call["name"] }}_MSG as u64) << 16) | {{ call["args"] | header }}u64 << 8);
pub const {{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_HEADER: u64 = (({{ interface_name | upper }}_{{ call["name"] }}_REPLY_MSG as u64) << 16) | {{ call["rets"] | header }}u64 << 8);
{% endfor %}

pub struct {{ interface_name | camel }} {
    cid: CId
}

impl {{ interface_name | camel }} {
    pub fn from_cid(cid: CId) -> {{ interface_name | camel }} {
        {{ interface_name | camel }} {
            cid: cid
        }
    }

    pub fn from_channel(ch: &Channel) -> {{ interface_name | camel }} {
        {{ interface_name | camel }} {
            cid: ch.to_cid()
        }
    }

    pub fn connect() -> {{ interface_name | camel }} {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover({{ interface_id | upper }}_SERVICE).unwrap();
        {{ interface_name | camel }} {
            cid: ch.to_cid()
        }
    }

    // Stubs
{%-for call in calls %}
    pub fn {{ call["name"] }}(&self{{ call["args"] | rust_args(true) }}) -> ServerResult<({{ call["rets"] | rust_types }})> {
        let mut __r: Payload = 0;
        {%- for ret in call["rets"] %}
        let mut ret["name"]: Payload = 0;
        {% endfor %}

        unsafe {
            ipc_call(self.cid, {{ interface_name | upper }}_{{ call["name"] | upper }}_HEADER as Payload{{ call["args"] | rust_params }});
            Ok(({{ rets | rust_rets }}))
        }
    }
{% endfor %}
}

pub trait Server {
{%-for call in calls %}
    fn {{ call["name"] }}(&self, from: Channel{{ call["args"] | rust_args }}) -> ServerResult<({{ call["rets"] | rust_types(true) }})>;
{% endfor %}
}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        unsafe {
            match header.msg_type() {
{%-for call in calls %}
                {{ interface_name | upper }}_{{ call["name"] | upper }}_MSG => {
                    match self.{{ call["name"] }}(from{{ call["args"] | rust_server_params }}) {
                        Ok(({{ call["rets"] | rust_ids }})) => ({{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_MSG | OK_HEADER{{ call["rets"] | rust_server_rets }}),
                        Err(err) => (${reply_header_name} | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                    }
                },
{%- endfor %}
                _ => {
                    ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
                },
            }
        }
    }
}
"""

def generate_rust_file(types, interface):
    def rename_type(type_, with_ool_wrapper):
        return {
            "u8": 'u8',
            "u16": 'u16',
            "u32": 'u32',
            "u64": 'u64',
            "uptr": 'usize',
            "imax": "isize",
            "channel": 'Channel',
            "usize": "usize",
            "string": "OoL<&[u8]>" if with_ool_wrapper else "&[u8]",
            "buffer": "OoL<&[u8]>" if with_ool_wrapper else "&[u8]",
        }[type_]

    def header(args):
        value = 0
        for i, arg in enumerate(args):
            type_id = get_type_id_by_name(types, arg["type"])
            value |= type_id << (i * 2)
        return str(value)

    def rust_args(args, with_ool_wrapper=False):
        s = ""
        for arg in args:
            s += f", {arg['name']}: {rename_type(arg['type'], with_ool_wrapper)}"
        return s

    def rust_types(args, with_ool_wrapper=False):
        s = ""
        for arg in args:
            s += f", {rename_type(arg['type'], with_ool_wrapper)}"
        return s

    def rust_ids(args):
        ids = []
        for arg in args:
            ids.append(arg['name'])
        return ", ".join(ids)

    def rust_params(args):
        s = ""
        skip_next = False
        for arg in args:
            type_id = get_type_id_by_name(types, arg["type"])
            if skip_next:
                skip_next = False
                continue

            if type_id == OOL_PAYLOAD:
                # Skip an ool length; we use slice.len() instead.
                skip_next = True
                s += f", {arg['name']}.as_ptr() as Payload, {arg['name']}.len() as Payload"
            elif type_id == CHANNEL_PAYLOAD:
                s += f", {arg['name']}.to_cid() as Payload"
            else:
                s += f", {arg['name']} as Payload"

        # Append unused message payloads.
        for _ in range(len(args), 4):
            s += ", 0"

        return s

    def rust_server_params(args):
        ool_length = False
        s = ""
        for i, arg in enumerate(args):
            type_id = get_type_id_by_name(types, arg["type"])
            if ool_length:
                ool_length = False
            elif type_id == OOL_PAYLOAD:
                s += f", OoL::from_payload(a{i} as usize, a{i + 1} as usize)"
                ool_length = True
            elif type_id == CHANNEL_PAYLOAD:
                s += f", Channel::from_cid(a{i} as CId)"
            else:
                s += f", a{i} as {rename_type(arg['type'], True)}"
        return s

    def rust_rets(rets):
        rs = []
        for ret in rets:
            type_id = get_type_id_by_name(types, type_)
            if type_id == CHANNEL_PAYLOAD:
                rs.append(f"Channel::from_cid({ret['name']} as CId)")
            elif ool_payload is not None:
                rs.append(f"OoL::from_payload({ool_payload} as usize, {ret['name']} as usize)")
            elif type_id == OOL_PAYLOAD:
                ool_payload = name
            else:
                rs.append(f"{ret['name']} as {rename_type(ret['type'], False)}")
        return ", ".join(rs)

    def rust_server_rets(rets):
        s = ""
        ool_payload = None
        for ret in rets:
            type_id = get_type_id_by_name(types, ret["type"])
            if type_id == CHANNEL_PAYLOAD:
                s += f"{ret['name']}.to_cid() as Payload"
            elif ool_payload is not None:
                s += f"{ool_payload}.len() as Payload"
                ool_payload = None
            elif type_id == OOL_PAYLOAD:
                ool_payload = ret["name"]
                s += f"{ret['name']}.as_ptr() as Payload"
            else:
                s += f"{ret['name']} as Payload"
        return s

    env = jinja2.Environment()
    env.filters["header"] = header
    env.filters["rust_args"] = rust_args
    env.filters["rust_types"] = rust_types
    env.filters["rust_ids"] = rust_ids
    env.filters["rust_params"] = rust_params
    env.filters["rust_server_params"] = rust_server_params
    env.filters["rust_rets"] = rust_rets
    env.filters["rust_server_rets"] = rust_server_rets
    env.filters["camel"] = lambda s: s.replace("_", " ").title().replace(" ", "")

    return env.from_string(RUST_TEMPLATE).render(
        interface_name=interface["name"],
        interface_id=interface["attrs"]["id"],
        calls=interface["calls"]
    )

def generate_rust_stubs(out_dir, idl_files):
    mod_rs = ""
    for idl_file in idl_files:
        try:
            defs = idl.parse(idl_file)
        except Exception as e:
            sys.exit(f"failed to parse {idl_file}:" + str(e))

        types = defs["types"]
        for interface in defs["interfaces"]:
            stub = generate_rust_file(types, interface)
            mod_rs += f"pub mod {interface['name']};\n"
            write_file(os.path.join(out_dir, f"{interface['name']}.rs"), stub)

    write_file(os.path.join(out_dir, f"mod.rs"), mod_rs)

def main():
    argparser = argparse.ArgumentParser()
    argparser.add_argument("--lang", required=True)
    argparser.add_argument("--out-dir", required=True)
    argparser.add_argument("interfaces", nargs="+")
    args = argparser.parse_args()

    if args.lang == 'c':
        generate_c_stubs(args.out_dir, args.interfaces)
    elif args.lang == 'rust':
        generate_rust_stubs(args.out_dir, args.interfaces)

if __name__ == "__main__":
    main()
