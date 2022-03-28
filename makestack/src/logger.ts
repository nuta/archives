const { green, blue, red, cyanBright, magenta, bold } = require("chalk");

function join(messages: any[]): string {
    return messages.map((m) => m.toString()).join(" ");
}

export type LoggerSubject = "server" | "device";
export class Logger {
    private subject?: LoggerSubject;
    private stdout: boolean;
    private buffer: string;

    constructor(subject?: LoggerSubject) {
        this.subject = subject;
        this.stdout = true;
        this.buffer = "";
    }

    private _log(str: string, forceStdout: boolean) {
        if (forceStdout || this.stdout) {
            console.log(str);
        } else {
            this.buffer += str;
        }
    }

    private log(str: string, forceStdout: boolean = false) {
        if (this.subject) {
            const colors = {
                server: blue,
                device: magenta,
            };

            const prefix = colors[this.subject](this.subject.padStart(8) + ":");
            this._log(prefix + " " + str, forceStdout);
        } else {
            this._log(str, forceStdout);
        }
    }

    public enableStdout() {
        this.stdout = true;
        const buffer = this.buffer;
        this.buffer = "";
        return buffer;
    }

    public disableStdout() {
//        this.stdout = false;
    }

    public debug(...messages: any[]) {
        this.log(join(messages));
    }

    public info(...messages: any[]) {
        this.log(cyanBright(join(messages)));
    }

    public warn(...messages: any[]) {
        this.log(red.yellow(join(messages)));
    }

    public error(...messages: any[]) {
        const msg = join(messages);
        const prefix = (msg.startsWith("Error:")) ? "" : "Error: ";
        this.log(red.bold(prefix + msg), true);
        process.exit(1);
    }

    public command(...messages: any[]) {
        const maxLen = process.stdout.columns || 80;
        const str = join(messages);
        let truncated;
        if (str.length > maxLen) {
            truncated = str.slice(0, maxLen - 3) + "...";
        } else {
            truncated = str;
        }
        this.log(bold(truncated));
    }

    public success(...messages: any[]) {
        this.log(bold(green("==> ") + join(messages)));
    }

    public failure(...messages: any[]) {
        this.log(bold(red("==> ") + join(messages)));
    }

    public progress(...messages: any[]) {
        this.log(bold(blue("==> ") + join(messages)));
    }

    public action(action: string, target: string) {
        this.log(magenta.bold(action.padStart(8)) + "  " + target);
    }
}

export const logger = new Logger();
