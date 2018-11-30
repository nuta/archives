#include <kernel/types.h>
#include <kernel/process.h>
#include <kernel/thread.h>
#include <kernel/kfs.h>
#include <kernel/server.h>
#include <kernel/printk.h>
#include <kernel/string.h>
#include <resea/discovery.h>
#include <resea/kbd_device.h>

static struct channel *server;
static struct channel *stdin;
static struct process *current;
static int exitcode;

void dos_syscall_handler(void);

static char keycode_to_ascii(int keycode) {
    switch (keycode) {
        case 0x8008: return '\n';
        case 0x801d: return ' ';
        default: return keycode;
    }
}

static void exec_file(const char *path, const char *param) {
    struct process *proc = process_create();
    process_set_syscall_handler(proc, dos_syscall_handler);

    struct kfs_dir dir;
    struct kfs_file file;
    kfs_opendir(&dir);
    while (kfs_readdir(&dir, &file) != NULL) {
        if (!strcmp(path, file.name)) {
            current = proc;
            elf_launch_process(proc, file.data, file.length, kfs_pager, file.pager_arg);
            return;
        }
    }

    WARN("dos: failed to execute '%s'", path);
}

static void keyboard_input_handler(u64_t keycode) {
    ipc_notify(stdin->cid, 0, keycode);
}

static void mainloop(void) {
    channel_t from;
    payload_t a0, a1, a2, a3;
    payload_t r0 = 0, r1 = 0, r2 = 0, r3 = 0;
    header_t header = ipc_recv(server->cid, &from, &a0, &a1, &a2, &a3);
    for (;;) {
        error_t error = ERROR_DONT_REPLY;
        switch (MSGTYPE(header)) {
            case KBD_DEVICE_KEYDOWN_MSG:
                keyboard_input_handler((u64_t) a0);
                error = ERROR_DONT_REPLY;
                break;
        }

        if (error == ERROR_DONT_REPLY) {
            header = ipc_recv(server->cid, &from, &a0, &a1, &a2, &a3);
        } else {
            header = ipc_replyrecv(&from, header, r0, r1, r2, r3, &a0, &a1, &a2, &a3);
        }
    }
}

static void dos_main(void) {
    DEBUG("dos: initializing dos layer...");
    stdin = channel_create(kernel_process);
    server = channel_create(kernel_process);
    channel_t kernel_server = channel_connect(kernel_channel, kernel_process);

    DEBUG("dos: connecting to kbd_device");
    channel_t kbd_device;
    header_t err;
    err = call_discovery_discover(kernel_server, KBD_DEVICE_SERVICE, &kbd_device);
    if (ERRTYPE(err) != ERROR_NONE) {
        PANIC("discovery.discover returned an error: %d", err);
    }

    struct channel *kbd_device_listener = channel_create(kernel_process);
    channel_transfer(kbd_device_listener, server);
    err = call_kbd_device_listen(kbd_device, kbd_device_listener->cid);
    if (ERRTYPE(err) != ERROR_NONE) {
        PANIC("kbd_device.listen returned an error: %d", err);
    }

    printk("Resea DOS version " VERSION "\n");
    printk("Launching /bin/command\n");
    exec_file("/bin/command", "");
    mainloop();
}

void dos_syscall(u64_t *rax, u64_t *rbx, u64_t *rcx, u64_t *rdx) {
    // Note that we're in the user process context, namely
    // CPUVAR->current->process == current.
#ifdef DOS_DEBUG
    DEBUG("syscall: rax=%p, rbx=%p, rcx=%p, rdx=%p", *rax, *rbx, *rcx, *rdx);
#endif
    u8_t ah = *rax >> 8;
    u8_t al = *rax & 0xff;
    u8_t dl = *rdx & 0xff;

    switch (ah) {
        // Read from stdin: returns AL=character
        case 0x01: {
            struct msg *m = channel_recv(stdin);
            *rax = keycode_to_ascii(m->payloads[0]);
            break;
        }
        // Write to stdout: AL=character
        case 0x02:
            printk("%c", dl);
            break;
        // Terminate the program: AL=exit code
        case 0x4c:
            exitcode = al;
            process_destroy(current);
            exec_file("/bin/command", "");
            break;
        // Execute a program: RBX=cmd, RCX=cmd_len/param_len RDX=param
        case 0xf0: {
            char cmd[32];
            char param[64];
            int cmd_len = *rcx & 0xffff;
            int param_len = (*rcx >> 16) & 0xffff;

            if (cmd_len > sizeof(cmd) - 1 || param_len > sizeof(param) - 1) {
                WARN("too long command or param");
                break;
            }

            // XXX: verify user pointers because a page fault will destroys
            // the current dos kernel thread.
            arch_copy_from_user(&cmd, *rdx, cmd_len);
            arch_copy_from_user(&param, *rbx, param_len);
            cmd[cmd_len] = '\0';
            param[param_len] = '\0';
            INFO("cmd='%s', len=%d", cmd, cmd_len);

            // to lower
            char cmd_path[48];
            int j;
            memcpy(&cmd_path, "/bin/", 5);
            for (j=0; j < cmd_len; j++) {
                cmd_path[j + 5] = cmd[j] + 0x20;
            }
            cmd_path[j + 5] = '\0';

            exec_file(cmd_path, param);
            break;
        }
    }
}

void layer_init(void) {
    thread_resume(thread_create(kernel_process, (uptr_t) dos_main, 0));
}
