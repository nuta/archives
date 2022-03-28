const app = require("makestack");

app.onReady((device) => {
    device.print("lang-test");

    // String concatenation.
    device.print("print 'abcd' = '" +  "abcd" + "'");

    // Do math.
    let x;
    x = 3 + 20 / 2 * 5; // 53
    device.print("53 = " + x);
    x += 2;  // 55
    device.print("55 = " + x);
    x /= 5;  // 11
    device.print("11 = " + x);
    x *= 2;  // 22
    device.print("22 = " + x);
    x -= 12; // 10
    device.print("10 = " + x);
});
