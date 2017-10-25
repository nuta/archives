codeclimate-clang-static-analyzer
=================================
[![Docker Automated build](https://img.shields.io/docker/automated/seiyanuta/codeclimate-clang-static-analyzer.svg)](https://hub.docker.com/r/seiyanuta/codeclimate-clang-static-analyzer/)

A CodeClimate engine with [Clang Static Analyzer](http://clang-analyzer.llvm.org/).

How to use
----------
1. Download the Docker image.
```
$ docker pull seiyanuta/codeclimate-clang-static-analyzer
$ docker tag seiyanuta/codeclimate-clang-static-analyzer codeclimate/codeclimate-clang-static-analyzer
```

2. Add config to your `.codeclimate.yml`.
```yml
engines:
  clang-static-analyzer:
    enabled: true
    config:
      script: "./configure && make"
```

3. Run locally.
```
$ cd <your-project>
$ codeclimate analyze --dev
```

**Please note that this engine builds your project in a Docker container.**

License
-------
CC0 or MIT. Choose whichever you prefer.