ToDo
=====

Message passing
---------------
- Channel payload
  - create a new channel connected to sender's channel
  - create a new channel connected to the other side of sender's channel

- Session payload
  - Random number verified by the kernel
  - New session id is created if the session payload is 0

io
--
- deprecate `IORead` and `IOWrite`
- introduce `.map(addr_space, addr, size) -> (r, addr)`
- use the native IO instruction functions defined in lang

Config
------
- embed into the binary by `objcopy(1)`
- embed interface id-name hash by SDK
- conditional branch

Distributed computing
---------------------
- auth
- rpc
- scheduling: where to run

- zone: LAN
- node: a computer
- link: connection between zones

Channel chain
-------------
- `.register_with_name(ch, name)`
- `.listen_name(ch, name)`
- `.reserve_name(ch, name)`

- `.register_with_prefix(ch, prefix)`
- `.listen_prefix(ch, prefix)`
- `.reserve_prefix(ch, prefix)`

Usecase: `/dev/sd` (kernel appends id to the prefix)

Exception
---------
Divide by zero error, for example. Exception message will be
sent to all registered channels in a thread group (not a thread).

- `.register(ch, type)`
- `.exception(type, arg)`

Linux support
-------------
- Linux ABI
- OS server

misc.
-----
- test mock
- back tracing

