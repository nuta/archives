import { sendToSupervisor } from '../helpers';

export function print(message: string) {
    sendToSupervisor("log", { body: message });
}

export function eprint(message: string) {
    print(`!${message}`);
}
