import * as EventEmitter from "events";
import { execWithPipe } from "../../helpers";
import * as readline from "readline";
const { blue } = require("chalk");

export function viewLog(opts: any) {
    let lastLine: string | undefined;
    const interval = 5;
    let current = interval;

    const readLog = () => {
        const argv = ["firebase", "functions:log", "--project", opts.firebaseProject];
        const { stdout } = execWithPipe(argv);
        lastLine = stdout.split("\n").filter(l => l.length > 0).pop();
        return stdout;
    }

    const printCountdown = () => {
        process.stdout.write(blue.bold(`==> Reloading in ${current} seconds...`));
    }

    const loop = () => {
        readline.clearLine(process.stdout, 0);
        readline.cursorTo(process.stdout, 0);
        current--;
        if (current > 0) {
            printCountdown();
            return;
        }

        let stdout = readLog();

        if (lastLine && stdout.includes(lastLine)) {
            stdout = stdout.split(lastLine, 2)[1];
        }

        if (stdout !== "\n") {
            process.stdout.write(stdout);
        }

        current = interval;
        printCountdown();
        resetTimer();
    };

    let timer: NodeJS.Timer | undefined;
    const resetTimer = () => {
        if (timer) {
            clearInterval(timer);
        }

        timer = setInterval(loop, 1000);
    };

    process.stdout.write(readLog());
    printCountdown();
    resetTimer();
}
