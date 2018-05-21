import { sendToSupervisor } from '../helpers';

export function publish(event: string, data?: string | number) {
    sendToSupervisor("log", { body: `@${event} ${data}` });
}
