---
title: Writing a Linux-compatible kernel in Rust
date: May 19, 2021
lang: en
---

***TL;DR:** I'm writing a Linux clone in Rust just for fun. It does NOT aim to replace the Linux kernel.*

For the recent months, I've been working on a new operating system kernel **[Kerla](https://github.com/nuta/kerla)**, written from scratch **in Rust** which aims to be Linux-compatible at the ABI level.
In other words, **support running unmodified Linux binaries**!

I've already implemented basic features: `fork(2)` and `execve(2)`, file operations, initramfs, TCP/UDP sockets, signals, tty / pty, pipe, poll, etc.

You can ssh into Kerla running on an ephemeral Firecracker microVM which is automatically launched just for you (it accepts arbitrary passwords):

```
$ ssh root@demo.kerla.dev
```

In this post, I'll share my first impression of using Rust in a OS kernel.

## Rust's success in the userland

Rust, a [beloved](https://insights.stackoverflow.com/survey/2020#technology-most-loved-dreaded-and-wanted-languages) programming language, enables you to develop reliable and performant software productively.

Rust's ownership concept and the type system let you focus on fixing logic errors,
not painful memory bugs and races. `enum` forces you to handle all possible input / ouput patterns. Moreover, its build system (`cargo`) and IDE support ([`rust-analyzer`](https://rust-analyzer.github.io/)) are just awesome.

Rust is gradually getting adopted on production: [lightweight VMM by AWS](https://aws.amazon.com/blogs/opensource/why-aws-loves-rust-and-how-wed-like-to-help/), [a part of the npm registry [PDF]](https://www.rust-lang.org/static/pdfs/Rust-npm-Whitepaper.pdf), and [many other companies](https://www.rust-lang.org/production/users). Rust is undoubtedly recognized as an excellent alternative (not a replacement!) to C and C++.

## Rust in the kernel-land

Rewriting existing software in Rust has been getting attention in recent years. 

Bryan Cantrill talks about that topic on operating systems in *"[Is It Time to Rewrite the Operating System in Rust?](https://www.infoq.com/presentations/os-rust/)"* In the presentation, a hybrid approach is proposed:

> So one hybrid approach is that you retain your existing C-/assembly-based kernel, the way we have had for many years. And then you allow for Rust-based things to be developed, Rust-based drivers, Rust-based file systems, Rust-based in-kernel software.

This is what people in [Rust for Linux](https://github.com/Rust-for-Linux/linux) are working on. You may have heard of [a patch](https://lkml.org/lkml/2021/4/14/1023) suggested on LKML recently.

I completely agree that rewriting existing large, feature-rich, and robust operating system kernels is not a good idea. However, a question came to my mind: what are the pros and cons of writing a *practical* UNIX-like OS kernel in Rust *from scratch*? How does it look like? This is why I started this project.

To explore Rust's strengths and weaknesses in the kernel-land, I decided to develop something practical, specifically, a Linux-compatible kernel that can be used as a [Function-as-a-Service](https://en.wikipedia.org/wiki/Function_as_a_service) runtime environment on virtual machines. For this purpose, you only need few device drivers (e.g. [virtio](https://wiki.osdev.org/Virtio)) and we won't have to support advanced Linux features like eBPF. Furthermore, kernel crashes won't be a critical problem compared to other long-running workloads. That doesn't sound impossible, does it?

## Is Rust good for the kernel-land?

In my opinion, yes *unless* the target environment is not too resource-constrained.

The kernel-land is a little bit eccentric: `panic!` will lead to a system crash, memory allocation failures should be handled without panicking, hidden control flows and hidden allocations are generally disliked.

Rust's advantages also apply to the kernel-land. My favorite example is null-able pointer handling:
if a pointer is nullable (i.e. `Option<NonNull<T>>`), you can't dereference it until you 
explicitly handle the null (`None`) case! Furthermore, using the [new type idiom](https://doc.rust-lang.org/rust-by-example/generics/new_types.html), you can distinguish between kernel and user pointers.

However, we still have some issues with using Rust in the kernel-land:

### Allocations failures are handled by `panic!` 

This issue is also mentioned by [Linus Torvalds on the Linux kernel's Rust support patch](https://lkml.org/lkml/2021/4/14/1099).

Let's take a look at the definition of [`Arc::new()`](https://doc.rust-lang.org/alloc/sync/struct.Arc.html#method.new), a constructor which creates a thread-safe reference-counting pointer:

> ```rust
> pub fn new(data: T) -> Arc<T>
> ```
> 
> Constructs a new `Arc<T>`.

Looks super intuitive, right? However, it has an implicit panic case: a failure of the internal buffer allocation.

Handling allocation failures is boring. The first step when I start a C project is to write my own [`xmalloc(3)`](https://www.freebsd.org/cgi/man.cgi?query=xmalloc&apropos=0) so that I don't need to check if the result is NULL. If it runs out of memory, I'd let it crash. It's not a big deal. All I need is to spawn a new VM with more memory or buy a new memory on Amazon.

However, `panic!`-ing in the kernel-space leads to literally a *kernel panic*. This is a big deal. We should manage to recover from the low memory situation to keep the system working. Nobody wants to see Blue Screen.

In my project, to take advantage of existing convenient crates, I decided to go with the current Rust's way: allow panicking by allocation failures.
That said, in the near future, I do think I need to use (or write our own) failable versions of dynamically allocated containers.

### Bigger binary size

[Resea](https://resea.org), a microkernel-based operating system written in C (authored by me) takes the only 845KiB (release build, stripped) including userland applications like the TCP/IP server, the Intel VT-x based hypervisor, and Linux ABI emulation support.

In contrast, A Kerla image takes about 1.1 MiB (release build, `opt-level = 'z'`, stripped) excluding initramfs. Although minimalistic microkernels and monolithic kernels have opposite philosophies, it seems to me that a Rust implementation tends to be bigger than a C implementation.

While [some size optimization ways](https://github.com/johnthagen/min-sized-rust) are available, in extremely constrained devices (e.g. [class 1 devices](https://tools.ietf.org/html/rfc7228)) this could be a problem.

### `make menuconfig` is missing

Operating system kernels have many parameters. In Linux kernel, they can be configured using config tools like `make menuconfig` and `make xconfig`. In Rust, we have [feature flags](https://doc.rust-lang.org/cargo/reference/features.html) to enable/disable features in a crate. However, what if you want to change a hardcoded parameter, like the interval of heartbeating? Do you configure it through an environment variable and [`env!`](https://doc.rust-lang.org/std/macro.env.html) macro? Nah, it accepts only a string. 

We might need a feature-rich build configuration mechanism just like [Kconfig](https://www.kernel.org/doc/html/latest/kbuild/kconfig-language.html) in Cargo. 

### These issues would be resolved sooner or later!

I'd stress that these problems are not originated from the language design.

Rust is getting improved continuously. Regarding the allocation failures, people have already started working on it
(see [the tracking issue](https://github.com/rust-lang/rust/issues/32838)).
Moreover, you don't have to use `liballoc`. [`heapless` crate](https://docs.rs/heapless/) will be a good alternative.

## Good points on Rust in the kernel-land

Despite the issues I mentioned above, I find it productive to write the kernel in Rust and believe. Let me talk about my favorite good things on kernel-land Rust:

- **Rust makes me confident:** its type system and ownership & lifetime concepts make me realize that my implementation won't work **at the compile time**, because of a violation of the *shared XOR mutable* rule, for example. Once the compile passes, it works without surprises (like nasty data races and dangling pointer dereferences).
- **Forces handling all input patterns:** `enum` and pattern matching (`match`) allow you to handle all possible cases in an expressive way. You don't need to be bothered by a messy chain of `else if`.
- **It already has things I need to write a kernel:** [packed struct](https://doc.rust-lang.org/nomicon/other-reprs.html#reprpacked), [raw pointers](https://doc.rust-lang.org/std/primitive.pointer.html), [improved inline assembly syntax](https://blog.rust-lang.org/inside-rust/2020/06/08/new-inline-asm.html), [embedding assembly files](https://doc.rust-lang.org/unstable-book/library-features/global-asm.html), ...
- **Convenient `no_std` (freestanding) crates are available:** [bitflags manipulation library](https://docs.rs/bitflags/1.2.1/bitflags/), [array-based vector and string implementation](https://docs.rs/arrayvec/0.7.0/arrayvec/), [multi-producer multi-consumer queue](https://docs.rs/crossbeam/0.8.0/crossbeam/queue/struct.ArrayQueue.html), ...
- **Built-in unit testing:** writing and running unit tests in Rust is pretty easy. What is more, you can run unit tests on QEMU or real machines thanks to the [custom_test_frameworks](https://rust-lang.github.io/rfcs/2318-custom-test-frameworks.html) feature.
- **Developer-friendly great toolchain:** [linter](https://github.com/rust-lang/rust-clippy) helps you write a good Rust code, [cross-compiling](https://rust-lang.github.io/rustup/cross-compilation.html) is pretty easy, and [rust-analyzer](https://rust-analyzer.github.io/) turns your favorite editor into a highly-productive IDE like IntelliJ IDEA.

## I need your help!

This kernel is still in the very early stage. Some key features like futex, epoll, UNIX domain socket, are still not yet implemented. To put it the other way around, the code is still simple and easy to understand! If you're interested in writing an operating system kernel in Rust, please join the development :)

Lastly, I'd quickly mention another promising alternative to C: [Zig programming language](https://ziglang.org/). You'll be impressed especially if you're a C programmer. I think it has great potential in the kernel-land too.
