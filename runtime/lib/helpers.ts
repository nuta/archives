import { logger } from './logger';

export function sendToSupervisor(type: string, meta: { [key: string]: string }) {
    if (!process.send) {
        logger.error('process.send it not defined')
        return
    }

    process.send(Object.assign({ type }, meta))
}
