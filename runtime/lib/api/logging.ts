import { sendToSupervisor } from '../helpers';

export function println(message: string) {
    sendToSupervisor("log", { body: message });
}

export function error(message: string) {
    println(`!${message}`);
}
