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
#define {{ interface_name | upper }}_{{ call["name"] | upper }}_MSG          (({{ interface_name | upper }}_SERVICE << 8) | {{ call["id"] }}UL)
#define {{ interface_name | upper }}_{{ call["name"] | upper }}_HEADER       (({{ interface_name | upper }}_{{ call["name"] | upper }}_MSG << 16UL) | (({{ call["args"] | header }}) << 8))

{%- if call["oneway"] %}
static inline header_t send_{{ interface_name }}_{{ call["name"] }}(channel_t __server{{ call | c_args }}) {
    return ipc_send(__server, {{ interface_name | upper }}_{{ call["name"] | upper }}_HEADER{{ call | c_params }});
}

{% else %}
#define {{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_MSG    (({{ interface_name | upper }}_SERVICE << 8) | ({{ call["id"] }}UL + 1))
#define {{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_HEADER (({{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_MSG << 16UL) | (({{ call["rets"] | header }}) << 8))
static inline header_t call_{{ interface_name }}_{{ call["name"] }}(channel_t __server{{ call | c_args }}) {
    payload_t __tmp;

    return ipc_call(__server, {{ interface_name | upper }}_{{ call["name"] | upper }}_HEADER{{ call | c_params }});
}
{% endif %}
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
        if not call["oneway"]:
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

        if not call["oneway"]:
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
        except idl.ParseError as e:
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
use {Channel, Result as ServerResult, OoL, ErrorCode, CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
use interfaces::discovery;
use syscalls::{ipc_open, ipc_call, ipc_send, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = {{ interface_id }};
pub const {{interface_name | upper }}_SERVICE: u16 = {{ interface_id }};
{%- for call in calls %}
pub const {{ interface_name | upper }}_{{ call["name"] | upper }}_MSG: u16 = ({{ interface_name | upper }}_SERVICE  << 8) | {{ call["id"] }}u16;
pub const {{ interface_name | upper }}_{{ call["name"] | upper }}_HEADER: u64 = (({{ interface_name | upper }}_{{ call["name"] | upper }}_MSG as u64) << 16) | ({{ call["args"] | header }}u64 << 8);
{%- if not call["oneway"] %}
pub const {{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_MSG: u16 = ({{ interface_name | upper }}_SERVICE  << 8) | {{ call["id"] }}u16 + 1;
pub const {{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_HEADER: u64 = (({{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_MSG as u64) << 16) | ({{ call["rets"] | header }}u64 << 8);
{% endif %}
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
        let ch = discovery.discover({{ interface_name | upper }}_SERVICE).unwrap();
        {{ interface_name | camel }} {
            cid: ch.to_cid()
        }
    }

    // Stubs
{%-for call in calls %}
{%- if call["oneway"] %}
    pub fn {{ call["name"] }}(&self{{ call["args"] | rust_args }}) -> ServerResult<()> {
        unsafe {
            let __header = ipc_send(self.cid, {{ interface_name | upper }}_{{ call["name"] | upper }}_HEADER as Payload{{ call | rust_params }});
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }
{% else %}
    pub fn {{ call["name"] }}(&self{{ call["args"] | rust_args }}) -> ServerResult<({{ call["rets"] | rust_types(true) }})> {
        let mut __r: Payload = 0;
        {%- for ret in call["rets"] %}
        let mut {{ ret["name"] }}: Payload = 0;
        {% endfor %}

        unsafe {
            let __header: Header = ipc_call(self.cid, {{ interface_name | upper }}_{{ call["name"] | upper }}_HEADER as Payload{{ call | rust_params }});
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(({{ call["rets"] | rust_rets }}))
            } else {
                Err(__header.error_type())
            }
        }
    }
{% endif %}
{% endfor %}
}

pub trait Server {
{%-for call in calls %}
{%- if call["oneway"] %}
    fn {{ call["name"] }}(&self, from: Channel{{ call["args"] | rust_args(true) }});
{% else %}
    fn {{ call["name"] }}(&self, from: Channel{{ call["args"] | rust_args(true) }}) -> ServerResult<({{ call["rets"] | rust_types }})>;
{% endif %}
{% endfor %}
}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        match header.msg_type() {
{%-for call in calls %}
{%- if call["oneway"] %}
            {{ interface_name | upper }}_{{ call["name"] | upper }}_MSG => {
                self.{{ call["name"] }}(from{{ call["args"] | rust_server_params }});
                ((ErrorCode::DontReply as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
{%- else %}
            {{ interface_name | upper }}_{{ call["name"] | upper }}_MSG => {
                match self.{{ call["name"] }}(from{{ call["args"] | rust_server_params }}) {
                    Ok(({{ call["rets"] | rust_ids }})) => ({{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_HEADER | OK_HEADER{{ call["rets"] | rust_server_rets }}),
                    Err(err) => ({{ interface_name | upper }}_{{ call["name"] | upper }}_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
{%- endif %}
{%- endfor %}
            _ => {
                ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
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
            "string": "OoL" if with_ool_wrapper else "&[u8]",
            "buffer": "OoL" if with_ool_wrapper else "&[u8]",
        }[type_]

    def header(args):
        value = 0
        for i, arg in enumerate(args):
            type_id = get_type_id_by_name(types, arg["type"])
            value |= type_id << (i * 2)
        return str(value)

    def rust_args(args, with_ool_wrapper=False):
        s = ""
        skip_next = False
        for arg in args:
            if skip_next:
                skip_next = False
                continue

            if get_type_id_by_name(types, arg["type"]) == OOL_PAYLOAD:
                skip_next = True

            s += f", {arg['name']}: {rename_type(arg['type'], with_ool_wrapper)}"
        return s

    def rust_types(args, with_ool_wrapper=False):
        ts = []
        skip_next = False
        for arg in args:
            if skip_next:
                skip_next = False
                continue

            if get_type_id_by_name(types, arg["type"]) == OOL_PAYLOAD:
                skip_next = True

            ts.append(f"{rename_type(arg['type'], with_ool_wrapper)}")
        return ", ".join(ts)

    def rust_ids(args):
        ids = []
        skip_next = False
        for arg in args:
            if skip_next:
                skip_next = False
                continue

            if get_type_id_by_name(types, arg["type"]) == OOL_PAYLOAD:
                skip_next = True

            ids.append(arg['name'])
        return ", ".join(ids)

    def rust_params(call):
        s = ""
        skip_next = False
        for arg in call["args"]:
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
        for _ in range(len(call["args"]), 4):
            s += ", 0"

        if not call["oneway"]:
            for ret in call["rets"]:
                s += f", &mut {ret['name']}"

            # Append unused message payloads.
            for _ in range(len(call["rets"]), 4):
                s += ", &mut __r"

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
        ool_payload = None
        for ret in rets:
            type_id = get_type_id_by_name(types, ret["type"])
            if type_id == CHANNEL_PAYLOAD:
                rs.append(f"Channel::from_cid({ret['name']} as CId)")
            elif ool_payload is not None:
                rs.append(f"OoL::from_payload({ool_payload} as usize, {ret['name']} as usize)")
            elif type_id == OOL_PAYLOAD:
                ool_payload = ret["name"]
            else:
                rs.append(f"{ret['name']} as {rename_type(ret['type'], False)}")

        return ", ".join(rs)

    def rust_server_rets(rets):
        s = ""
        ool_payload = None
        for ret in rets:
            type_id = get_type_id_by_name(types, ret["type"])
            if type_id == CHANNEL_PAYLOAD:
                s += f", {ret['name']}.to_cid() as Payload"
            elif ool_payload is not None:
                s += f", {ool_payload}.len() as Payload"
                ool_payload = None
            elif type_id == OOL_PAYLOAD:
                ool_payload = ret["name"]
                s += f", {ret['name']}.as_ptr() as Payload"
            else:
                s += f", {ret['name']} as Payload"

        for _ in range(len(rets), 4):
            s += ", 0"

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
        except idl.ParseError as e:
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
