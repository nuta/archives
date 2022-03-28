# Device-side JavaScript
The app's JavaScript program consists of two contexts: *server context* and *device context*:

```js
/* A server context: never executed in the device-side. */
app.onEvent("my-sensor-data", (value) => {
    console.log("received a data from a device:", value);
})

app.onReady((device) => {
    /* A device context: in the server-side, this callback is ignored. */
    while (1) {
        device.publish("my-sensor-data", analogRead(10));
        device.delaySeconds(15);
    }
})
```

In the server-side, the example above is simply executed as it is. The device context declarations (`app.onReady`) does nothing.

In the device-side, since we don't want to implement a full-featured JavaScript engine, we utilize *transpilation* approach. The transpiler (`src/transpiler`) transpiles device contexts into a device-side C++ program. In the example above, the transpiler extracts the device context (the callback of `app.onReady`), converts the abstract syntax tree into a C++ source code, and then the build system compiles the C++ file as a part of the firmware.

The runtime library of the generated C++ program behaves differently from full-featured JavaScript engines and is incomplete as described below.

## Differences from the *real* JavaScript (ECMAScript)
- Silly implicit type conversions are not supported.
    - Are you really want to execute `1 + true`?
- No exceptions.
- No `require`.
- No `console.log`.
    - Use `print` API instead.

## Major missing features
- No global variables.
    - You have to define constants like `LED_PIN` in the callback of `device.onReady`, not in the global scope.
- No object literals.
- No classes.
- No standard libraries such as RegExp and JSON.
- Block scope is not yet supported.

## Supported features
### Supported types
- number (integer only; floating-point number is not yet supported)
- string (methods are not yet implemented)
- boolean
- undefined
- null
- object

### Supported operators
- Binary operators (all type): `==, !=, <, >, <=, >=`
- Binary operators (number & string): `+`
- Binary operators (number only): `+, -, *, / >=, &, |, ^, <<, >>, %`
- Compound operators (number only): ` =, +=, -=, *=, /=,  &=, |=, ^=, <<=, >>=`
- Unary operators (number only): `!, ~, +, -`
- Logical operators: `||, &&`

### Supported expressions
- Conditional expression: `(x) ? a : b`
- Template literal: ``` `Hello ${name}!` ```
- Accessing a property: `foo.bar`
- Call: `foo()`
- Increment/decrement: `x++, y--`
- Arrow function: `(a, b, c) => { ... }`

### Statements
- `let`, `const`, `var` (all variables are defined in a function scope and `const` is handled as `let` for now)
- `if`
- `for`
- `while`
- `do-while`
- `break` / `continue` (labels are not supported)
- `return`
