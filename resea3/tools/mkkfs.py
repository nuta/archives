#!/usr/bin/env python3
import argparse
import struct
import sys
import os

def main():
    parser = argparse.ArgumentParser(description="Generates a Kernel File System image.")
    parser.add_argument("image")
    parser.add_argument("dir")
    args = parser.parse_args()

    with open(args.image, 'wb') as f:
        f.write(struct.pack("BB6s", 0x0f, 0x0b, bytes("KFS!!", encoding="utf-8")))

        for dirpath, _, filenames in os.walk(args.dir):
            for filename in filenames:
                filepath = os.path.relpath(os.path.join(dirpath, filename), args.dir)

                if len(filepath) >= 128:
                    sys.exit(f"kfs: too long file path: {filepath}")

                f.write(struct.pack("<128sI",
                            bytes("/" + filepath, encoding="utf-8"),
                            os.path.getsize(os.path.join(dirpath, filename))))
                f.write(open(os.path.join(dirpath, filename), 'rb').read())

        f.write(struct.pack("B", 0x00))

if __name__ == "__main__":
    main()
