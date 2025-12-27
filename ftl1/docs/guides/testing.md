# Testing

FTL provides multiple ways to test your code.

## Unit Testing

FTL kernel can be built as a normal Rust application. It means you can just use `cargo` to run unit tests:

```bash
cargo test
```

See [Tests - Rust by Example](https://doc.rust-lang.org/rust-by-example/cargo/test.html) for more details.

> [!TIP]
> [cargo-nextest](https://nexte.st/) is a recommended drop-in replacement for `cargo test`.

## Integration Testing

You can run integration tests that boot a QEMU VM and observe how the OS behaves. To run tests, use `bin/ftl test` command:

```bash
bin/ftl test
```

Tests are in the `tests` directory, and are written in TypeScript.

```ts
import { describe, expect, it } from "bun:test";
import { boot } from "./helpers/emulator";

describe('Hello World', () => {
    it('prints a boot message', async () => {
        using vm = await boot();
        await vm.waitForLogs(async (logs) => {
            expect(logs).toContainEqual({
                name: 'kernel',
                level: 'info',
                message: 'Booting FTL...'
            });
        });
    });
});
```

See [Writing Tests - Bun](https://bun.com/docs/test/writing-tests) for more details.
