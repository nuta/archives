const { green, blue, red, cyanBright, magenta, bold } = require("chalk");

function join(messages: any[]): string {
    return messages.map((m) => m.toString()).join(" ");
}

export class Logger {

    private _log(str: string, forceStdout: boolean) {
        console.log(str);
    }

    private log(str: string, forceStdout: boolean = false) {
        this._log(str, forceStdout);
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
        this.log(red.bold(join(messages)), true);
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
