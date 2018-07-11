#!/usr/bin/env python3
import argparse
from glob import glob
import os
from string import Template
import sys
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

def generate_c_file(service):
    service_name = service["name"].replace("-", "_")
    server_includes = f"#include <resea/{service_name}.h>"
    stub = ""
    types = {}

    # Type aliases.
    for type_ in service["types"]:
        alias_of = type_["alias_of"] + "_t"
        stub += f"typedef {alias_of} {type_['new_name']};\n"
        types[type_["new_name"]] = type_["alias_of"]

    # RPCs.
    stub += "\n"
    msg_id = 1
    for call in service["calls"]:
        call_name = call["name"]
        header = "0"
        reply_header = "0"
        args = ""
        params = ""
        server_params = ""
        for i in range(0, 4):
            try:
                name = call["args"][i]["name"]
                type_ = call["args"][i]["type"]
            except IndexError:
                params += ", 0"
            else:
                type_id = get_type_id_by_name(types, type_)
                header += f" | ({type_id} << {i * 2}UL)"
                args += f", {type_}_t {name}"
                params += f", (payload_t) {name}"
                server_params += f", ({type_}_t) a{i}"

        for i in range(0, 4):
            try:
                name = call["rets"][i]["name"]
                type_ = call["rets"][i]["type"]
            except IndexError:
                params += ", &__tmp"
            else:
                type_id = get_type_id_by_name(types, type_)
                reply_header += f" | ({type_id} << {i * 2}UL)"
                args += f", {type_}_t *{name}"
                params += f", (payload_t *) {name}"
                server_params += f", ({type_}_t *) &r{i}"

        msg_name = f"{service_name.upper()}_{call_name.upper()}_MSG"
        reply_msg_name = f"{service_name.upper()}_{call_name.upper()}_REPLY_MSG"
        header_name = f"{service_name.upper()}_{call_name.upper()}_HEADER"
        reply_header_name = f"{service_name.upper()}_{call_name.upper()}_REPLY_HEADER"
        stub += f"""\
#define {msg_name}       (({service_name.upper()}_SERVICE << 8) | {msg_id}UL)
#define {reply_msg_name} (({service_name.upper()}_SERVICE << 8) | {msg_id + 1}UL)
#define {header_name} (({msg_name} << 16UL) | (({header}) << 8))
#define {reply_header_name} (({reply_msg_name} << 16UL) | (({reply_header}) << 8))
static inline header_t call_{service_name}_{call_name}(channel_t __server{args}) {{
    payload_t __tmp;

    return ipc_call(
        __server, {header_name}{params}
    );
}}
"""
        msg_id += 1

    return f"""\
#ifndef __RESEA_STUB_{service_name}_H__
#define __RESEA_STUB_{service_name}_H__

#define {service_name.upper()}_SERVICE ({service["id"]}U)

{stub}

#endif

"""


def generate_c_stubs(idl_dir, out_dir, interfaces):
    for interface in interfaces:
        service = idl.parse(os.path.join(idl_dir, f"{interface}.idl"))
        stub = generate_c_file(service)

        write_file(os.path.join(out_dir, f"resea/{interface}.h"), stub)


