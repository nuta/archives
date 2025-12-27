import fs from 'node:fs/promises';
import path from 'node:path';
import { resolveRepoPath } from './utils';

interface QemuArch {
    bin: string;
    flags(): string[];
}

const QEMU_COMMON_FLAGS = [
    '-kernel', resolveRepoPath('build/ftl.elf'),
    '-nographic', '-serial', 'mon:stdio', '--no-reboot',
    '-global', 'virtio-mmio.force-legacy=false',
    '-device', 'virtio-net-device,netdev=net0,bus=virtio-mmio-bus.0',
];

const QEMU_ARCHS: Record<Params['arch'], QemuArch> = {
    arm64: {
        bin: 'qemu-system-aarch64',
        flags() {
            return [
                '-cpu', 'host', '-accel', 'hvf',
                '-machine', 'virt,gic-version=2',
            ];
        }
    },
    riscv64: {
        bin: 'qemu-system-riscv64',
        flags() {
            return [
                '-machine', 'virt',
                '-bios', 'default',
            ];
        }
    }
}

export interface Port {
    protocol: 'tcp';
    guestPort: number;
    hostPort: number;
}

export interface Params {
    arch: string
    debugMode?: boolean
    ports?: Port[];
    stdio: 'inherit' | 'pipe';
}

export async function createDiskImage(imagePath: string): Promise<void> {
    const size = 100 * 1024 * 1024;

    const handle = await fs.open(imagePath, 'w');
    await handle.truncate(size);
    await handle.close();
}

export async function startQemu(params: Params): Promise<ReturnType<typeof Bun.spawn>> {
    const arch = QEMU_ARCHS[params.arch];
    if (!arch) {
        throw new Error(`unknown arch for QEMU: ${params.arch}`)
    }

    const argv = [arch.bin, ...QEMU_COMMON_FLAGS, ...arch.flags()]
    if (params.debugMode) {
        argv.push('-object', 'filter-dump,id=filter0,netdev=net0,file=network.pcap')
        argv.push('-d', 'cpu_reset,unimp,guest_errors,int')
        argv.push('-D', 'qemu.log');
        argv.push('-gdb', 'tcp::7778');

    }

    const hostfwds = [];
    if (params?.ports) {
        for (const port of params.ports) {
            hostfwds.push(`${port.protocol}:127.0.0.1:${port.hostPort}-:${port.guestPort}`);
        }
    }

    if (hostfwds.length > 0) {
        argv.push('-netdev', `user,id=net0,hostfwd=${hostfwds.join(',')}`);
    } else {
        argv.push('-netdev', 'user,id=net0');
    }

    const imagePath = resolveRepoPath('disk.img');
    await createDiskImage(imagePath);

    return Bun.spawn(argv, {
        env: {
            ...process.env,
        },
        stdio: [
            null,
            params.stdio,
            params.stdio,
        ],
    });
}
