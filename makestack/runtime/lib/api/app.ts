import { sendToSupervisor } from '../helpers';

export class AppAPI {
    public enableUpdate() {
        sendToSupervisor("setUpdateLock", { body: 'unlock' });
    }

    public disableUpdate() {
        sendToSupervisor("setUpdateLock", { body: 'lock' });
    }

    public onExit(callback: () => void) {
        process.on("SIGTERM", () => {
            callback();
        });
    }
}