def generate_rust_file(service):
    service_name = service["name"].replace("-", "_")
    stub = ""
    server = ""
    handle = ""
    consts = ""
    types = {}
    service_id = service["id"]
    service_name_upper = service_name.upper()
    service_name_camel = service["name"].replace("_", " ").title().replace(" ", "")

    # Type aliases.
    for type_ in service["types"]:
        alias_of = type_["alias_of"] + "_t"
        stub += f"typedef {alias_of} {type_['new_name']};\n"
        types[type_["new_name"]] = type_["alias_of"]

    def rename_type(type_, with_ool_wrapper):
        return {
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

    # RPCs.
    stub += "\n"
    server += "\n"
    msg_id = 1
    for call in service["calls"]:
        call_name = call["name"]
        header = "0"
        reply_header = "0"
        args = ""
        server_args = ""
        params = ""
        tmps = []
        rets_def = []
        server_rets_def = []
        rets = []
        server_params = []
        server_rets = []
        casted_server_rets = []
        skip_next = False
        for i in range(0, 4):
            try:
                name = call["args"][i]["name"]
                type_ = call["args"][i]["type"]
            except IndexError:
                params += ", 0"
            else:
                type_id = get_type_id_by_name(types, type_)
                header += f" | ({type_id} << {i * 2}u64)"
                if skip_next:
                    skip_next = False
                    continue

                if type_id == OOL_PAYLOAD:
                    # Skip an ool length; we use slice.len() instead.
                    skip_next = True
                    params += f", {name}.as_ptr() as Payload, {name}.len() as Payload"
                elif type_id == CHANNEL_PAYLOAD:
                    params += f", {name}.to_cid() as Payload"
                else:
                    params += f", {name} as Payload"

                args += f", {name}: {rename_type(type_, False)}"
                server_args += f", {name}: {rename_type(type_, True)}"

        ool_length = False
        for i in range(0, len(call["args"])):
            type_ = call["args"][i]["type"]
            type_id = get_type_id_by_name(types, type_)
            if ool_length:
                ool_length = False
            elif type_id == OOL_PAYLOAD:
                server_params.append(f"OoL::from_payload(a{i} as usize, a{i + 1} as usize)")
                ool_length = True
            elif type_id == CHANNEL_PAYLOAD:
                server_params.append(f"Channel::from_cid(a{i} as CId)")
            else:
                server_params.append(f"a{i} as {rename_type(type_, True)}")

        ool_payload = None
        for i in range(0, 4):
            try:
                name = call["rets"][i]["name"]
                type_ = call["rets"][i]["type"]
            except IndexError:
                params += ", &mut __r as *mut Payload"
                casted_server_rets.append("0")
            else:
                type_id = get_type_id_by_name(types, type_)
                reply_header += f" | ({type_id} << {i * 2}u64)"
                tmps.append(f"let mut {name}: Payload = 0;")
                params += f", &mut {name} as *mut Payload"
                if type_id == CHANNEL_PAYLOAD:
                    rets.append(f"Channel::from_cid({name} as CId)")
                    rets_def.append(rename_type(type_, True))
                    server_rets_def.append(rename_type(type_, False))
                    server_rets.append(name)
                    casted_server_rets.append(f"{name}.to_cid() as Payload")
                elif ool_payload is not None:
                    rets.append(f"OoL::from_payload({ool_payload} as usize, {name} as usize)")
                    casted_server_rets.append(f"{ool_payload}.len() as Payload")
                    rets_def.append("OoL<&[u8]>")
                    server_rets_def.append("&[u8]")
                    ool_payload = None
                elif type_id == OOL_PAYLOAD:
                    ool_payload = name
                    server_rets.append(name)
                    casted_server_rets.append(f"{name}.as_ptr() as Payload")
                else:
                    rets.append(f"{name} as {rename_type(type_, False)}")
                    server_rets.append(name)
                    casted_server_rets.append(f"{name} as Payload")
                    rets_def.append(rename_type(type_, True))
                    server_rets_def.append(rename_type(type_, False))

        rets_def = ", ".join(rets_def)
        server_rets_def = ", ".join(server_rets_def)
        rets = ", ".join(rets)
        tmps = "\n".join(tmps)
        server_params = ", ".join(server_params)
        server_rets = ", ".join(server_rets)
        casted_server_rets = ", ".join(casted_server_rets)
        msg_name = f"{service_name.upper()}_{call_name.upper()}_MSG"
        reply_msg_name = f"{service_name.upper()}_{call_name.upper()}_REPLY_MSG"
        header_name = f"{service_name.upper()}_{call_name.upper()}_HEADER"
        reply_header_name = f"{service_name.upper()}_{call_name.upper()}_REPLY_HEADER"
        consts += Template("""\
pub const ${msg_name}: u16 = (${service_name_upper}_SERVICE  << 8) | ${msg_id}u16;
pub const ${reply_msg_name}: u16 = (${service_name_upper}_SERVICE  << 8) | ${msg_id}u16 + 1;
pub const ${header_name}: u64 = ((${msg_name} as u64) << 16) | ((${header}) << 8);
pub const ${reply_header_name}: u64 = ((${reply_msg_name} as u64) << 16) | ((${reply_header}) << 8);
""").substitute(**locals())

        stub += Template("""\
    pub fn $call_name(&self${args}) -> ServerResult<(${rets_def})> {
        let mut __r: Payload = 0;
        ${tmps}

        unsafe {
            ipc_call(self.cid, ${header_name} as Payload${params});
            Ok((${rets}))
        }
    }
""").substitute(**locals())

        server += Template("""\
    fn $call_name(&self, from: Channel${server_args}) -> ServerResult<(${server_rets_def})>;
""").substitute(**locals())

        handle += Template("""\
            ${msg_name} => {
                match self.${call_name}(from, ${server_params}) {
                    Ok((${server_rets})) => (${reply_header_name} | OK_HEADER, ${casted_server_rets}),
                    Err(err) => (${reply_header_name} | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            }
""").substitute(**locals())

        msg_id += 1

    return Template("""\
#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use server::{ServerResult};
use channel::{Channel};
use interfaces::discovery;
use arch::{
    CId, Payload,
    OoL,
    Header, HeaderTrait,
    ErrorCode, ERROR_OFFSET,
    ipc_open, ipc_call, ipc_recv, ipc_replyrecv,
};

pub const SERVICE_ID: u16 = $service_id;
pub const ${service_name_upper}_SERVICE: u16 = $service_id;
${consts}

pub struct $service_name_camel {
    cid: CId
}

impl $service_name_camel {
    pub fn from_cid(cid: CId) -> $service_name_camel {
        $service_name_camel {
            cid: cid
        }
    }

    pub fn connect() -> $service_name_camel {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover(${service_name_upper}_SERVICE).unwrap();
        $service_name_camel {
            cid: ch.to_cid()
        }
    }

    $stub
}

pub trait Server {
    $server
}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        unsafe {
            match header.msg_type() {
                $handle
                _ => {
                    ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
                },
            }
        }
    }
}
""").substitute(**locals())

def generate_mod_rs(interfaces):
    code = ""
    for interface in interfaces:
        code += f"pub mod {interface};\n"
    code += "\n"

    return code

def generate_rust_stubs(idl_dir, out_dir, interfaces):
    for interface in interfaces:
        service = idl.parse(os.path.join(idl_dir, f"{interface}.idl"))
        stub = generate_rust_file(service)
        write_file(os.path.join(out_dir, f"{interface}.rs"), stub)

    existing_interfaces = list(filter(lambda m: m != "mod", map(lambda f:
        os.path.splitext(os.path.basename(f))[0],
        glob(os.path.join(out_dir, "*.rs")))))

    stub = generate_mod_rs(existing_interfaces)
    write_file(os.path.join(out_dir, f"mod.rs"), stub)

def main():
    argparser = argparse.ArgumentParser()
    argparser.add_argument("--lang", required=True)
    argparser.add_argument("--out-dir", required=True)
    argparser.add_argument("--idl-dir", required=True)
    argparser.add_argument("interfaces", nargs="+")
    args = argparser.parse_args()

    if args.lang == 'c':
        generate_c_stubs(args.idl_dir, args.out_dir, args.interfaces)
    elif args.lang == 'rust':
        generate_rust_stubs(args.idl_dir, args.out_dir, args.interfaces)

if __name__ == "__main__":
    main()
