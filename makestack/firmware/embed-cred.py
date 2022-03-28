#!/usr/bin/env python3
import argparse
import struct
import sys
import os

"""
struct cred {
    uint64_t version;
    char adapter[8];
    char wifi_ssid[64];
    char wifi_password[64];
    char server_url[256];
};
"""

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("file")
    parser.add_argument("--endian", default="little")
    parser.add_argument("--adapter")
    parser.add_argument("--version")
    parser.add_argument("--wifi-ssid")
    parser.add_argument("--server-url")
    args = parser.parse_args()

    # Don't pass the password from the command line argument!
    wifi_password = os.environ.get("WIFI_PASSWORD")
    if len(args.wifi_ssid) > 0 and wifi_password is None:
        sys.exit("WIFI_PASSWORD environment variable is not set")

    if args.adapter not in ["serial", "wifi"]:
        sys.exit("invalid adapter name")

    # Subtract the max len by 1 to guarantee that strings are null-terminated.
    if len(args.adapter) > 8 - 1:
        sys.exit("too long adapter name")
    if len(args.wifi_ssid) > 64 - 1:
        sys.exit("too long wifi ssid")
    if len(wifi_password) > 64 - 1:
        sys.exit("too long wifi password")
    if len(args.server_url) > 256 - 1:
        sys.exit("too long server URL")

    endian = "<" if args.endian == "little" else ">"
    cred = struct.pack(endian + "Q8s64s64s256s",
        int(args.version),
        bytes(args.adapter, "utf-8"),
        bytes(args.wifi_ssid, "utf-8"),
        bytes(wifi_password, "utf-8"),
        bytes(args.server_url.rstrip("/"), "utf-8"))

    with open(args.file, "rb") as f:
        image = f.read()

    START_HEADER = bytes("__MAKESTACK_CRED_START__", "ascii")
    END_HEADER = bytes("__MAKESTACK_CRED_END__", "ascii")
    header_start = image.find(START_HEADER)
    end = image.find(END_HEADER)
    if header_start < 0 or end < 0:
        sys.exit("failed to locate the cred area")

    start = header_start + len(START_HEADER)
    area_len = end - start
    if len(cred) > area_len:
        sys.exit("the cred area is too short")

    image = image[:start] + cred + image[start + len(cred):]

    with open(args.file, "wb") as f:
        f.write(image)

if __name__ == "__main__":
    main()
