# Quick Start

This guide will walk you through the process of building and running FTL operating system. Don't worry, it's super quick and easy.

## Installation

### macOS

1. Visit [Rustup](https://rustup.rs/) and install Rustup, the Rust toolchain manager.

2. Install other dependencies from Homebrew:

```bash
brew install qemu
```

### Ubuntu

1. Visit [Rustup](https://rustup.rs/) and install Rustup, the Rust toolchain manager.
2. Install other dependencies from apt:

```bash
sudo apt install -y qemu-system
```

## Clone the Git Repository

FTL is managed as a monorepo and everything is in there:

```bash
git clone https://github.com/nuta/ftl
cd ftl
```

## Running FTL

`run.sh` builds and runs FTL in QEMU:

```bash
bin/ftl run
```

Once it finishes building, it will start QEMU and you should see the following output:

```
$ bin/ftl run
...
==> Starting QEMU 64-bit Arm (HVF hardware acceleration)
[kernel      ] INFO   Booting FTL...
[kernel      ] DEBUG  reserved area: 0000000040200000 - 00000000404c8000 (2 MiB)
[kernel      ] DEBUG  reserved area: 0000000040000000 - 0000000040100000 (1 MiB)
[kernel      ] INFO   free RAM area: 0000000040100000 (1 MiB)
```

Congratulations! 🎉 You've successfully got FTL running.

> [!NOTE]
> Now you can [start writing your first application](/guides/writing-applications).
