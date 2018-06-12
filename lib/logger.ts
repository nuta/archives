const { green, blue, red, cyanBright, magenta, bold } = require("chalk");

function join(messages: any[]): string {
    return messages.map(m => m.toString()).join(" ");
}

export type LoggerSubject = "server" | "device";
export class Logger {
    private subject?: LoggerSubject;

    constructor(subject?: LoggerSubject) {
        this.subject = subject;
    }

    private log(str: string) {
        if (this.subject) {
            const colors = {
                server: blue,
                device: magenta
            }

            const prefix = colors[this.subject](this.subject.padStart(8) + ":");
            console.log(prefix + " " + str);
        } else {
            console.log(str);
        }
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
};

export const logger = new Logger();
