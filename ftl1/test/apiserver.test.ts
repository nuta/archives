import { describe, expect, it } from "bun:test";
import { boot, getRandomPort, Params, Vm } from "./helpers/emulator";

async function bootWithApiserver(params?: Params): Promise<{ vm: Vm, url: string }> {
    const hostPort = await getRandomPort();
    const vm = await boot({
        ...params,
        ports: [
            ...(params?.ports ?? []),
            {
                protocol: 'tcp',
                guestPort: 80,
                hostPort,
            },
        ],
    });

    await vm.waitForLogs(async (logs) => {
        expect(logs).toContainEqual({
            name: 'apiserver',
            level: 'info',
            message: expect.stringContaining('listening on TCP'),
        });
    });

    const url = `http://127.0.0.1:${hostPort}`;
    return { vm, url };
}

describe('API server', () => {
    it('works', async () => {
        const { vm, url } = await bootWithApiserver();

        // Send a request to the apiserver.
        const resp = await fetch(`${url}/`, {
            signal: AbortSignal.timeout(200),
        });
        expect(resp.status).toBe(200);
        const body = await resp.text();
        expect(resp.headers).toContainEqual(['x-powered-by', 'ftl']);
        expect(body).toContain('<!DOCTYPE html>');
    });

    it('handles backpressure', async () => {
        const { vm, url } = await bootWithApiserver();

        const largeBody = 'x'.repeat(32 * 1024);
        const resp = await fetch(`${url}/echo`, {
            method: 'POST',
            body: largeBody,
            signal: AbortSignal.timeout(1000),
        });

        await vm.waitForLogs(async (logs) => {
            expect(logs).toContainEqual(expect.objectContaining({
                name: 'tcpip',
                message: expect.stringContaining('TCP write buffer is full'),
            }));
        });

        expect(resp.status).toBe(200);
        const responseBody = await resp.text();
        expect(responseBody).toBe(largeBody);
    });
});
