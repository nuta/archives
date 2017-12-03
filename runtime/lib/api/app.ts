export class AppAPI {
  public enableUpdate() {
    process.send({ type: "setUpdateLock", body: 'lock' });
  }

  public disableUpdate() {
    process.send({ type: "setUpdateLock", body: 'unlock' });
  }

    public onExit(callback) {
        process.on("SIGTERM", () => {
            callback();
        });
    }
}
