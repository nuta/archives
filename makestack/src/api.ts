
import { Request, Response } from "express";

export type HTTPCallback = (req: Request, res: Response) => void;

(global as any).__httpEndpoints = {
    get: [],
    post: [],
    delete: [],
    put: [],
}

export function get(name: string, callback: HTTPCallback) {
    (global as any).__httpEndpoints["get"].push({ name, callback });
}

export function post(name: string, callback: HTTPCallback) {
    (global as any).__httpEndpoints["post"].push({ name, callback });
}

export function delete_(name: string, callback: HTTPCallback) {
    (global as any).__httpEndpoints["delete"].push({ name, callback });
}

export function put(name: string, callback: HTTPCallback) {
    (global as any).__httpEndpoints["put"].push({ name, callback });
}

(global as any).__eventEndpoints = {}
export function onEvent(name: string, callback: (value: any) => void) {
    (global as any).__eventEndpoints[name] = callback;
}

export interface DeviceAPI {
    print: (msg: string) => void;
    publish: (eventName: string, value: boolean | number | string) => void;
    delay: (milliseconds: number) => void;
    delaySeconds: (seconds: number) => void;
    delayMinutes: (minutes: number) => void;
    pinMode: (pin: number, mode: "OUTPUT" /* TODO: add more modes */) => void;
    digitalWrite: (pin: number, level: boolean) => void;
    digitalRead: (pin: number, level: boolean) => boolean;
    analogRead: (pin: number) => number;
}

//
// Device API dummy functions for typing. The actual API implementation is located
// in `firmware`.
//
export function onReady(callback: (device: DeviceAPI) => void): void {
    /* We're in the server context. Do nothing. */
}
