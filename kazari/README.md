# Kazari - A no_std GUI software stack (Work-in-Progress)

<div align="center">
    <img src="https://raw.githubusercontent.com/nuta/kazari/main/screenshot.png">
</div>

<div align="center">
:construction: <b>Kazari is still in the very early stage of development.</b> :construction:
</div>

Kazari is a no_std GUI software stack which provides a
graphic desktop environment based on [Wayland Protocol](https://wayland.freedesktop.org/).

It's orignally designed for [Resea](https://github.com/nuta/resea), a microkernel-based
minimalistic operating system written from scratch. However, Kazari is platform-agnostic
and it should be easy to port to your operating system.

Features
--------
- It consists of the server (so-called *compositor*) and clients (applications) connected over [Wayland Protocol](https://wayland.freedesktop.org/docs/html/apa.html), a well-known display protocol as a promising alternative to X Window System.
- Compositing window manager library (window, mouse cursor, ...).
- A 2D graphics library to render window contents in the client side (drawing lines, rectangles, texts, images, ...).
- Implemented in no_std (but depends on liballoc).
- The HTML5 Canvas API backend for debugging.

Backends
--------
- Web: `web/src/lib.rs`
- UEFI Application: `demo/uefi/` (**incomplete**)
- Resea: **ToDo**

How to try
----------
Using the Web-based backend is the best way for Kazari development.

1. Install [`wasm-pack`](https://rustwasm.github.io/docs/wasm-pack/quickstart.html)..

2. Start the local web server. It automatically rebuilds and reloads the web page when you updated a source file.
```
$ cd web
$ yarn dev
```

----

Running wayland-scanner
-----------------------
Since the original `wayland-scanner` does not support generating Rust code, we have
our own code generator not to write message definitions by hand.

```
./wayland-scanner.py src/wl/protocols protocols/*.xml
```

References
----------
- [Introduction - The Wayland Protocol](https://wayland-book.com/introduction.html)
- [Wayland Protocol Specification](https://wayland.freedesktop.org/docs/html/apa.html)

License
-------
See [LICENSE.md](https://github.com/nuta/kazari/blob/main/LICENSE.md).
