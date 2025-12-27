# Kernel Development

Kernel development is much more strict than application development. This guide presents some implicit rules that you should follow when writing kernel code.

> [!TIP]
> Want to learn kernel development? Check [Operating System in 1,000 Lines](https://operating-system-in-1000-lines.vercel.app/) first!

## Execution Flow

Once the kernel is booted, it will behave like an event handler: it waits for events (e.g. system calls, interrupts, and exceptions), saves the current thread's state, does the necessary job, and resumes a thread.

## Single Kernel Stack

Unlike traditional operating systems, the FTL kernel uses a single stack per CPU, instead of having a dedicated stack for each thread (so-called *"single kernel stack"* design).

This design resembles how async Rust works - we need a separate state machine to resume the thread's execution later. In kernel, we don't use `async`/`await` syntax, but we use `Promise` to represent the state of a blocked thread.

## APIs

In kernel, we use some `std` alternatives that are more suitable for kernel:

| `libstd` equivalent | Kernel API | Remarks |
|----------------|--------------------|----|
| `Arc` | `crate::refcount::SharedRef` | Weak references are not supported. |
| `Mutex` | `crate::spinlock::SpinLock` | This is a simple busy wait. |
| `thread_local` | `CpuVar` | A CPU-local variable. |

## Isolation

To support multiple isolation mechanisms like Unikernel-style, user mode, and WebAssembly in the future, the kernel abstracts the user pointer access with `Isolation` trait.

## Rules

- Avoid `panic`s. If you use `unwrap`, describe why you think it never fails.
- Handle allocation failures in collections (e.g. `Vec`). Use `try_reserve` before adding a new element to a collection.
