#!/usr/bin/env python3
import argparse
import os
from string import Template

TEMPLATES = {
    "x64": """\
{
    "llvm-target": "x86_64-unknown-none-gnu",
    "data-layout": "e-m:e-i64:64-f80:128-n8:16:32:64-S128",
    "linker-flavor": "ld",
    "linker": "$lld_path",
    "pre-link-args": {
        "ld": [
            "--script=$repo_dir/servers/libs/resea/app.ld"
        ]
    },
    "target-endian": "little",
    "target-pointer-width": "64",
    "target-c-int-width": "32",
    "arch": "x86_64",
    "os": "none",
    "features": "-mmx,-sse,+soft-float",
    "disable-redzone": true,
    "panic": "abort",
    "executables": true,
    "relocation_model": "static"
}
"""
}

def generate(arch):
    return Template(TEMPLATES[arch]).substitute(
        lld_path="/usr/local/opt/llvm/bin/ld.lld",
        repo_dir=os.path.abspath(os.path.join(os.path.dirname(__file__), "../../.."))
    )

def main():
    parser = argparse.ArgumentParser(description="Rust target json generator.")
    parser.add_argument("arch")
    parser.add_argument("file")
    args = parser.parse_args()

    target_json = generate(args.arch)
    if os.path.exists(args.file) and target_json == open(args.file).read():
        # We don't have to save.
        return

    with open(args.file, "w") as f:
        f.write(target_json)


if __name__ == "__main__":
    main()
