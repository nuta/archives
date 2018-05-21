EFI.js (WIP)
=============
[![Build Status](https://travis-ci.org/seiyanuta/EFI.js.svg?branch=master)](https://travis-ci.org/seiyanuta/EFI.js)

EFI.js is a JavaScript runtime environment for [Unified Extensible Firmware Interface](http://www.uefi.org/) built on V8.

Progress
--------

- [x] Port GNU EFI
- [x] Port musl
- [x] Port C++ runtime libraries
- [ ] Implement libc functions (in `src/libc.cpp`) **[WIP]**
- [ ] Implement `v8::base::OS` (in `src/v8-platform.cpp`)
- [ ] Run `console.log("Hello World!")`
- [ ] REPL
- [ ] Implement Node APIs using UEFI


Building
--------

### Requirements

1. Clone this repository.
```
$ git clone https://github.com/seiyanuta/EFI.js
```

2. Build & install build toolchain and make a coffee run.
```
$ make install-devtools
```

3. Build EFI.js. It's time to make a coffee run again.
```
$ make -j4
```

4. Finally You're done! Let's play with EFI on QEMU.
```
$ make run
```

License
-------
All parts of EFI.js **except** external libraries in `vendor` directory are licensed under MIT License.
