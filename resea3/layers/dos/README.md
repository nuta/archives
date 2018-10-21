DOS-like Operating System
==========================

*dos* is an operating system like good old DOS systems. This toy OS is developed
to show you how to create an OS (*layer*) on Resea and there are some diffierences
to genuine DOSes:

- **Runs in 64-bit mode:** because Resea does not support real (16-bit) mode and implementing the feature would be complex and quite boring job.
- **No binary compatiblity:** because apps run in 64-bit mode and Resea does not support INT instruction system calls.
- **Multiple address space:** an app is in an isolated address space.

Directory Structure
-------------------
```
.
├── bin       -- Applications
├── kext      -- Kernel extension: system call handler, etc.
└── lib       -- Libraries for applications
```

Building
--------
```
$ make -f layers/dos/Makefile
```

Run on QEMU
-----------
```
$ make -f layers/dos/Makefile run-gui
```
