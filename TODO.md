ToDo
=====

Config
------
- embed into the binary by `objcopy(1)`
- embed interface id-name hash by SDK

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

