# Writing Applications

In this tutorial, you will learn how to write a simple application in Rust.

## Scaffolding

Applications are located in the `apps/<name>` directory. Create a new directory for your application:

```
cargo new --lib apps/tutorial
```

> [!NOTE]
>
> Use `--lib` flag to create a library crate. Applications are built as libraries to support [Unikernel isolation](../learn/isolation) transparently.

## Hello World!

```toml
// apps/tutorial/Cargo.toml
[dependencies]
ftl = { workspace = true } // [!code ++]
serde = { workspace = true } // [!code ++]
```

```rs
// apps/tutorial/src/lib.rs
#![no_std]

use ftl::application::Application;
use ftl::application::InitCtx;
use ftl::prelude::*;
use ftl::spec::AppSpec;

pub const SPEC: AppSpec = AppSpec {
    name: "tutorial",
    start: |vsyscall| ftl::start(vsyscall, main),
    env: &[],
    exports: &[],
};

#[derive(serde::Deserialize)]
struct Environ {}

fn main(env: Environ) {
    info!("Hello, world!");
}
```

- `![no_std]` means you can't use `std` library. Use FTL's own standard library (`ftl`) instead.
- `SPEC` declares the application's specification, especially what the application requires to run (`env`), and services the application provides to others (`exports`).

> [!TIP]
>
> When `main` function returns, the process will be terminated automatically. Threads will be aborted as well.

## Adding the Application

1. Add the application to the `kernel/Cargo.toml` file:

```toml
[dependencies]
tutorial = { path = "../apps/tutorial" } // [!code ++]
```

2. Add the application to the `kernel/src/startup.rs` file:

```rs
const INKERNEL_APP_SPECS: &[&AppSpec] = &[
    &::tutorial::SPEC, // [!code ++]
];
```

## Running the Application

Let's build and run the application. Run `run.sh` script:

```ansi
$ bin/ftl run
[1m==> Starting QEMU 64-bit Arm (HVF hardware acceleration)[0m
[kernel      ] [96mINFO  [0m Booting FTL...
[kernel      ] [0mDEBUG [0m reserved area: 0000000040200000 - 00000000404bf000 (2 MiB)
[kernel      ] [0mDEBUG [0m reserved area: 0000000040000000 - 0000000040100000 (1 MiB)
[kernel      ] [96mINFO  [0m free RAM area: 0000000040100000 (1 MiB)
[kernel      ] [96mINFO  [0m free RAM area: 00000000404bf000 (251 MiB)
[tutorial    ] [96mINFO  [0m Hello, world!
```

Congratulations! You've successfully run your first application! 🎉

> [!INFO]
>
> **What happens internally?**
>
> 1. The system (kernel) reads the `SPEC` to understand what the application requires to run, and construct an environment.
> 2. Kernel creates a new process and a thread for the application, and starts the thread from the `start` function specified in the `SPEC`.
> 3. `main` function prints the hello world message.

## Service Discovery

Service discovery is a mechanism to connect to other applications. In this example, let's connect to the `echo` service, which replies with the same message we sent.

To connect to other applications, add the service name to the `env` field in the `SPEC` declaratively:

```rs
use ftl::spec::EnvItem; // [!code ++]
use ftl::spec::EnvType; // [!code ++]

pub const SPEC: AppSpec = AppSpec {
    name: "tutorial",
    start: |vsyscall| ftl::start(vsyscall, main),
    env: &[
        EnvItem { // [!code ++]
            name: "echo" /* name in the env */, // [!code ++]
            ty: EnvType::Service { name: "echo" /* service name */ }, // [!code ++]
        }, // [!code ++]
    ],
    exports: &[],
};
```

Add the channel field to the `Environ` struct:

```rs
// apps/tutorial/src/lib.rs
use ftl::channel::Channel; // [!code ++]

#[derive(serde::Deserialize)]
struct Environ {
    echo: Channel, // [!code ++]
}
```

That's it! The system will do service discovery and automatically and inject the channel the envrionment. Let's dump what we got:

