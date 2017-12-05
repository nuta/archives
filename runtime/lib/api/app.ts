export class AppAPI {
    public enableUpdate() {
        process.send({ type: "setUpdateLock", body: 'unlock' });
    }

    public disableUpdate() {
        process.send({ type: "setUpdateLock", body: 'lock' });
    }

    public onExit(callback) {
        process.on("SIGTERM", () => {
            callback();
        });
    }
}
