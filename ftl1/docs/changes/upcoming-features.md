# Upcoming Features

Here are the exciting features that we plan to ship in the future:

| Feature | Timeline | Priority | Status |
|---------|----------|----------|--------|
| [Filesystem](#filesystem) | 2026 Q1 | High | In Progress |
| [Linux compatibility](#linux-compatibility) | 2026 Q1 | High | [PoC done](https://seiya.me/blog/hypervisor-as-a-library) |
| [More programming languages](#more-programming-languages) | TBD | Medium | Planned |
| [Kubernetes-like dynamic controller](#kubernetes-like-dynamic-controller) | TBD | Low | Planned |
| [User-mode isolation](#user-mode-isolation) | TBD | Low | PoC done |
| [Multi-threaded API](#multi-threaded-api) | TBD | Low | Planned |

## Filesystem

Design the filesystem API. This blocks Linux compatibility because we want to support Linux container images that are typically too large to fit into the memory.

## Linux Compatibility

Run your Linux containers on FTL. Learn more in [Linux Compatibility](../learn/linux-compatibility).

## More Programming Languages

Currently, FTL supports Rust only for applications. We plan to support more programming languages in the future. Candidates are:

- TypeScript / JavaScript
- Python
- [Swift](https://www.swift.org/get-started/embedded/)
- ... or build our own programming language

## Kubernetes-like Dynamic Controller

Currently, FTL kernel starts statically defined applications. This is enough for the current use cases, but it means you can't spawn new applications at runtime without rebuilding the OS.

Our plan is to add a new controller application that manages the applications dynamically based on your declarative spec files (maybe in YAML). This means you can manage the entire OS state declaratively - the controller spawns and manages the applications to match the desired state like Kubernetes does.

## User-mode Isolation

Run your apps in the user mode. This is the traditional isolation mechanism to run memory-unsafe code safely.

Switching unikernel/usermode isolation will be seamless thanks to carefully designed Rust APIs.

## Multi-threaded API

Currently, FTL applications are single-threaded. This is fast enough and is very ergonomic because we can leverage Rust's borrow checker at compile time. However, it's not fast enough if you want to squeeze every bit of performance from modern CPUs.

We plan to add support for multi-threaded API (maybe in the thread-per-core design) and SMP (Symmetric Multi-Processing).
