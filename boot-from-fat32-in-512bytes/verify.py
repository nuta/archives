#!/usr/bin/env python3
import argparse
import math
import sys
import struct

def unpack(offset, size, buf):
    if size == 1:
        fmt = 'B'
    elif size == 2:
        fmt = 'H'
    elif size == 4:
        fmt = 'I'
    elif size == 8:
        fmt = 'Q'

    return struct.unpack(fmt, buf[offset:offset + size])[0]

bad = False
def print_result(desc, additional, result):
    if result:
        result_msg = 'OK'
    else:
        result_msg = 'BAD'
        bad = True

    print(f"[{result_msg}] {desc}: {additional}")

def verify_disk_image(filepath):
    global sectors_per_cluster
    image = open(filepath, "rb").read()
    sector_size = unpack(0x0b, 2, image)
    sectors_per_cluster = unpack(0x0d, 1, image)
    total_sectors = unpack(0x20, 4, image)
    total_clusters = total_sectors // sectors_per_cluster
    start_of_cluters_high = unpack(0x14, 2, image)

    print_result(
        "The BPB must starts with [0xeb, 0x58, 0x90]",
        f"{image[0:3]}",
        image[0:3] == bytes([0xeb, 0x58, 0x90])
    )

    print_result(
        "The file system must be FAT-32",
        f"{total_clusters} clusters",
        total_clusters > 0xfff7
    )

    print_result(
        "The sector size must be 512",
        f"{sector_size}",
        sector_size == 512
    )

    res = math.log2(sectors_per_cluster)
    print_result(
        "# of sectors per cluster must be a power of 2",
        f"log2({sectors_per_cluster}) = {res}",
        res.is_integer()
    )


def verify_kernel_image(filepath):
    image = open(filepath, "rb").read()

    print_result(
        "The kernel image must be an ELF file.",
        f"{image[0:4]}",
        image[0:4] == bytes("\x7fELF", encoding="utf-8")
    )

    machine = unpack(0x12, 2, image)
    print_result(
        "The kernel image must be x86_64.",
        f"{hex(machine)}",
        machine == 0x3e
    )

    offset = 64
    segments_num = unpack(0x38, 2, image)
    for i in range(0, segments_num):
        type_ = unpack(offset, 4, image)
        if type_ == 1:
            # PT_LOAD
            file_offset = unpack(offset + 0x08, 8, image)
            vaddr = unpack(offset + 0x10, 8, image)

            align = sectors_per_cluster * 512
            res = file_offset % align
            print_result(
                "The segment file offset must be aligned to cluster * sector_size",
                f"{offset} % {align} == {res}",
                res == 0
            )

            print_result(
                "The segment virtual address must be less than 0xa0000",
                f"{hex(vaddr)}",
                vaddr < 0xa0000
            )

        offset += 0x38


def main():
    parser = argparse.ArgumentParser(description="Disk image and kernel image verifier.")
    parser.add_argument("partition")
    parser.add_argument("kernel")
    args = parser.parse_args()

    verify_disk_image(args.partition)
    verify_kernel_image(args.kernel)

    if bad:
        sys.exit("Some tests are failed.")

if __name__ == "__main__":
    main()