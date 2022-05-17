#!/usr/bin/env python3
import requests
import jinja2
import subprocess
import ipaddress
import smtplib
import re
import struct
import sys
from pathlib import Path
import ipaddress

PRIVATE_NETWORK = ipaddress.ip_network("10.123.0.0/24")

GRUB_40_CUSTOM = jinja2.Template("""\
#!/bin/sh
exec tail -n +3 $0
# This file provides an easy way to add custom menu entries.  Simply type the
# menu entries you want to add after this comment.  Be careful not to change
# the 'exec tail' line above.

default saved

menuentry "Kerla" {
    multiboot2 /boot/kerla.elf dhcp=off ip4={{ ip4 }} gateway_ip4={{ gateway_ip4 }} pci_device=0:4
    savedefault 0
    boot
}

""")

ansi_escape = re.compile(r'\x1b(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')

def main():
    # Run this script on every boot.
    cloud_cfg = open("/etc/cloud/cloud.cfg", "r").read()
    cloud_cfg = cloud_cfg.replace("- scripts-user", "- [scripts-user, always]")
    open("/etc/cloud/cloud.cfg", "w").write(cloud_cfg)

    # Report the kernel crash.
    if Path("/kerla.dump").exists():
        with Path("/kerla.dump").open("rb") as f:
            magic = struct.unpack("<I", f.read(4))[0]
            if magic == 0xdeadbeee:
                len = struct.unpack("<I", f.read(4))[0]
                crash_log = f.read(len).decode("utf-8", errors="backslashreplace")
                crash_log = ansi_escape.sub("", crash_log)
                crash_log = crash_log.replace("\n", "\r\n")
                with smtplib.SMTP("localhost") as smtp:
                    m = "Subject: Kerla has crashed\n\n"
                    m += crash_log
                    smtp.sendmail("kerla-crash-report@seiya.me", "email@seiya.me", m)

    subprocess.run(["dd", "if=/dev/zero", "of=/kerla.dump", "bs=1024", "count=64"])

    # Look for the bastion.
    bastion_ip = None
    for ip in PRIVATE_NETWORK:
        try:
            if requests.get(f"http://{ip}/hostname").text.strip() == "bastion":
                bastion_ip = ip
                break
        except:
            pass

    if not bastion_ip:
        sys.exit("failed to locate bastion in the private network")

    image_url = f"http://{bastion_ip}/uploads/kerla.elf"
    print(f"image URL: {image_url}")
    resp = requests.get(image_url, allow_redirects=True)
    if resp.status_code != 200:
        sys.exit(f"failed to download an image from {image_url}")
    with open("/boot/kerla.elf", "wb") as f:
        f.write(resp.content)

    ip4_addr = requests.get("http://169.254.169.254/metadata/v1/interfaces/private/0/ipv4/address").text.strip()
    gateway_ip4 = requests.get("http://169.254.169.254/metadata/v1/interfaces/private/0/ipv4/gateway").text.strip()
    netmask = requests.get("http://169.254.169.254/metadata/v1/interfaces/private/0/ipv4/netmask").text.strip()
    netmask_len = ipaddress.IPv4Network(f"0.0.0.0/{netmask}").prefixlen

    with open("/etc/grub.d/40_custom", "w") as f:
        f.write(GRUB_40_CUSTOM.render(ip4=f"{ip4_addr}/{netmask_len}", gateway_ip4=gateway_ip4))

    # Boot Kerla in the only next boot.
    subprocess.run(["update-grub"])
    subprocess.run(["grub-reboot", "Kerla"])

    subprocess.run(["reboot"])

if __name__ == "__main__":
    main()
