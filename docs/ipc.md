Inter Process Communication
===========================

In Resea world everything is channel. Want to spawn a new thread? Create a channel, connect
it to a process server, and send a thread creation request message through the channel.

The messages are defined in *Interface Definition Language*, IDL in short. Fundamental ones are
in `interfaces/`.

Message Structure
------------------

A message consists of header and payloads. *Header* is a 64-bit value in the following format:

```

+--------------------------------------------------------------------------+
|              reserved            | interface |   msg   |  types  | error |
+--------------------------------------------------------------------------+
```
