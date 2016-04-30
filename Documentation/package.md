Package & Executable
====================

Package
-------
A *package* is an isolated component of Resea. There are 5 categories:

### Application
A application packages is [application](https://en.wikipedia.org/wiki/Application_software). For example, device drivers, TCP/IP, file system drivers, and OS servers.

### Library
A library package is [library](https://docs.python.org/3/library/). Printf library, for example.

### Interface
A interface packagae contains message definitions.

### HAL
A HAL (Hardware Abstraction Layer) package is a special library that satisfies HAL requirements.
It contains CPU-dependent stuffs: interrupt/exception handling, MMU abstraction, boot code, etc.


### Language
A language package is a special package that satisfies HAL requirements.
It defines a build rules of a programming language and contains runtime libraries.

Executable
----------

A *executable* is an executable file. In Linux,
[vmlinux](https://en.wikipedia.org/wiki/Vmlinux)
is a same concept. SDK compiles code in packages
and generate an executable. Every executable has one HAL package and an arbitrary number
of library, application, and language packages (interface library used only in building).

