ena (work in progress)
====
[![Build Status](https://travis-ci.com/seiyanuta/ena.svg?branch=master)](https://travis-ci.com/seiyanuta/ena)
[![Build status (Windows)](https://ci.appveyor.com/api/projects/status/21hfclx5w65r1uax?svg=true)](https://ci.appveyor.com/project/seiyanuta/ena)

*ena* is an intuitive and embeddable programming language for scripting.

```swift
// This is an example.
class User {
    func new(age) {
        self.age = age;
    }
}

var mike = User(23);
```

- Dynamically-typed, class-based, and object-oriented.
- Clean and familiar syntax inspired by Swift and Rust.
- Small and portable implementation (tree-walk, mark-and-sweep/conservative GC).

Building
--------
```
$ make
$ ./ena
```

Hacking
-------
### Install additional packages
```
$ pip3 -r tools/requirements.txt
```

### Commands
```
$ make test        # Run all tests.
$ make autotest    # Build and run tests automatically.
$ make benchmark   # Run benchmarks.
$ make coverage    # Measure test coverage.
$ make doxygen     # Generate source code references (docs/src).
$ make scan-build  # Run Clang Static Analyzer to find bugs. (macOS only)
$ make valgrind    # Run tests in Valgrind to find memory leaks. (requires Docker)
$ make emscripten  # Compile into wasm.
```

License
-------
CC0 or MIT. Choose whichever you prefer.
