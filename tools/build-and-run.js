const child_process = require("child_process");
const fs = require("fs");
const path = require("path");
const chalk = require("chalk");

let exts = [
    ".c", ".cc", ".cpp", ".h", ".mk"
];

console.info(chalk.green("*** started watching the source files"));

fs.watch(".", { recursive: true }, (type, filepath) => {
    if (exts.includes(path.extname(filepath))) {
        console.info(chalk.cyan(`*** ${type} ${filepath}`));
        
        try {
            let timeout = 10 * 1000; /* kill QEMU after 5 seconds */
            child_process.execSync("make -j4 run", {stdio: [0,1,2], timeout});
        } catch (err) {
            console.error(chalk.red("make returned non-zero exit status"));
        }

        console.info(chalk.green("*** watching changes..."));
    }
});