```rs
// apps/tutorial/src/lib.rs
fn main(env: Environ) {
    info!("Hello, world!");
    info!("echo channel: {:?}", env.echo); // [!code ++]
}
```

```ansi
$ bin/ftl run
...
[tutorial    ] [96mINFO  [0m Hello, world!
[tutorial    ] [96mINFO  [0m echo channel: Channel { handle: OwnedHandle(HandleId(3)) }
```

> [!INFO]
>
> **Exposing Services:**
>
> To expose a service, declare the service name in the `exports` field in the `SPEC`:
>
> ```rs
> pub const SPEC: AppSpec = AppSpec {
>     name: "echo_server",
>     start: |vsyscall| ftl::start(vsyscall, main),
>     env: &[],
>     exports: &[
>        ExportItem::Service { name: "echo" }, // [!code ++]
>    ],
> };
> ```

## Sending a Message

Now we have a channel to the `echo` service. Let's send a message to it:

```rs
// apps/bin/tutorial/src/lib.rs
use ftl::channel::Message; // [!code ++]

fn main(env: Environ) {
    info!("Hello, world!");
    info!("echo channel: {:?}", env.echo);

    env.echo.send(Message::Data { data: b"Hi!" }).unwrap(); // [!code ++]
}
```

As described in [Channel](../learn/channel), there are some pre-defined message types. In this example, we send a data message (`Message::Data`) with a byte array `Hi!`:

```ansi
$ bin/ftl run
...
[tutorial    ] [96mINFO  [0m Hello, world!
[tutorial    ] [96mINFO  [0m echo channel: Channel { handle: OwnedHandle(HandleId(3)) }
...
[echo        ] [96mINFO  [0m echoing back "Hi!"
```

We can see the message is received by the `echo` service!

## Waiting for a Message

`EventLoop` API provides a way to wait for events to happen, such as *"a message has been arrived on a channel"*. It's a polling-based API similar to Linux's `epoll`, and likewise you can use it for other objects such as timers and interrupts.

Update the `main` function to use the event loop:

```rs
// apps/bin/tutorial/src/lib.rs
use ftl::eventloop::EventLoop; // [!code ++]

fn main(env: Environ) {
    info!("Hello, world!");
    info!("echo channel: {:?}", env.echo);
    
    env.echo.send(Message::Data { data: b"Hi!" }).unwrap();

    let mut eventloop = EventLoop::new().unwrap(); // [!code ++]

    // Register the channel to the event loop. // [!code ++]
    eventloop.add_channel(env.echo, ()).unwrap(); // [!code ++]

    loop { // [!code ++]
        // Wait for an event to happen. // [!code ++]
        let (_, event) = eventloop.wait().unwrap(); // [!code ++]
        match event { // [!code ++]
            Event::ChannelReadable { ch } => { // [!code ++]
                info!("a new message available!"); // [!code ++]
            } // [!code ++]
            _ => { // [!code ++]
                warn!("unexpected event: {:?}", event); // [!code ++]
            } // [!code ++]
        } // [!code ++]
    } // [!code ++]
}
```

## Receiving a Message

To receive a message, call `Channel::recv` method:

```rs
fn main(env: Environ) {
    info!("Hello, world!");
    info!("echo channel: {:?}", env.echo);

    env.echo.send(Message::Data { data: b"Hi!" }).unwrap();

    let mut eventloop = EventLoop::new().unwrap();

    // Register the channel to the event loop.
    eventloop.add_channel(env.echo, ()).unwrap();

    loop {
        // Wait for an event to happen.
        let (_, event) = eventloop.wait().unwrap();
        match event {
            Event::ChannelReadable { ch } => {
                let msg = ch.recv().unwrap(); // [!code ++]
                match msg { // [!code ++]
                    Message::Data { data } => { // [!code ++]
                        info!("received {:?}", core::str::from_utf8(data)); // [!code ++]
                    } // [!code ++]
                    _ => { // [!code ++]
                        warn!("unexpected message: {:?}", msg); // [!code ++]
                    } // [!code ++]
                } // [!code ++]
            } 
            _ => {
                warn!("unexpected event: {:?}", event);
            }
        }
    }
}
```

