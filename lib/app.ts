export class App {
    public module: any;

    constructor(appPath: string) {
        this.module = require(appPath);
    }
}
