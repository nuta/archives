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
        this.stdout = false;
        this.buffer = "";
    }

    private _log(str: string) {
        if (this.stdout) {
            console.log(str);
        } else {
            this.buffer += str;
        }
    }

    private log(str: string) {
        if (this.subject) {
            const colors = {
                server: blue,
                device: magenta,
            };

            const prefix = colors[this.subject](this.subject.padStart(8) + ":");
            this._log(prefix + " " + str);
        } else {
            this._log(str);
        }
    }

    public enableStdout() {
        this.stdout = true;
        const buffer = this.buffer;
        this.buffer = "";
        return buffer;
    }

    public disableStdout() {
        this.stdout = false;
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
        this.log(red.bold(join(messages)));
    }

    public command(...messages: any[]) {
        this.log(bold("==> " + join(messages)));
    }

    public action(action: string, target: string) {
        this.log(blue.bold(action.padStart(8)) + "  " + target);
    }

    public progress(...messages: any[]) {
        this.log(bold(green("==> ") + join(messages)));
    }
}

export const logger = new Logger();
