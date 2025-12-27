import { describe, expect, it } from "bun:test";
import { boot } from "./helpers/emulator";

describe('Hello World', () => {
    it('prints a boot message', async () => {
        using vm = await boot();
        await vm.waitForLogs(async (logs) => {
            expect(logs).toContainEqual({
                name: 'kernel',
                level: 'info',
                message: 'Booting FTL...'
            });
        });
    });
});
