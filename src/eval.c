#include "api.h"
#include "parser.h"
#include "eval.h"
#include "internal.h"
#include "malloc.h"

#define EVAL_NODE(name) static ena_value_t eval_##name(UNUSED struct ena_vm *vm, UNUSED struct ena_node *node)
#define EVAL_BINOP_NODE(op_name, result_type, c_op) \
    EVAL_NODE(op_name) { \
        struct ena_int *lhs = ena_cast_to_int(eval_node(vm, &node->child[0])); \
        struct ena_int *rhs = ena_cast_to_int(eval_node(vm, &node->child[1])); \
        return ena_create_##result_type(lhs->value c_op rhs->value); \
    }

static ena_value_t eval_node(struct ena_vm *vm, struct ena_node *node);

void init_scope(struct ena_scope *scope, struct ena_scope *parent) {
    scope->flags = SCOPE_FLAG_LOCALS;
    scope->parent = parent;
    scope->refcount = 1;
    ena_hash_init_ident_table(&scope->vars);
}

static struct ena_scope *create_scope(struct ena_scope *parent) {
    // TODO: Allocate a scope by alloca().
    struct ena_scope *scope = ena_malloc(sizeof(*scope));
    init_scope(scope, parent);
    return scope;
}

EVAL_BINOP_NODE(OP_ADD, int, +)
EVAL_BINOP_NODE(OP_SUB, int, -)
EVAL_BINOP_NODE(OP_LT,  bool, <)
EVAL_BINOP_NODE(OP_EQ,  bool, ==)

EVAL_NODE(STMTS) {
    for (int i = 0; i < node->num_childs; i++) {
        eval_node(vm, &node->child[i]);
    }

    return ENA_UNDEFINED;
}

EVAL_NODE(PROGRAM) {
    return eval_node(vm, node->child);
}

EVAL_NODE(VAR) {
    ena_value_t initial = ENA_UNDEFINED;
    ena_ident_t name = ena_cstr2ident(vm, node->token->str);
    if (node->num_childs == 1) {
        // Variable declaration with initializer (e.g. var x = expr).
        // Evaluate it.
        initial = eval_node(vm, node->child);
    }

    ena_define_var(vm, name, initial);
    return ENA_UNDEFINED;
}

EVAL_NODE(OP_ASSIGN) {
    ena_ident_t name = ena_cstr2ident(vm, node->child[0].token->str);
    ena_value_t rvalue = eval_node(vm, &node->child[1]);

    if (node->child[0].type == ENA_NODE_PROP) {
        struct ena_node *obj = node->child[0].child;
        if (!ena_strcmp(obj->token->str, "self")) {
            if (!vm->current_instance) {
                RUNTIME_ERROR("self is not available");
            }

            struct ena_hash_table *table;
            struct ena_hash_entry *entry;
            table = &vm->current_instance->props;
            entry = ena_hash_search_or_insert(table, (void *) name, (void *) rvalue);
            if (entry) {
                // Already defined in self. Replace its value intead of
                // inserting a new variable.
                entry->value = (void *) rvalue;
            }
        } else {
            NOT_YET_IMPLEMENTED();
        }
    } else {
        struct ena_hash_entry *entry = lookup_var(vm->current_scope, name);
        if (!entry) {
            RUNTIME_ERROR("%s is not defined", ena_ident2cstr(vm, name));
        }

        entry->value = (void *) rvalue;
    }

    return rvalue;
}

EVAL_NODE(FUNC) {
    ena_ident_t name = ena_cstr2ident(vm, node->token->str);
    struct ena_node *param_names = &node->child[0];
    struct ena_node *stmts = &node->child[1];
    struct ena_func *func = (struct ena_func *) ena_malloc(sizeof(*func));
    func->header.type = ENA_T_FUNC;
    func->header.refcount = 1;
    func->name = name;
    func->param_names = param_names;
    func->stmts = stmts;
    func->scope = vm->current_scope;

    uint32_t type;
    struct ena_hash_table *table;
    if (vm->current_class) {
        // A method definition.
        table = &vm->current_class->methods;
        type = FUNC_FLAGS_METHOD;
    } else {
        // A function definition.
        table = &vm->current_scope->vars;
        type = FUNC_FLAGS_FUNC;
    }

    DEBUG(">>> func def %s", ena_ident2cstr(vm, name));
    ena_hash_dump_ident_value_table(vm, table);

    func->flags = FUNC_FLAGS_METHOD;
    if (ena_hash_search_or_insert(table, (void *) name, (void *) func)) {
        RUNTIME_ERROR("%s is already defined", ena_ident2cstr(vm, name));
    }

    return ENA_UNDEFINED;
}

