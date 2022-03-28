import { Transpiler } from "../transpiler";

function transpile(code: string): string {
    const transpiler = new Transpiler();
    return ignoreWhitespace(transpiler.transpile(code));
}

// Removes whitespaces and newlines to make toStrictEqual ignore them.
function ignoreWhitespace(code: string): string {
    return code.replace(/[ \n]/g, "");
}

test("Hello World!", () => {
    expect(transpile(`\
        const app = require("makestack");
        app.get("/foo", (req, res) => { res.send("Hi") });

        app.onReady((device) => {
            device.print("Hello World!");
        });
    `)).toStrictEqual(ignoreWhitespace(`
        VM_FUNC_DEF(__lambda_0, __closure_0) {
            VM_FUNC_ENTER1(__closure_0, "device");
            VM_CALL(VM_ANON_LOC(5),VM_MGET(VM_GET("device"), VM_STR("print")),
            1, VM_STR("Hello World!"));
            return VM_UNDEF;
        }

        void app_setup(Context *__ctx) {
          VM_CALL(VM_APP_LOC("(top level)", 4), VM_GET("__onReady"), 1,
                  VM_FUNC(__lambda_0, __closure_0));
        }
    `));
});

test("binary operators", () => {
    expect(transpile(`\
        const app = require("makestack");
        app.onReady(() => {
            let ans = 0;
            ans = 1 + 2 - 3 * 4;
            ans += 5;
        });
    `)).toStrictEqual(ignoreWhitespace(`
        VM_FUNC_DEF(__lambda_0, __closure_0) {
            VM_FUNC_ENTER0(__closure_0);
            VM_SET("ans", VM_INT(0));
            VM_SET("ans", ((VM_INT(1)+VM_INT(2))-(VM_INT(3)*VM_INT(4))));
            (VM_GET("ans") += VM_INT(5));
            return VM_UNDEF;
        }

        void app_setup(Context *__ctx) {
            VM_CALL(VM_APP_LOC("(top level)", 2), VM_GET("__onReady"), 1,
                    VM_FUNC(__lambda_0, __closure_0));
        }
    `));
});

test("if statement", () => {
    expect(transpile(`\
        const app = require("makestack");
        app.onReady((device) => {
            if (1 == 2)
                device.print("Something went wrong!");

            if (device.location == "earth") {
                device.print("I'm on the earth!");
            } else if (device.name === "moon")
                device.print("I'm on the moon!");
            else
                device.print("Where am I?");
        });
    `)).toStrictEqual(ignoreWhitespace(`
        VM_FUNC_DEF(__lambda_0, __closure_0) {
            VM_FUNC_ENTER1(__closure_0, "device");
            if ((VM_INT(1) == VM_INT(2)))
                VM_CALL(VM_ANON_LOC(4),VM_MGET(VM_GET("device"), VM_STR("print")),
                       1, VM_STR("Something went wrong!"));;

            if ((VM_MGET(VM_GET("device"), VM_STR("location")) == VM_STR("earth"))) {
                VM_CALL(VM_ANON_LOC(7),VM_MGET(VM_GET("device"), VM_STR("print")),
                       1, VM_STR("I'm on the earth!"));
            } else if((VM_MGET(VM_GET("device"), VM_STR("name"))==VM_STR("moon")))
                VM_CALL(VM_ANON_LOC(9), VM_MGET(VM_GET("device"), VM_STR("print")),
                        1, VM_STR("I'm on the moon!"));
            else
                VM_CALL(VM_ANON_LOC(11), VM_MGET(VM_GET("device"), VM_STR("print")),
                        1, VM_STR("WhereamI?"));;

            return VM_UNDEF;
        }

        void app_setup(Context *__ctx) {
            VM_CALL(VM_APP_LOC("(top level)", 2), VM_GET("__onReady"), 1,
                    VM_FUNC(__lambda_0, __closure_0));
        }
    `));
});

test("while statement", () => {
    expect(transpile(`\
        const app = require("makestack");
        app.onReady((device) => {
            while (1) {
                device.print("infinite loop");
            }

            while (1)
                device.print("unreachable!");
        });
    `)).toStrictEqual(ignoreWhitespace(`
        VM_FUNC_DEF(__lambda_0, __closure_0) {
            VM_FUNC_ENTER1(__closure_0, "device");
            while (VM_INT(1)) {
                VM_CALL(
                    VM_ANON_LOC(4),
                    VM_MGET(VM_GET("device"), VM_STR("print")),
                    1,
                    VM_STR("infinite loop")
                );
            };

            while (VM_INT(1))
                VM_CALL(
                    VM_ANON_LOC(8),
                    VM_MGET(VM_GET("device"), VM_STR("print")),
                    1,
                    VM_STR("unreachable!")
                );;

            return VM_UNDEF;
        }

        void app_setup(Context *__ctx) {
          VM_CALL(VM_APP_LOC("(top level)", 2), VM_GET("__onReady"), 1,
                  VM_FUNC(__lambda_0, __closure_0));
        }
    `));
});

test("for statement", () => {
    expect(transpile(`\
        const app = require("makestack");
        app.onReady((device) => {
            for (let i = 0; i < 100; i++) {
                device.print("finite loop");
            }
        });
    `)).toStrictEqual(ignoreWhitespace(`
        VM_FUNC_DEF(__lambda_0,__closure_0) {
            VM_FUNC_ENTER1(__closure_0, "device");
            VM_SET("i", VM_INT(0));
            for (; (VM_GET("i") < VM_INT(100)); VM_GET("i")++) {
                VM_CALL(VM_ANON_LOC(4), VM_MGET(VM_GET("device"), VM_STR("print")),
                        1, VM_STR("finiteloop"));
            };
            return VM_UNDEF;
        }

        void app_setup(Context *__ctx) {
            VM_CALL(VM_APP_LOC("(top level)", 2), VM_GET("__onReady"), 1,
                    VM_FUNC(__lambda_0, __closure_0));
        }
    `));
});
