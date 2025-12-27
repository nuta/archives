# Debugging

## Printf Debugging

Add `info!`, `trace!`, and `debug!` everywhere you want to print debug information. Since incremental builds are fast, this is the quickest way to debug the OS.

> [!WARNING]
> `trace!` and `debug!` logs are disabled in release builds.

## LLDB

1. Run `bin/ftl run` to start QEMU. It also starts a debugger server listening on port 7778.
2. Attach to the QEMU with LLDB:

```bash
bin/ftl debug
```

> [!WARNING]
>
> Recent macOS releases broke GDB stub support for HVF-accelerated QEMU. Upgrade QEMU to v10.1.0 or later ([commit](https://gitlab.com/qemu-project/qemu/-/commit/90f0078d023fc364c870188075f530f84f652758)).

Now you can add breakpoints, print variables/backtrace, and more like:

```
$ bin/ftl debug
(lldb) bt
* thread #1, stop reason = signal SIGTRAP
  * frame #0: 0xffffff808021ccf2 ftl.elf`idle at idle.rs:46:13
    frame #1: 0xffffff80802132a6 ftl.elf`return_to_user at thread.rs:187:9
    frame #2: 0xffffff808021d2d8 ftl.elf`handle_syscall at syscall.rs:94:5
    frame #3: 0xffffff8080263d64 ftl.elf`syscall at syscall.rs:49:26 [inlined]
    frame #4: 0xffffff8080263d48 ftl.elf`syscall4 at syscall.rs:24:5
    frame #5: 0xffffff808026358a ftl.elf`wait at poll.rs:59:19
    frame #6: 0xffffff80802164e2 ftl.elf`wait<apiserver::Main> at application.rs:257:45 [inlined]
    frame #7: 0xffffff808021647a ftl.elf`run<apiserver::Main> at application.rs:306:46
    frame #8: 0xffffff8080216e20 ftl.elf`main<apiserver::Main> at application.rs:435:15
```

See [Tutorial - LLDB](https://lldb.llvm.org/use/tutorial.html) to learn commands in LLDB.
