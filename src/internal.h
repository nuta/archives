#ifndef __ENA_INTERNAL_H__
#define __ENA_INTERNAL_H__

#include "api.h"
#include "hash.h"
#include "lexer.h"
#include "parser.h"

#ifdef _MSC_VER
#define UNUSED
#define UNREACHABLE
#else
#define UNUSED __attribute__((unused))
#define UNREACHABLE  __builtin_unreachable()
#endif

/// TODO: implement by ourselves
#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include <string.h>
#define ena_memchr memchr
#define ena_snprintf snprintf
#define ENA_ASSERT assert
#define ena_setjmp setjmp
#define ena_longjmp longjmp
#define ena_jmpbuf jmp_buf

#ifdef ENA_DEBUG_BUILD
// Including non-freestanding libraries are allowed in debug build.
#include <stdio.h>
#define DEBUG(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)
#define DEBUG_NONL(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#define DEBUG_NONL(fmt, ...)
#endif

#define ENA_PANIC(error_type, fmt, ...) \
    do {\
        vm->error.type = error_type; \
        ena_snprintf( \
            (char *) &vm->error.str, sizeof(vm->error.str), \
            fmt, ## __VA_ARGS__ \
        ); \
        ena_longjmp(vm->panic_jmpbuf, 1); \
    } while (0)
#define BUG(fmt, ...) \
    ENA_PANIC(ENA_ERROR_BUG, "BUG: " fmt, ## __VA_ARGS__)
#define TYPE_ERROR(fmt, ...) \
    ENA_PANIC(ENA_ERROR_TYPE, "Type Error: " fmt, ## __VA_ARGS__)
#define RUNTIME_ERROR(fmt, ...) \
    ENA_PANIC(ENA_ERROR_RUNTIME, "Runtime Error: " fmt, ## __VA_ARGS__)
#define SYNTAX_ERROR(fmt, ...) \
    ENA_PANIC(ENA_ERROR_INVALID_SYNTAX, "Syntax Error: " fmt, ## __VA_ARGS__)

#define NOT_YET_IMPLEMENTED() \
    ENA_PANIC( \
        ENA_ERROR_NOT_YET_IMPLEMENTED, \
        "%s:%s():%d: not yet implemented", \
        __FILE__, __func__, __LINE__ \
    )

#define PUSH_SAVEPOINT() \
    do { \
        struct ena_savepoint *sp = ena_malloc(sizeof(*sp)); \
        sp->prev = vm->current_savepoint; \
        vm->current_savepoint = sp; \
    } while(0)
#define EXEC_SAVEPOINT() ena_setjmp(vm->current_savepoint->jmpbuf)
#define UNWIND_SAVEPOINT(unwind_type) ena_longjmp(vm->current_savepoint->jmpbuf, unwind_type); UNREACHABLE
#define POP_SAVEPOINT() \
    do { \
        struct ena_savepoint *current_sp = vm->current_savepoint; \
        if (!current_sp) { \
            BUG("pop savepoint in the top level"); \
        } \
        vm->current_savepoint = current_sp->prev; \
        ena_free(current_sp); \
    } while(0)

struct ena_error {
    enum ena_error_type type;
    char str[256];
    int line;
    int column;
};

struct ena_vm {
    struct ena_error error;
    ena_jmpbuf panic_jmpbuf;
    size_t next_ident;
    struct ena_lexer lexer;
    struct ena_hash_table ident2cstr;
    struct ena_hash_table cstr2ident;
    struct ena_module *main_module;
    struct ena_scope *current_scope;
    struct ena_savepoint *current_savepoint;
    struct ena_class *current_class;
    ena_value_t self;
    struct ena_ast *ast_list;
    struct ena_class *string_class;
};

typedef uintptr_t ena_ident_t;

static inline int ena_isdigit(int c) {
    return '0' <= c && c <= '9';
}

static inline int ena_isalpha(int c) {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

static inline int ena_isupper(int c) {
    return 'A' <= c && c <= 'Z';
}

static inline int ena_isalnum(int c) {
    return ena_isdigit(c) || ena_isalpha(c);
}

void *ena_memcpy(void *dst, const void *src, size_t len);
int ena_memcmp(void *ptr1, const void *ptr2, size_t len);
int ena_strcmp(const char *s1, const char *s2);
int ena_strncmp(const char *s1, const char *s2, size_t n);
size_t ena_strlen(const char *str);
int ena_str2int(const char *str);
char *ena_strdup(const char *str);
char *ena_strndup(const char *str, size_t len);

ena_ident_t ena_cstr2ident(struct ena_vm *vm, const char *str);
const char *ena_ident2cstr(struct ena_vm *vm, ena_ident_t ident);

// TODO: Move these functions to an appropriate file.
struct ena_module *ena_create_module(void);
struct ena_int *ena_cast_to_int(ena_value_t value);
struct ena_string *ena_cast_to_string(ena_value_t value);
ena_value_t get_var_value(struct ena_scope *scope, ena_ident_t name);
struct ena_hash_entry *lookup_var(struct ena_scope *scope, ena_ident_t name);
void ena_assign_to_var(struct ena_vm *vm, struct ena_hash_table *table, ena_ident_t name, ena_value_t value, bool allow_undefined);
void ena_define_var(struct ena_vm *vm, ena_ident_t name, ena_value_t value);
void ena_define_var_in(struct ena_vm *vm, struct ena_scope *scope, ena_ident_t name, ena_value_t value);
void ena_define_var(struct ena_vm *vm, ena_ident_t name, ena_value_t value);
ena_value_t get_var_from(struct ena_hash_table *table, ena_ident_t name);
struct ena_class *ena_create_class(void);
void ena_check_args(struct ena_vm *vm, const char *name, const char *rule, ena_value_t *args, int num_args);

#endif
