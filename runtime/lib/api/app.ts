export class AppAPI {
  public enableUpdate() {
    process.send({ type: "setUpdateEnabled", body: true });
  }

  public disableUpdate() {
    process.send({ type: "setUpdateEnabled", body: false });
  }

  public onExit(callback) {
    process.on("SIGTERM", () => {
      callback();
    });
  }
}
