import { Request, Response } from "express";
import { Device } from "./device";

export type HTTPCallback = (req: Request, res: Response) => void;
export type HeartbeatCallback = (device: Device) => void;

export const endpoints: { [name: string]: HTTPCallback } = {};
export const callbacks = {
    heartbeat: [] as HeartbeatCallback[],
};

export const http = {
    get(name: string, callback: HTTPCallback): void {
        endpoints[name] = callback;
    },
};

export const device = {
    onHeartbeat(callback: HeartbeatCallback): void {
        callbacks.heartbeat.push(callback);
    },
};
