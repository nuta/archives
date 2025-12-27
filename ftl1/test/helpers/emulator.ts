import { EventEmitter } from "node:events";
import * as net from "node:net";
import { AddressInfo } from "node:net";
import { startQemu, type Port } from "../../libs/typescript/cli/src/qemu";

function debounce(cb: () => void) {
    let timer: NodeJS.Timeout | null = null;
    return () => {
        if (timer) {
            clearTimeout(timer);
        }

        timer = setTimeout(cb, 5);
    };
}

export interface Log {
    name: string;
    level: 'error' | 'warn' | 'info' | 'debug' | 'trace';
    message: string;
}

async function streamToLines(stream: ReadableStream<Uint8Array>, cb: (line: string) => void) {
    const decoder = new TextDecoder();
    const reader = stream.getReader();

    let buf = "";
    while (true) {
        const { done, value } = await reader.read();
        if (done) {
            return;
        }

        buf += decoder.decode(value, { stream: true });
        const lines = buf.split(/\n|\r\n?/);
        buf = lines.pop() ?? "";
        for (const line of lines) {
            cb(line);
        }

    }
}

// Example: [tcpip       ] INFO   tcpip: listening on 0.0.0.0:80
const LOG_REGEX = /^\[(?<name>[a-zA-Z0-9_-]+)\s*\].*(?<level>ERROR|WARN|INFO|DEBUG|TRACE)\s*\u001b\[0m\s+(?<message>.+)$/;

function parseLog(line: string): Log | string {
    const match = line.match(LOG_REGEX);
    if (!match) {
        // Malformed log lines. Keep them as strings.
        return line;
    }

    return {
        name: match.groups?.name as string,
        level: match.groups?.level.toLowerCase() as Log['level'],
        message: match.groups?.message as string,
    };
}

export interface Params {
    arch?: 'arm64' | 'riscv64';
    ports?: Port[];
}

export interface Vm {
    waitForLogs(cb: (logs: (Log | string)[]) => Promise<void>): Promise<void>;
    [Symbol.dispose](): void;
}

export async function boot(params?: Params): Promise<Vm> {
    const child = await startQemu({
        arch: params?.arch ?? 'arm64',
        stdio: 'pipe',
        ports: params?.ports,
    });
    const emitter = new EventEmitter();

    const onNewLog = debounce(() => {
        emitter.emit('log');
    });;

    if (!(child.stdout instanceof ReadableStream)) {
        throw new Error(`stdout is not a readable stream: ${typeof child.stdout}`);
    }

    if (!(child.stderr instanceof ReadableStream)) {
        throw new Error(`stderr is not a readable stream: ${typeof child.stderr}`);
    }

    const logs: (Log | string)[] = [];
    streamToLines(child.stdout, (line) => {
        logs.push(parseLog(line));
        onNewLog();
    }).catch((error) => {
        console.error(`stdout stream error: ${error}`);
    });

    streamToLines(child.stderr, (line) => {
        console.warn(`[stderr] ${line}`);
    }).catch((error) => {
        console.error(`stderr stream error: ${error}`);
    });

    return {
        async waitForLogs(cb: (logs: (Log | string)[]) => Promise<void>) {
            return new Promise<void>((resolve, reject) => {
                setTimeout(async () => {
                    for (const log of logs) {
                        if (typeof log === 'string') {
                            console.error(`[stdout] ${log}`);
                        } else {
                            console.error(`[${log.name}] ${log.level.toUpperCase()} ${log.message}`);
                        }
                    }
                    reject(new Error(`waitForLogs timeout: ${logs.length} logs received`));
                }, 500);

                emitter.on('log', async () => {
                    try {
                        await cb(logs);
                        resolve();
                    } catch (error) {
                        // Assertion failures. Try again later.
                    }
                });
            });
        },
        [Symbol.dispose]() {
            child.kill();
        }
    }
}

export async function getRandomPort(): Promise<number> {
    const socket = new net.Server();
    return new Promise((resolve, reject) => {
        socket.on('error', reject);
        socket.on('listening', () => {
            const port = (socket.address() as AddressInfo).port;
            socket.close();
            resolve(port);
        });

        socket.listen({ port: 0, host: '127.0.0.1' });
    });
}
