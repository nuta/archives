#include <arch.h>
#include <debug.h>
#include <channel.h>
#include <printk.h>
#include <process.h>
#include <thread.h>

/// The symbol table. tools/link.py embeds it during the build.
extern struct symbol_table __symtable;

/// Resolves a symbol name and the offset from the beginning of symbol.
/// This function returns "(invalid address)" if the symbol does not
/// exist in the symbol table.
const char *find_symbol(vaddr_t vaddr, size_t *offset) {
    ASSERT(__symtable.magic == SYMBOL_TABLE_MAGIC
           && "invalid symbol table magic");

    struct symbol *symbols = __symtable.symbols;
    uint16_t num_symbols = __symtable.num_symbols;
    const char *strbuf = __symtable.strbuf;

    // Do a binary search. Since `num_symbols` is unsigned 16-bit integer, we
    // need larger signed integer to hold -1 here, namely, int32_t.
    int32_t l = -1;
    int32_t r = num_symbols;
    while (r - l > 1) {
        // We don't have to care about integer overflow here because
        // `INT32_MIN < -1 <= l + r < UINT16_MAX * 2 < INT32_MAX` always holds.
        //
        // Read this article if you are not familiar with a famous overflow bug:
        // https://ai.googleblog.com/2006/06/extra-extra-read-all-about-it-nearly.html
        int32_t mid = (l + r) / 2;
        if (vaddr >= symbols[mid].addr) {
            l = mid;
        } else {
            r = mid;
        }
    }

    if (l == -1) {
        *offset = 0;
        return "(invalid address)";
    }

    *offset = vaddr - symbols[l].addr;
    return &strbuf[symbols[l].offset];
}

/// Prints the stack trace.
void backtrace(void) {
    WARN("Backtrace:");
    struct stack_frame *frame = get_stack_frame();
    for (int i = 0; i < BACKTRACE_MAX; i++) {
        if (frame->return_addr < KERNEL_BASE_ADDR) {
            break;
        }

        size_t offset;
        const char *name = find_symbol(frame->return_addr, &offset);
        WARN("    #%d: %p %s()+0x%x", i, frame->return_addr, name, offset);

        if ((uint64_t) frame->next < KERNEL_BASE_ADDR) {
            break;
        }

        frame = frame->next;
    }
}

/// Returns the pointer to the kernel stack's canary. The stack canary exists
/// at the end (bottom) of the stack. The kernel stack should be large enough
/// in order not to modify the canry value. This function assumes that the size
/// of kernel stack equals to PAGE_SIZE.
static inline vaddr_t get_current_stack_canary_address(void) {
    STATIC_ASSERT(PAGE_SIZE == KERNEL_STACK_SIZE);
    return ALIGN_DOWN(arch_get_stack_pointer(), PAGE_SIZE);
}

/// Writes a kernel stack protection marker. The value `STACK_CANARY` is
/// verified by calling check_stack_canary().
void init_stack_canary(vaddr_t stack_bottom) {
    *((uint32_t *) stack_bottom) = STACK_CANARY;
}

/// Verifies that stack canary is alive. If not so, register a complaint.
void check_stack_canary(void) {
    uint32_t *canary = (uint32_t *) get_current_stack_canary_address();
    if (*canary != STACK_CANARY) {
        PANIC("The stack canary is no more! This is an ex-canary!");
    }
}

/// Each CPU have to call this function once during the boot.
void init_boot_stack_canary(void) {
    init_stack_canary(get_current_stack_canary_address());
}

#ifdef DEBUG_BUILD

//
// Undefined Behavior Sanitizer runtime (UBSan).
// https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
//
static void report_ubsan_event(const char *event) {
    PANIC("detected an undefined behavior: %s", event);
}

void __ubsan_handle_type_mismatch_v1(struct ubsan_mismatch_data_v1 *data,
                                     vaddr_t ptr) {
    if (!ptr) {
        report_ubsan_event("NULL pointer dereference");
    } else if (data->align != 0 && (ptr & (data->align - 1)) != 0) {
        report_ubsan_event("misaligned access");
    } else {
        PANIC("pointer %p is not large enough for %s",
              ptr, data->type->name);
    }
}

void __ubsan_handle_add_overflow() {
    report_ubsan_event("add_overflow");
}
void __ubsan_handle_sub_overflow() {
    report_ubsan_event("sub overflow");
}
void __ubsan_handle_mul_overflow() {
    report_ubsan_event("mul overflow");
}
void __ubsan_handle_divrem_overflow() {
    report_ubsan_event("divrem overflow");
}
void __ubsan_handle_negate_overflow() {
    report_ubsan_event("negate overflow");
}
void __ubsan_handle_pointer_overflow() {
    report_ubsan_event("pointer overflow");
}
void __ubsan_handle_out_of_bounds() {
    report_ubsan_event("out of bounds");
}
void __ubsan_handle_shift_out_of_bounds() {
    report_ubsan_event("shift out of bounds");
}
void __ubsan_handle_builtin_unreachable() {
    report_ubsan_event("builtin unreachable");
}

