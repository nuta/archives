Messaging
==========

Channel
-------
A *channel* is a bidirectional endpoint of inter-thread (including ones in other thread
groups) communciation. In other OS kernel, TCP/UDP port or socket is a
almost same concept. It can be used asynchronously and synchronously.

Threads sends messages from a chanel to another channel. This is the only
way to communicate with threads in another thread group.

Message
-------
A *message* is a variable-length continuous memory block. Payload Headers contains
the types of next 8 payloads. A payload is an arbitrary integers (inline), a pointer
to a memory buffer (out-of-line), or a channel. Size of a payload and a payload header
is equal and it depends on the environment. Generally it is `sizeof(uintmax_t)`.

The first payload shall be a message ID.


### OoL Payload
Like *Move Semantics* in C++, there are some OoL payload transfer strategies for efficiency:

- **copy:** The memory block is copied. Both sender and receiver have ownerships. This is the default.
- **move:** The memory block is not copied. Sender loses the ownership.
- **readonly:** The memory block is not copied. Receiver can read the block only until it sends a reply message; this type is usable in handling reuqest message in request-response (client-server) messaging.

Note that these strategies are only effective in kernel-space. In user-space, stubs
provide same semantics but all OoL messages are copied internally.

### Channel Payload
The behavior depends on the channel. If channel is:

- **linked:** A channel linked to the source channel linked to shall be created.
- **not linked:** A channel linked to the source channel shall be created.

## Message structure

```
+----------------------+
|   Payload Header 0   |
|----------------------|
|       Payload 0      |
|----------------------|
|       Payload 1      |       n           31       28        7       4 3       0
|----------------------|        +-----------------------------------------------+
|           .          |        | (unused)  | type 8 |   ...  | type 1 | type 0 |
|           .          |        +-----------------------------------------------+
|           .          |
|----------------------|
|       Payload 7      |             Types:
|----------------------|
|   Payload Header 1   |                 0x0: NULL payload
|----------------------|                 0x1: inline payload
|       Payload 8      |                 0x2: channel payload
|----------------------|                 0x3: out-of-line payload (copy)
|           .          |                 0x4: out-of-line payload (move)
|           .          |                 0x5-0xf: undefined
|           .          |
+----------------------+
```

System call
-----------
```
open() -> ch
```
Create a new channel.

```
close(ch)
```
Destroy a channel.

```
wait(ch)
```
Wait for a new message.

```
setoptions(id, handler, buffer)
```
Set channel options.

```
send(ch, m, size)
```
Send a message.

```
recv(ch) -> m
```
Receive a message.

```
call(ch, m, size, buffer)
```
Send a message and then receive a message.

```
link(ch1, ch2)
```
Connect ch1 and ch2 each other. Messages to `ch1` arrives to `ch2` and
vice versa.

```
transfer(src, dest)
```
Transfer messages to `src` to `dest`. Messages arrived to `src` is not
saved `src`, instead, they are saved to `dest`. Their Channel ID are
not modified to `dest` (retain `src`).

Interfaces
-----------
The kernel does not care about semantics of a message. Thread have to determine the
type of a message and know the its structure. *Interface* is a set of message definitions just
like [Protocol Buffers](https://developers.google.com/protocol-buffers/) which defines
structures of messages.
