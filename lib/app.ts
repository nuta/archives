export class App {
    module: any;

    constructor(appPath: string) {
        this.module = require(appPath);
    }
}
