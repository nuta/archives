# Channel

Channel is an Inter-Process Communication (IPC) mechanism in FTL.

## Overview

Channel is:

- A bounded message queue, where each message contains a byte array and a handle array.
- Connected to its peer channel and is bi-directional. Messages are delivered to the peer channel.
- Asynchronous and non-blocking.
- Movable between processes.

## Channel vs. `std::sync::mpsc`

[`std::sync::mpsc`](https://doc.rust-lang.org/std/sync/mpsc/fn.channel.html) (`mpsc` for short) is a similar API in Rust's standard library. Both our channel and `mpsc` channel are message queues, but they are different in some aspects.

### Handle Transfer

`mpsc` channel is an inner-process communication mechanism. FTL channel is an inter-process communication mechanism, which supports transferring handles between processes.

### Uni-directional vs. Bi-directional

`mpsc` channel creates a pair of sender and receiver (uni-directional):

```rust
let (tx, rx) = std::sync::mpsc::channel();
```

Our channel creates a pair of channels connected to each other (bi-directional):

```rust
// FTL channel
let (ch1, ch2) = Channel::new();
```

Both `ch1` and `ch2` provides `send` and `recv` methods. `send`-ing a message using `ch1` will deliver the message to `ch2`, and vice versa. This is similar to TCP connections where both sides can send and receive data.

## Message

A message is a unit of transfer between channels. It is a packet-like structure that contains a byte array and a handle array.

We use a few pre-defined message types:

| Name | Fields  | Description |
|------|---------|-------------|
| `Connect` | `ch: Channel, uri: bytes` | A new connection to the server. `ch` is a new control channel connected to the client. `uri` is application-specific context to identify the connection. |
| `Data` | `data: bytes` | A frame/chunk of data, such as network packets or file contents. |
| `Open` | `cookie: Cookie, uri: bytes` | Open a resource, such as a file or a TCP socket. |
| `OpenReply` | `cookie: Cookie, ch: Channel` | Reply to an `Open` request with a new channel. |
| `ErrorReply` | `cookie: Cookie, error: ErrorCode` | Reply to an `Open` request when it fails. `error` is the error code. |

> [!TIP]
> `Close` message is not defined because you can simply close the channel.

> [!NOTE]
> **Design decision: Schema-less IPC**
>
> Unlike other IPC systems like gRPC, Fuchsia's FIDL, or Mach's MIG, FTL does not use Interface Definition Language (IDL) to define message types.
>
> This approach sacrifices some type safety and flexibility. However, IDL requires auto-generated code (IPC stubs) which introduces another layer of abstraction to learn, and adds tons of verbose code into the Git repository.
>
> My finding is that OS service interfaces are generally simple, compared to applications. By providing a limited set of message types, OS services have similar IPC patterns, which makes it easier to understand and optimize.

## Channel Types

For convenience, there are a couple of categories of channels based on how they are used:

- **Control Channel**: A channel used to send open requests (`Open` and `OpenReply` messages). The reply contains a new listening channel (TCP), or a new data channel (file), based on the request.
- **Listening Channel**: A channel used to accept new data channels (`Connect` messages).
- **Data Channel**: A channel used to send and receive data associated with the TCP connection or a file (e.g. `Data` messages).

Let's look at an example of a TCP server in a pseudo code to learn the differences.  In this example, `tcpip_ch` is a control channel, `listen_ch` is a listening channel, and `conn_ch` is a data channel:

```ts
/* Pseudo code */

// Control channel: Send an open request to the TCP/IP server, typically
// automatically injected via environ.
await tcpip_ch.send("Open", { uri: "tcp-listen:0.0.0.0:8080" });

// Wait for a reply from the TCP/IP server.
const { ch: listen_ch } = await tcpip_ch.recv("OpenReply");

for (;;) {
    // Listening channel: Wait for a new TCP connection.
    const { ch: conn_ch } = await listen_ch.recv("Connect");

    // Data channel: Read data from the client until the connection
    // is closed.
    for await (const { data } of conn_ch.recv("Data")) {
        console.log(`received ${data.length} bytes from a client`);
    }
}
```

> [!TIP]
> Doesn't the code look like socket programming in UNIX? Yes, in UNIX-like systems, sockets are also categorized into listening sockets and connected sockets. They are indeed abstracted as files, but used for different purposes just like FTL channels!

## URI

URI is a byte sequence (typically an ASCII string) to specify a resource to open. It is used to identify a resource, such as a TCP socket.

> [!NOTE]
>
> We don't aim to conform to the URI standard (RFC 3986) strictly. It's up to the application to define format.

### Schemes

The following schemes are used today in FTL:

| URI format | Description | Example | Applications |
|--------|-------------|---------------|---------|
| `tcp-listen:<host>:<port>` | Listen on a TCP port. | `tcp-listen:0.0.0.0:8080` | HTTP server |
| `ethernet:<mac>` | Register a network device. | `ethernet:00:11:22:33:44:55` | Network device driver |

## Backpressure

If the queue is full, the kernel returns an error immediately without waiting for the peer's queue to be writable. This is called *backpressure*, which is a common pattern in asynchronous programming to avoid overwhelming the system ([Node.js](https://nodejs.org/en/learn/modules/backpressuring-in-streams)). Handling backpressure is a crucial part of writing a robust system.

### Backpressure when Sending

When the peer's queue is full, the best practice is to call `eventloop.listen(ch, Readiness::WRITABLE)` to listen for the peer's queue to be writable, and retry sending the message later when `Event::ChannelWritable` happens.

### Backpressure when Receiving

In some cases you want to stop consuming messages when the internal buffer is full, for example, TCP write buffer. In this case, you can call `eventloop.unlisten(ch, Readiness::READABLE)` to stop receiving `ChannelReadable` events, and call `eventloop.listen(ch, Readiness::READABLE)` to resume receiving.
