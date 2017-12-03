---
title: Quickstart
---

**Currently MakeStack is severly unstable. You will have to debug MakeStack to try. Good luck!**

## How to use
1. Purchase a Mac.
2. Install Homebrew.
3. Clone https://github.com/seiyanuta/makestack
4. Start MakeStack Server.
5. Install MakeStack Linux ([Download an image from here!](https://github.com/seiyanuta/makestack/releases))
6. Edit `config.txt` in the MakeStack Linux disk.
7. Open [http://localhost:8080](http://localhost:8080).

## Server

```bash
$ brew install postgresql redis yarn
$ brew services list # make sure that postgresql and redis are running

$ cd server
$ bundler install --path vendor/bundle
$ cd ui
$ yarn
$ rails db:migrate
$ foreman start
```

## Client
```bash
$ cd client
$ yarn
$ ./bin/makestack --help
```

## Installer
```bash
$ cd installer
$ yarn
$ yarn dev
```

## Linux
```bash
$ cd linux
$ make TARGET=raspberrpi3
$ ls build/raspberrypi/bootfs build/raspberrypi/*.img
```
