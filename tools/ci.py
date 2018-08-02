#!/usr/bin/env python3
import os
import subprocess

antlr = "java -jar {}/antlr-4.7.1-complete.jar".format(os.getcwd())
qemu = "{}/qemu/x86_64-softmmu/qemu-system-x86_64".format(os.getcwd())

def test_kernel(arch):
    if arch == "posix":
        target = "coverage"
    else:
        target = "test"

    subprocess.run(
        [
            "make", "-j2", target,
            "ARCH=" + arch,
            "ANTLR4=" + antlr,
            "SERVERS=test",
            "CC=clang-6.0",
            "LD=ld.lld-6.0",
            "QEMU=" + qemu
        ],
        check=True
    )

def test_lib(lib):
    subprocess.run(["cargo", "test"], cwd=os.path.join("libs", lib), check=True)

def main():
    subprocess.run(["make", "stubs", "ANTLR4=" + antlr], check=True)

    if os.environ["M"] == "kernel":
        test_kernel(os.environ["ARCH"])
    elif os.environ["M"] == "lib":
        test_lib(os.environ["LIB"])

if __name__ == "__main__":
    main()