//
//  Kernel Address Santizer (KASan) runtime.
// (https://github.com/google/kasan/wiki)
//
//  The implementation is in the very early stage: redzones, stack overflow
//  detection, shadow propagation, and other features are not yet supported.
//
//  Even though it is incomplete, it's able to detect some memory bugs: NULL
//  pointer dereference, use of an uninitialized memory, etc.
//
// TODO: SMP support: asan_enabled should be a CPU-local variable.
//

static bool asan_enabled = false;

static inline uint8_t *get_asan_shadow_memory(vaddr_t addr) {
    ASSERT(addr >= KERNEL_BASE_ADDR);
    return (uint8_t *) ASAN_SHADOW_MEMORY + into_paddr((void *) addr);
}

static bool set_asan_enabled(bool state) {
    bool prev_state = asan_enabled;
    asan_enabled = state;
    return prev_state;
}

static bool needs_asan_check(vaddr_t addr) {
    return asan_enabled
           && arch_asan_is_kernel_address(addr)
           && addr < (vaddr_t) from_paddr(STRAIGHT_MAP_ADDR);
}

/// Checks whether the memory read is valid.
static void check_load(vaddr_t addr, size_t size) {
    if (!needs_asan_check(addr)) {
        return;
    }

    // Disable ASan temporarily to prevent infinte recursive errors.
    set_asan_enabled(false);

    if (!addr) {
        PANIC("NULL pointer dereference (read) at %p", addr);
    }

    if (addr <= KERNEL_BASE_ADDR) {
        PANIC("invalid memory read access at %p", addr);
    }

    for (size_t i = 0; i < size; i++) {
        uint8_t *shadow = get_asan_shadow_memory(addr + i);
        switch (*shadow) {
        case ASAN_VALID:
            break;
        case ASAN_FREED:
            PANIC("asan: detected a use-after-free bug of %p", addr);
        case ASAN_NOT_ALLOCATED:
            PANIC("asan: detected an use of unallocated memory at %p", addr);
        case ASAN_UNINITIALIZED:
            PANIC("asan: detected an use of uninitialized memory at %p", addr);
        default:
            PANIC("asan: bad shadow memory state for %p (shadow=%p, value=%x)",
                  addr, shadow, *shadow);
        }
    }

    set_asan_enabled(true);
}

/// Checks whether the memory write is valid.
static void check_store(vaddr_t addr, size_t size) {
    if (!needs_asan_check(addr)) {
        return;
    }

    // Disable ASan temporarily to prevent infinte recursive errors.
    set_asan_enabled(false);

    if (!addr) {
        PANIC("NULL pointer dereference (write) at %p", addr);
    }

    if (addr <= KERNEL_BASE_ADDR) {
        PANIC("invalid memory write access at %p", addr);
    }

    for (size_t i = 0; i < size; i++) {
        uint8_t *shadow = get_asan_shadow_memory(addr + i);
        switch (*shadow) {
        case ASAN_VALID:
            break;
        case ASAN_UNINITIALIZED:
            // TODO: Propagate shadow memory.
            *shadow = ASAN_VALID;
            break;
        case ASAN_FREED:
            PANIC("asan: detected a use-after-free bug of %p", addr);
        case ASAN_NOT_ALLOCATED:
            PANIC("asan: detected an use of unallocated memory at %p", addr);
        default:
            PANIC("asan: bad shadow memory state for %p (shadow=%p, value=%x)",
                  addr, shadow, *shadow);
        }
    }

    set_asan_enabled(true);
}

// ASan interfaces used by the compiler.
void __asan_load1_noabort(vaddr_t addr)            { check_load(addr, 1); }
void __asan_load2_noabort(vaddr_t addr)            { check_load(addr, 2); }
void __asan_load4_noabort(vaddr_t addr)            { check_load(addr, 4); }
void __asan_load8_noabort(vaddr_t addr)            { check_load(addr, 8); }
void __asan_loadN_noabort(vaddr_t addr, size_t n)  { check_load(addr, n); }
void __asan_store1_noabort(vaddr_t addr)           { check_store(addr, 1); }
void __asan_store2_noabort(vaddr_t addr)           { check_store(addr, 2); }
void __asan_store4_noabort(vaddr_t addr)           { check_store(addr, 4); }
void __asan_store8_noabort(vaddr_t addr)           { check_store(addr, 8); }
void __asan_storeN_noabort(vaddr_t addr, size_t n) { check_store(addr, n); }
void __asan_handle_no_return(void) { /* TODO: */; }