```ansi
$ bin/ftl run
...
[tutorial    ] [96mINFO  [0m Hello, world!
[tutorial    ] [96mINFO  [0m echo channel: Channel { handle: OwnedHandle(HandleId(3)) }
[echo        ] [96mINFO  [0m echoing back "Hi!"
[tutorial    ] [96mINFO  [0m received Ok("Hi!")
```

It worked! We received the message echoed back by the `echo` service.

> [!NOTE]
>
> `Channel::recv` method is non-blocking. If there is no message available, it will return an error.

## Per-Object Context

We've seen the basics of message passing. In real-world applications, we often need to maintain some state to handle the messages.

For example, what if we want to send a message to the `echo` service again, but up to N times? This is where per-object context comes into play.

### Defining a Context

Typically you'll define an enum to represent contexts:

```rs
enum Context { // [!code ++]
    Echo { // [!code ++]
        remaining: usize, // [!code ++]
    } // [!code ++]
} // [!code ++]
```

### Using the Context

```rs
fn main(env: Environ) {
    info!("Hello, world!");
    info!("echo channel: {:?}", env.echo);
    
    let mut eventloop = EventLoop::new().unwrap();
    eventloop.add_channel(env.echo, Context::Echo { remaining: 5 }).unwrap(); // [!code ++]

    loop {
        let (ctx, event) = eventloop.wait().unwrap(); // [!code ++]
        match (ctx, event) { // [!code ++]
            (Context::Echo { remaining }, Event::ChannelReadable { ch }) => { // [!code ++]
                let msg = ch.recv().unwrap();
                match msg {
                    Message::Data { data } => {
                        info!("received {:?}", core::str::from_utf8(data));
                    }
                    _ => {
                        warn!("unexpected message: {:?}", msg);
                    }
                }

                if remaining == 0 { // [!code ++]
                    let id = ch.handle_id(); // [!code ++]
                    eventloop.remove(id).unwrap(); // [!code ++]
                } else { // [!code ++]
                    let text = format!("Hi {}", remaining); // [!code ++]
                    ch.send(Message::Data { data: text.as_bytes() }).unwrap(); // [!code ++]
                    *remaining -= 1; // [!code ++]
                } // [!code ++]
            }
            _ => {
                warn!("unexpected event: {:?}", event);
            }
        }
    }
```

```ansi
$ bin/ftl run
...
[tutorial    ] [96mINFO  [0m received "Hi!"
[echo        ] [96mINFO  [0m echoing back "Hi 5"
[tutorial    ] [96mINFO  [0m received "Hi 5"
[echo        ] [96mINFO  [0m echoing back "Hi 4"
[tutorial    ] [96mINFO  [0m received "Hi 4"
[echo        ] [96mINFO  [0m echoing back "Hi 3"
[tutorial    ] [96mINFO  [0m received "Hi 3"
[echo        ] [96mINFO  [0m echoing back "Hi 2"
[tutorial    ] [96mINFO  [0m received "Hi 2"
[echo        ] [96mINFO  [0m echoing back "Hi 1"
[tutorial    ] [96mINFO  [0m received "Hi 1"
[tutorial    ] [0mTRACE [0m [ftl::handle] dropping handle HandleId(3)
```

Nice! It sends the message 5 times and then closes the channel correctly.

> [!TIP]
>
> **Async programming without `async` Rust:**
>
> As you've seen, `main` function is a single-threaded event-driven API. This might sound like a step back from the modern async programming style, however, it's very intuitive and easy to write applications. You don't need mutexes and can rely on the borrow checker.
>
> Instead of using `async`/`await` syntax, you need to manually write state machines in `Context` to do things asynchronously. This might be a bit more verbose, but OS components are much simpler than your web applications.

## What's Next?

Now you've learned the basics of FTL programming. Applications, including TCP/IP stack, device drivers, are written in the same way.