static ena_value_t eval_func_call(
    struct ena_vm *vm,
    struct ena_node *expr_list,
    struct ena_instance *instance,
    struct ena_func *func
) {
    int num_params = func->param_names->num_childs;
    int num_args = expr_list->num_childs;
    if (num_args != num_params) {
        RUNTIME_ERROR("incorrect # of arguments");
        return ENA_UNDEFINED;
    }

    // Evaluate and fill arguments into the newly created scope.
    struct ena_scope *caller_scope = vm->current_scope;
    struct ena_node *args = expr_list->child;
    struct ena_scope *new_scope = create_scope(func->scope);
    for (int i = 0; i < num_args; i++) {
        ena_ident_t param_name = ena_cstr2ident(vm, func->param_names->child[i].token->str);
        ena_value_t arg_value = eval_node(vm, &args[i]);
        ena_define_var_in(vm, new_scope, param_name, arg_value);
    }

    // Enther the scope and execute the body.
    PUSH_SAVEPOINT();
    int unwind_type;
    ena_value_t ret_value = ENA_UNDEFINED;
    if ((unwind_type = EXEC_SAVEPOINT()) == 0) {
    DEBUG(">>> call %s", ena_ident2cstr(vm, func->name));
    ena_hash_dump_ident_value_table(vm, &new_scope->parent->vars);

        vm->current_scope = new_scope;
        vm->current_instance = instance;
        eval_node(vm, func->stmts);
    } else {
        // UNWIND_SAVEPOINT() is invoked in the callee.
        switch (unwind_type) {
            case ENA_UNWIND_RETURN:
                ret_value = vm->current_savepoint->ret_value;
                break;
        }
    }

    // Returned from the function.
    vm->current_scope = caller_scope;
    vm->current_instance = NULL;
    POP_SAVEPOINT();
    return ret_value;
}

static struct ena_func *lookup_method(struct ena_class *cls, ena_ident_t name) {
    struct ena_hash_entry *e = ena_hash_search(&cls->methods, (void *) name);
    if (e) {
        return (struct ena_func *) e->value;
    }

    return NULL;
}

static ena_value_t eval_method_call(struct ena_vm *vm, struct ena_node *node) {
    struct ena_node *prop_node = &node->child[0];
    ena_ident_t method_name = ena_cstr2ident(vm, prop_node->token->str);
    ena_value_t lhs = eval_node(vm, node->child[0].child);
    if (ena_get_type(lhs) != ENA_T_INSTANCE) {
        RUNTIME_ERROR("lhs is not an instance");
    }

    // Invoke the method.
    struct ena_instance *instance = (struct ena_instance *) lhs;
    struct ena_func *method = lookup_method(instance->cls, method_name);
    if (!method) {
        RUNTIME_ERROR("method %s is not defined", ena_ident2cstr(vm, method_name));
    }

    return eval_func_call(vm, &node->child[1], instance, method);
}

static ena_value_t eval_instantiate(struct ena_vm *vm, struct ena_node *node, struct ena_class *cls) {
    struct ena_instance *instance = ena_malloc(sizeof(*instance));
    instance->header.type = ENA_T_INSTANCE;
    instance->cls = cls;
    instance->header.refcount = 1;
    ena_hash_init_ident_table(&instance->props);

    // Call the constructor if it exists.
    struct ena_func *new_method = lookup_method(cls, ena_cstr2ident(vm, "new"));
    if (new_method) {
        eval_func_call(vm, &node->child[1], instance, new_method);
    }

    return (ena_value_t) instance;
}

EVAL_NODE(CALL) {
    if (node->child[0].type == ENA_NODE_ID) {
        ena_value_t func_name = ena_cstr2ident(vm, node->child[0].token->str);
        ena_value_t func = get_var_value(vm->current_scope, func_name);
        switch (ena_get_type(func)) {
            case ENA_T_FUNC:
                return eval_func_call(vm, &node->child[1], NULL, (struct ena_func *) func);
            case ENA_T_CLASS:
                return eval_instantiate(vm, node, (struct ena_class *) func);
            default:
                RUNTIME_ERROR("%s is not a functon or class.", ena_ident2cstr(vm, func_name));
        }
    } else {
        return eval_method_call(vm, node);
    }
}

EVAL_NODE(RETURN) {
    ena_value_t ret_value = eval_node(vm, node->child);
    if (vm->current_scope == NULL) {
        RUNTIME_ERROR("return from the top level");
        return ENA_UNDEFINED;
    }

    vm->current_scope = vm->current_scope->parent;
    vm->current_savepoint->ret_value = ret_value;
    UNWIND_SAVEPOINT(ENA_UNWIND_RETURN);

    /* UNREACHABLE */
}

EVAL_NODE(CLASS) {
    ena_value_t name = ena_cstr2ident(vm, node->token->str);
    struct ena_node *stmts = node->child;

    // Create a new class.
    struct ena_class *cls = ena_malloc(sizeof(*cls));
    cls->header.type = ENA_T_CLASS;
    cls->header.refcount = 1;
    ena_hash_init_ident_table(&cls->methods);

    // Evaluate the content.
    vm->current_class = cls;
    eval_node(vm, stmts);
    vm->current_class = NULL;

    ena_define_var(vm, name, (ena_value_t) cls);
    return ENA_UNDEFINED;
}