/// Fills the shadow memory of the given memory area with the tag.
void asan_init_area(enum asan_shadow_tag tag, void *ptr, size_t len) {
    bool asan_state = set_asan_enabled(false);
    inlined_memset(get_asan_shadow_memory((vaddr_t) ptr), tag, len);
    set_asan_enabled(asan_state);
}

/// Detects double-free bugs.
void asan_check_double_free(struct free_list *free_list) {
    uint8_t *shadow = get_asan_shadow_memory((vaddr_t) &free_list->padding);
    if (*shadow == ASAN_FREED) {
        PANIC("asan: detected double-free bug (free_list=%p)", free_list);
    }
}

/// Initializes the ASan runtime.
static void asan_init(void) {
    arch_asan_init();

    // Kmalloc small objects.
    asan_init_area(ASAN_NOT_ALLOCATED, (void *) OBJECT_ARENA_ADDR,
                  OBJECT_ARENA_LEN);
    // Kmalloc page objects.
    asan_init_area(ASAN_NOT_ALLOCATED, (void *) PAGE_ARENA_ADDR,
                  PAGE_ARENA_LEN);

    set_asan_enabled(true);
}

//
//  Kernel Debugger (kdebug).
//
#define DPRINTK(fmt, ...) printk(COLOR_BOLD fmt COLOR_RESET, ##__VA_ARGS__)

static void dump_process(struct process *proc) {
    DPRINTK("Process #%d: %s\n", proc->pid, proc->name);
    LIST_FOR_EACH(thread, &proc->threads, struct thread, next) {
        DPRINTK("  Thread %pT: \n", thread);

        struct channel *send_from = thread->debug.send_from;
        if (send_from) {
            DPRINTK("    Sender at %pC <-> %pC => %pC (header=%p)\n",
                    send_from, send_from->linked_with,
                    send_from->linked_with->transfer_to,
                    thread->ipc_buffer->header);
        }

        struct channel *receive_from = thread->debug.receive_from;
        if (receive_from) {
            DPRINTK("    Receiver at %pC <-> %pC => %pC\n",
                    receive_from, receive_from->linked_with,
                    receive_from->linked_with->transfer_to);
        }
    }
}
static void debugger_run(const char *cmdline) {
    if (strcmp(cmdline, "help") == 0) {
        DPRINTK("Kernel debugger commands:\n");
        DPRINTK("\n");
        DPRINTK("  ps - List processes.\n");
        DPRINTK("  km - Show kernel memory statistics.\n");
        DPRINTK("\n");
    } else if (strcmp(cmdline, "ps") == 0) {
        LIST_FOR_EACH(proc, &process_list, struct process, next) {
            if (proc) {
                dump_process(proc);
            }
        }
    } else if (strcmp(cmdline, "km") == 0) {
        size_t num_used_pages = page_arena.num_objects;
        LIST_FOR_EACH(node, &page_arena.free_list, struct free_list, next) {
            if (node->magic1 != FREE_LIST_MAGIC1
                || node->magic2 != FREE_LIST_MAGIC2) {
                BUG("Corrupted free_list entry %p", node);
            }

            num_used_pages -= node->num_objects;
        }

        size_t num_used_objects = object_arena.num_objects;
        LIST_FOR_EACH(node, &object_arena.free_list, struct free_list, next) {
            if (node->magic1 != FREE_LIST_MAGIC1
                || node->magic2 != FREE_LIST_MAGIC2) {
                BUG("Corrupted free_list entry %p", node);
            }

            num_used_objects -= node->num_objects;
        }

        DPRINTK("Kernel Memory:\n");
        DPRINTK("  %d of %d (%d%%) pages are in use\n",
                num_used_pages, page_arena.num_objects,
                (num_used_pages * 100) / page_arena.num_objects);
        DPRINTK("  %d of %d (%d%%) objects are in use\n",
                num_used_objects, object_arena.num_objects,
                (num_used_objects * 100) / object_arena.num_objects);
    } else {
        WARN("Invalid debugger command: '%s'.", cmdline);
    }
}

void debugger_oninterrupt(void) {
    static char cmdline[128];
    static unsigned long cursor = 0;
    int ch;
    while ((ch = arch_debugger_readchar()) > 0) {
        if (ch == '\r') {
            printk("\n");
            cmdline[cursor] = '\0';
            if (cursor > 0) {
                debugger_run(cmdline);
                cursor = 0;
            }
            DPRINTK("kdebug> ");
            continue;
        }

        DPRINTK("%c", ch);
        cmdline[cursor++] = (char) ch;

        if (cursor > sizeof(cmdline) - 1) {
            WARN("Too long kernel debugger command.");
            cursor = 0;
        }
    }
}
#endif

/// Initializes the debug subsystem.
void debug_init(void) {
#ifdef DEBUG_BUILD
    asan_init();
#endif
}

