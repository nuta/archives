# Architecture

## Microkernel-based Design

FTL is a multi-server microkernel-based operating system. This means that the kernel provides minimal primitives.

Each application is a process isolated from others, and communicates with each other using IPC (Inter-Process Communication). Having this explicit boundary between components makes the system more secure and robust as shown in existing microkernels.

## OS Services are Applications

Components such as TCP/IP stack, file system, and device drivers, are implemented as applications. They collaborate with each other via message passing ([channels](../learn/channel)).

## Running Applications in Kernel

FTL provides [multiple isolation mechanisms](../learn/isolation), including running applications in kernel mode relying on Rust's safety. That is, FTL can be used as a (partially) unikernel or monolithic kernel for performance, but with adequately safe guarantees.

## Directory Structure

Here is the directory structure of [FTL Git repository](https://github.com/nuta/ftl) you would want to know:

- `apps/<category>/<name>`: Applications including OS services and device drivers.
- `libs/rust/*`: Libraries for Rust.
  - `libs/rust/ftl`: The FTL standard library for Rust.
  - `libs/rust/ftl_driver`: Device driver APIs.
- `kernel`: The kernel code.
- `test`: Integration tests in TypeScript ([bun tests](https://bun.com/docs/test/writing-tests)).

> [!NOTE]
> As you can see, FTL consists of only 3 components: applications, libraries, and the kernel.
