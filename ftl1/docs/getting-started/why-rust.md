# Why Rust?

We (and perhaps you too) love to debate the best text editor and programming language, sometimes very seriously and passionately. Let's explore why we chose Rust.

## FTL will support more languages (like TypeScript)

First, we want to make it clear that Rust will not be the only programming language supported by FTL. It is crystal clear that Rust (or any other language) is not the best language for everything.

That's why **FTL is designed to be language-agnostic to support more languages such as TypeScript** (see [GC-based Isolation](../learn/isolation#gc-based-isolation-javascript-in-kernel)).

## Running Applications in Kernel Space Safely

FTL aims to be a high-performance OS. To achieve this in the microkernel design, we need to run OS components in the kernel space while isolating them reasonably safely, with low overhead. Specifically, we need to ensure:

- **Memory safety:** Applications can't dereference NULL pointers, dangling pointers, or access memory out of bounds. Segmentation faults are not acceptable.
- **Unsafe code detection:** Applications can't perform unsafe operations such as inline assembly (the kernel mode allows privileged instructions!).
- **Performance:** Safety checks should not be a bottleneck.

These requirements are Rust's strong points! It provides memory safety, potentially dangerous operations are marked as `unsafe`, and they are performant because the Rust compiler ensures them at compile time (so-called *"zero cost abstraction"*).

## Memory Safety without GC

A common misconception is that memory safety is unique to Rust, but it is not. JavaScript, Python, Go, PHP are also memory safe, and actually we plan to support one of them (TypeScript) in the future for rapid prototyping.

The difference is that Rust provides memory safety without garbage collection (GC) which incurs additional overhead at run-time. This is what makes Rust unique and why it's great for OS components.

> [!TIP]
>
> **Rust is not always *"zero-cost"*:**
>
> Rust's memory safety is more like a *"pay-as-you-go"* cost. Even in Rust, you sometimes need to pay the cost for memory safety like reference counting (`Arc<T>`), or runtime ownership guarantees (`Mutex<T>`).

## Rust does not Guarantee *"Safe"* or *"Bug-free"*

Rust has been praised as *"safe"* but writing in Rust does not mean your program is bug-free. Its memory safety and fearless concurrency are huge advantages, but they are just few of bugs that can happen. For example:

- Logic errors
- Dead locks
- Assertions and panics (`panic!`)
- Memory leaks

This means you still need to trust applications you run in FTL's kernel space. Rust's based isolation (what we call *"unikernel-style"* isolation) is a trade-off between security and performance, and we consider it is very reasonable for trusted components.

## Rust is a Seatbelt

Rust's memory safety is not why people love the programming language. It is because Rust is a *"seatbelt"* for your program. Its shared-xor-mutable references model encourage you to write more straightfoward and maintainable code, algebraic data type (i.e. `enum`) and pattern matching make you aware of possible unhappy paths, [new type pattern](https://doc.rust-lang.org/rust-by-example/generics/new_types.html) distinguishes different meanings of integers, and `Option` and `Result` types help you handle errors gracefully.

Seatbelts are sometimes annoying indeed, but we know it saved us from countless bugs by enforcing good practices. Notably, you won't need address sanitizer most of the time when writing Rust because it crashes gracefully, instead of silently corrupting the memory and appear as a random hard-to-debug bug later.

## Simple Rust

This concept is inspired by [Simple English Wikipedia](https://simple.wikipedia.org/wiki/Wikipedia:About#Simple_English). The idea is to provide intuitive and easy-to-understand Rust APIs, by sacrificing some flexibility and efficiency.

Let's say take a look at our `Application` trait, which is the core of FTL's application programming model:

```rust
// Excerpt from ftl/src/application/mod.rs
pub trait Application: Sized {
    /// The per-object session. Notice that this does not
    /// require Send + Sync.
    type Session;

    // Called when a data message is received.
    //
    // `ctx.session` contains `&mut Session`.
    fn receive_data(&mut self, ctx: &mut ChannelCtx<Self>, data: &[u8]);
}
```

`Application` trait provides methods to handle various events. The key point is it's `&mut self`, which means it is not thread-safe and needs to be used in a single thread.

Running in a single thread sounds like a huge step back from the modern async programming style. It is less efficient indeed, but it eliminates the need for mutexes and any other synchronization primitives you need to remember, and it's easier to debug and reason about the code.

This is one of the trade-offs we make when writing FTL - **prefer simplicity over perfect efficiency**. Less types, less lifetime specifiers, less macros, and less complex signatures.

## When not to Use Rust

The "Simple Rust" concept indicates that Rust can be cumbersome to use in some cases, and sometimes GC-backed languages fit better. For example, JavaScript's strings and `async`/`await` syntax would be more ergonomic for writing web applications than Rust.

This is why we plan to support TypeScript in the future. It is less efficient than Rust, but it allows you to write code faster and more easily. When it becomes bottleneck, you can switch to Rust. Make it work first, then make it fast. This is the philosophy of FTL.