EVAL_NODE(ID) {
    ena_value_t name = ena_cstr2ident(vm, node->token->str);
    ena_value_t value = get_var_value(vm->current_scope, name);
    if (value == ENA_UNDEFINED) {
        RUNTIME_ERROR("%s is not defined", ena_ident2cstr(vm, name));
    }

    return value;
}

EVAL_NODE(PROP) {
    struct ena_instance *obj;
    if (!ena_strcmp(node->child->token->str, "self")) {
        obj = vm->current_instance;
    } else {
        NOT_YET_IMPLEMENTED();
    }

    ena_value_t prop_name = ena_cstr2ident(vm, node->token->str);
    ena_value_t value = get_var_from(&obj->props, prop_name);
    if (value == ENA_UNDEFINED) {
        RUNTIME_ERROR("%s is not defined", ena_ident2cstr(vm, prop_name));
    }
    return value;
}

EVAL_NODE(IF) {
    struct ena_node *condition_part = &node->child[0];
    struct ena_node *then_part = &node->child[1];
    struct ena_node *else_part = &node->child[2];
    ena_value_t condition = eval_node(vm, condition_part);
    if (ENA_TEST(condition)) {
        eval_node(vm, then_part);
    } else {
        if (node->num_childs == 3) {
            eval_node(vm, else_part);
        }
    }

    return ENA_UNDEFINED;
}

EVAL_NODE(WHILE) {
    struct ena_node *condition_part = &node->child[0];
    struct ena_node *stmts = &node->child[1];

    // Enther the scope and execute the body.
    PUSH_SAVEPOINT();
    int unwind_type;
    if ((unwind_type = EXEC_SAVEPOINT()) == 0) {
        while (ENA_TEST(eval_node(vm, condition_part))) {
            eval_node(vm, stmts);
while_continue:;
        }
    } else {
        // UNWIND_SAVEPOINT() is invoked in the callee.
        switch (unwind_type) {
            case ENA_UNWIND_BREAK:
                break;
            case ENA_UNWIND_CONTINUE:
                goto while_continue;
            case ENA_UNWIND_RETURN:
                // Continue unwinding.
                UNWIND_SAVEPOINT(unwind_type);
        }
    }

    POP_SAVEPOINT();
    return ENA_UNDEFINED;
}

EVAL_NODE(CONTINUE) {
    UNWIND_SAVEPOINT(ENA_UNWIND_CONTINUE);
    return ENA_UNDEFINED;
}

EVAL_NODE(BREAK) {
    UNWIND_SAVEPOINT(ENA_UNWIND_BREAK);
    return ENA_UNDEFINED;
}

EVAL_NODE(INT_LIT) {
    int value = ena_str2int(node->token->str);
    return ena_create_int(value);
}

EVAL_NODE(TRUE) {
    return ENA_TRUE;
}

EVAL_NODE(FALSE) {
    return ENA_FALSE;
}

static ena_value_t eval_node(struct ena_vm *vm, struct ena_node *node) {
    DEBUG("eval: %s", ena_get_node_name(node->type));

    switch (node->type) {
#define EVAL_CASE(name) \
        case ENA_NODE_ ## name: \
            return eval_ ## name(vm, node)

        EVAL_CASE(PROGRAM);
        EVAL_CASE(STMTS);
        EVAL_CASE(OP_ADD);
        EVAL_CASE(OP_SUB);
        EVAL_CASE(OP_LT);
        EVAL_CASE(OP_EQ);
        EVAL_CASE(OP_ASSIGN);
        EVAL_CASE(VAR);
        EVAL_CASE(FUNC);
        EVAL_CASE(INT_LIT);
        EVAL_CASE(TRUE);
        EVAL_CASE(FALSE);
        EVAL_CASE(CALL);
        EVAL_CASE(IF);
        EVAL_CASE(RETURN);
        EVAL_CASE(ID);
        EVAL_CASE(PROP);
        EVAL_CASE(CLASS);
        EVAL_CASE(WHILE);
        EVAL_CASE(CONTINUE);
        EVAL_CASE(BREAK);
        default:
            NOT_YET_IMPLEMENTED();
    }

    return ENA_UNDEFINED;
}

/// @returns true on success or false on panic (e.g. syntax error).
bool ena_eval(struct ena_vm *vm, char *script) {
    struct ena_ast *ast;
    if (ena_setjmp(vm->panic_jmpbuf) == 0) {
        ast = ena_parse(vm, script);
    } else {
        return false;
    }

    if (ena_setjmp(vm->panic_jmpbuf) == 0) {
        vm->current_scope = (struct ena_scope *) vm->main_module;
        eval_node(vm, ast->tree);
    } else {
        return false;
    }

    return true;
}
