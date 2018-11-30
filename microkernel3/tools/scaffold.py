#!/usr/bin/env python3
import argparse
import sys
import os
import idl


def generate_server(services):
    includes = ""
    mainloop = ""
    handlers = ""

    for service in services:
        service_name = service["name"]
        includes += f"#include <resea/{service_name}.h>"
        service_name = service["name"].replace("-", "_")
        for call in service["calls"]:
            call_name = call["name"]
            args = ""
            params = ""
            for i in range(0, 4):
                try:
                    name = call["args"][i]["name"]
                    type_ = call["args"][i]["type"]
                except IndexError:
                    pass
                else:
                    args += f", {type_}_t {name}"
                    params += f", ({type_}_t) a{i}"

            for i in range(0, 4):
                try:
                    name = call["rets"][i]["name"]
                    type_ = call["rets"][i]["type"]
                except IndexError:
                    pass
                else:
                    args += f", {type_}_t *{name}"
                    params += f", ({type_}_t *) &r{i}"

            msg_name = f"{service_name.upper()}_{call_name.upper()}_MSG"
            reply_header_name = f"{service_name.upper()}_{call_name.upper()}_REPLY_HEADER"

            handlers += f"""\
static inline error_t handle_{service_name}_{call_name}(channel_t from{args}) {{
    /* TODO */
    return ERROR_NONE;
}}


"""

            mainloop += f"""\
            case {msg_name}:
                error_t error = handle_{service_name}_{call_name}(from{params});
                header = {reply_header_name} | (error << ERROR_OFFSET);
                break;
"""

    return f"""\
#include <resea.h>
#include <resea/{service_name}.h>


{handlers.strip()}


void server_mainloop(void) {{
    channel_t from;
    payload_t a0, a1, a2, a3;
    payload_t r0, r1, r2, r3;
    header_t header = ipc_recv(server, &from, &a0, &a1, &a2, &a3);
    for (;;) {{
        switch (MSGTYPE(header)) {{
{mainloop}
            default:
                /* Unknown message. */
                break;
        }}

        ipc_replyrecv(server, &from, header, r0, r1, r2, r3, &a0, &a1, &a2, &a3);
    }}
}}


void main(void) {{
    server_mainloop();
}}
"""

def main():
    argparser = argparse.ArgumentParser()
    argparser.add_argument('-o', required=True)
    argparser.add_argument('idl_files', nargs='*')
    args = argparser.parse_args()

    services = []
    for service_name in args.idl_files:
        services.append(idl.parse(service_name))

    code = generate_server(services)
    with open(os.path.join(args.o, "main.c"), 'w') as f:
        f.write(code)

if __name__ == "__main__":
    main()
