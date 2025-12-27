# Environ

Environ (short for *environment*) is a set of key-value pairs passed to a program when it starts.


## Environment Variables beyond Strings

Environ is similar to environment variables in UNIX-like operating systems. The key difference is values are not limited to strings - they can be numbers or even handles:

| OS | Key | Value |
|------|---------|-------------|
| FTL | `String` | `String \| Number \| Handle` |
| UNIX | `String` | `String` |

> [!NOTE]
>
> We plan to support more types as needed.

## Service Discovery

Environ is not only for configuration. It is also used for service discovery and service exports.

When instantiating a program, the system injects channels connected to other programs into the environ such as TCP/IP, file system, network device driver, etc.

## Declarative Initialization

Environ is defined declaratively in the program's source code. In Rust, it is the public `SPEC` constant, defining what items are required to run the program.

It also defines the services that a program provides so that other programs can discover and connect to it through the environ.

> [!NOTE]
> **Design decision: No command-line arguments**
>
> In UNIX-like systems, each program has its own command-line parser. While it is useful when typing commands in shell, you need to learn every command-line syntax.
>
> In FTL, environ is the only way to configure a program. No ambiguity.
