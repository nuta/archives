Resea
=====

[![Build Status](https://travis-ci.org/resea/resea.svg?branch=master)](https://travis-ci.org/resea/resea)
[![Code Climate](https://codeclimate.com/github/resea/resea/badges/gpa.svg)](https://codeclimate.com/github/resea/resea)

Resea is a sweet, flexible, delightful operating system kernel.

Features
--------

- Straightfoward messaging passing.
- Seamless kernel-space/user-space integration.
- Portable hardware abstraction layers.
- Relatively small footprint (lower than 8KB).

Building
--------

**example:**
```
$ ./genconfig ARCH=esp8266 TARGET=kernel APPS="channel-server makestack-driver app"
$ make -j2
$ file build/image
```

Testing
-------

```
$ ./tools/ktest
```

License
-------

Public Domain.
