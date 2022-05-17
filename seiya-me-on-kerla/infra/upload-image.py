#!/usr/bin/env python3
import argparse
import digitalocean
import subprocess
import os
import sys

do_token = os.environ['DO_TOKEN']
manager = digitalocean.Manager(token=do_token)

def find_vm_by_name(name):
    for vm in manager.get_all_droplets():
        if vm.name == name:
            return vm
    sys.exit(f"failed to find {name}")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("image")
    parser.add_argument("-i", help="The SSH key.", required=True)
    args = parser.parse_args()

    vm = find_vm_by_name("bastion")
    stdin = f"put {args.image} /uploads/kerla.elf".encode("utf-8")
    subprocess.run(["sftp", "-oStrictHostKeyChecking=no", "-i", args.i, "uploader@" + vm.ip_address], input=stdin, check=True)

if __name__ == "__main__":
    main()
