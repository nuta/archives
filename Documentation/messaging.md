Messaging
==========

Channel
-------
A *channel* is an endpoint of inter-thread (including ones in other thread
groups) communciation. In other OS kernel, TCP/UDP port or socket is a
almost same concept.

Threads sends messages from a chanel to another channel. This is the only
way to communicate with threads in another thread group.

Message
-------
A *message* is a variable-length continuous memory block. Payload Headers contains
the types of next 8 payloads. A payload is an arbitrary integers (inline), a pointer
to a memory buffer (out-of-line), or a channel. Size of a payload and a payload header
is equal and it depends on the environment. Generally it is `sizeof(uintmax_t)`.

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
TODO

```
open() -> ch
close(ch)
wait(ch)
setoptions(id, handler, buffer)
send(ch, m, size)
recv(channel) -> m
call(ch, m, size, buffer)
link(ch1, ch2)
transfer(ch1, ch2)
```

Interfaces
-----------
The kernel does not care about semantics of a message. Thread have to determine the
type of a message and know the its structure. *Interface* is a set of message definitions just
like [Protocol Buffers](https://developers.google.com/protocol-buffers/) which defines
structures of messages.

### Interface IDs used in official packages
- 1: channel
- 2: memory
- 3: pager
- 4: io
- 5: thread
- 6: datetime
- 7: exec
- 8: fs
- 9: tcpip
- 100: net_device
- 101: storage_device
- 102: datetime_device
