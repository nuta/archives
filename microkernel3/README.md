Resea
=====
[![Build Status](https://travis-ci.com/seiyanuta/resea.svg?branch=master)](https://travis-ci.com/seiyanuta/resea)
[![Maintainability](https://api.codeclimate.com/v1/badges/be1bfe8fec64510c1360/maintainability)](https://codeclimate.com/github/seiyanuta/resea/maintainability)

Resea is an operating system that aims to be aesthetic.

Features
--------
- **Microkernel-based design:** The operating system consists of *servers*, an normal userspace processes that provides serices such as device driver, file system, and TCP/IP.
- **Everything is a message passing:** Want to read a file? Send a message to file server. Want to serve HTTP? Send a listen request to net server. Message formats are defined in [IDL](https://github.com/seiyanuta/resea/tree/master/interfaces).
- **Servers written in Rust:** Take a look at [virtio-blk device driver](https://github.com/seiyanuta/resea/blob/master/servers/virtio-blk/src/main.rs). It looks pretty compared to other microkernel server code, doesn't it?
- **x64 support:** Of course we have.
- **Work-in-progress features:** FAT32, TCP/IP protocol stack, and Virtio block/network driver.


Building
--------
```
$ ./tools/bootstrap
$ make -j8 SERVERS="benchmark-client benchmark-server"
```

Running
--------
```
make -j8 SERVERS="benchmark-client benchmark-server" run
```

License
-------
CC0 or MIT.
