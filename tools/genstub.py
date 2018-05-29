#!/usr/bin/env python3
import argparse
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
        "usize": INLINE_PAYLOAD,
        "umax": INLINE_PAYLOAD,
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
                header += f" | ({type_id} << {i * 3}ULL)"
                args += f", {type_}_t {name}"
                params += f", (payload_t) {name}"
                server_params += f", ({type_}_t) a{i}"

        for i in range(0, 4):
            try:
                name = call["rets"][i]["name"]
                type_ = call["rets"][i]["type"]
            except IndexError:
                params += ", &__unused"
            else:
                type_id = get_type_id_by_name(types, type_)
                reply_header += f" | ({type_id} << {i * 3}ULL)"
                args += f", {type_}_t *{name}"
                params += f", (payload_t *) {name}"
                server_params += f", ({type_}_t *) &r{i}"

        msg_name = f"{service_name.upper()}_{call_name.upper()}_MSG"
        reply_msg_name = f"{service_name.upper()}_{call_name.upper()}_REPLY_MSG"
        header_name = f"{service_name.upper()}_{call_name.upper()}_HEADER"
        reply_header_name = f"{service_name.upper()}_{call_name.upper()}_REPLY_HEADER"
        stub += f"""\
#define {msg_name}       ({service_name.upper()}_SERVICE | {msg_id}ULL)
#define {reply_msg_name} ({service_name.upper()}_SERVICE | {msg_id + 1}ULL)
#define {header_name} (({msg_name} << 32ULL) | ({header}))
#define {reply_header_name} (({reply_msg_name} << 32ULL) | ({reply_header}))
static inline header_t call_{service_name}_{call_name}(channel_t __server{args}) {{
    payload_t __unused;

    return ipc_call(
        __server, {header_name}{params}
    );
}}
"""
        msg_id += 1

    return f"""\
#ifndef __RESEA_STUB_{service_name}_H__
#define __RESEA_STUB_{service_name}_H__

#define {service_name.upper()}_SERVICE ({service["id"]}U << 8U)

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
    consts = ""
    types = {}
    service_id = service["id"]
    service_name_upper = service_name.upper()
    service_name_cap = service_name.capitalize()

    # Type aliases.
    for type_ in service["types"]:
        alias_of = type_["alias_of"] + "_t"
        stub += f"typedef {alias_of} {type_['new_name']};\n"
        types[type_["new_name"]] = type_["alias_of"]

    def rename_type(type_):
        return {
            "u32": 'u32',
            "uptr": 'usize',
            "channel": 'Channel',
            "usize": "usize",
            "string": "&[u8]"
        }[type_]

    # RPCs.
    stub += "\n"
    msg_id = 1
    for call in service["calls"]:
        call_name = call["name"]
        header = "0"
        reply_header = "0"
        args = ""
        params = ""
        tmps = []
        rets_def = []
        rets = []
        skip_next = False
        for i in range(0, 4):
            try:
                name = call["args"][i]["name"]
                type_ = call["args"][i]["type"]
            except IndexError:
                params += ", 0"
            else:
                type_id = get_type_id_by_name(types, type_)
                header += f" | ({type_id} << {i * 3}u64)"
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

                args += f", {name}: {rename_type(type_)}"

        for i in range(0, 4):
            try:
                name = call["rets"][i]["name"]
                type_ = call["rets"][i]["type"]
            except IndexError:
                params += ", &mut __r as *mut Payload"
            else:
                type_id = get_type_id_by_name(types, type_)
                reply_header += f" | ({type_id} << {i * 3}u64)"
                tmps.append(f"let mut {name}: Payload = 0;")
                params += f", &mut {name} as *mut Payload"
                rets_def.append(rename_type(type_))
                if type_id == CHANNEL_PAYLOAD:
                    rets.append(f"Channel::from_cid({name} as CId)")
                else:
                    rets.append(f"{name} as {rename_type(type_)}")

        rets_def = ", ".join(rets_def)
        rets = ", ".join(rets)
        tmps = "\n".join(tmps)
        msg_name = f"{service_name.upper()}_{call_name.upper()}_MSG"
        reply_msg_name = f"{service_name.upper()}_{call_name.upper()}_REPLY_MSG"
        header_name = f"{service_name.upper()}_{call_name.upper()}_HEADER"
        reply_header_name = f"{service_name.upper()}_{call_name.upper()}_REPLY_HEADER"
        consts += Template("""\
const ${msg_name}: u16 = ${service_name_upper}_SERVICE | ${msg_id}u16;
const ${reply_msg_name}: u16 = ${service_name_upper}_SERVICE | ${msg_id}u16 + 1;
const ${header_name}: u64 = ((${msg_name} as u64) << 32) | ${header};
const ${reply_header_name}: u64 = ((${reply_msg_name} as u64) << 32) | ${reply_header};
""").substitute(**locals())

        stub += Template("""\
    pub fn $call_name(&self${args}) -> Result<(${rets_def}), GeneralError> {
        let mut __r: Payload = 0;
        ${tmps}

        unsafe {
            ipc_call(self.cid, ${header_name} as Payload${params});
        }
        Ok((${rets}))
    }
""").substitute(**locals())

    return Template("""\
#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::result::{Result};
use error::{GeneralError};
use channel::{Channel};
use arch::{ipc_call, CId, Payload};

const ${service_name_upper}_SERVICE: u16 = $service_id << 8;
${consts}

pub struct $service_name_cap {
    cid: CId
}

impl $service_name_cap {
    pub fn from_cid(cid: CId) -> $service_name_cap {
        $service_name_cap {
            cid: cid
        }
    }

    $stub
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

    stub = generate_mod_rs(interfaces)
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
