#include "api.h"
#include "parser.h"
#include "eval.h"
#include "internal.h"
#include "malloc.h"
#include "gc.h"

#define EVAL_NODE(name) static ena_value_t eval_##name(UNUSED struct ena_vm *vm, UNUSED struct ena_node *node)
static ena_value_t eval_node(struct ena_vm *vm, struct ena_node *node);

static void ena_share_scope(struct ena_scope *scope) {
    scope->refcount++;
}

/// Decrements the reference count of scope.
/// @arg scope The scope.
static void ena_delete_scope(struct ena_scope *scope) {
    scope->refcount--;
    if (scope->refcount == 0) {
        ena_free(scope);
    }
}

static inline struct ena_class *get_builtin_class_by_type(struct ena_vm *vm, ena_value_type_t type) {
    switch (type) {
        case ENA_T_INT:
            return vm->int_class;
        case ENA_T_STRING:
            return vm->string_class;
        case ENA_T_LIST:
            return vm->list_class;
        case ENA_T_MAP:
            return vm->map_class;
        default:;
    }

    NOT_YET_IMPLEMENTED();
    return NULL;
}

static ena_value_t copy_if_immutable(struct ena_vm *vm, ena_value_t value) {
    switch (ena_get_type(value)) {
        case ENA_T_INT:
            return ena_create_int(vm, ena_to_int_object(value)->value);
        default:
            return value;
    }
}

static ena_value_t do_call_func(struct ena_vm *vm, ena_value_t *args, int num_args, ena_value_t self, struct ena_func *func) {
    // Enther the scope and execute the body.
    struct ena_scope *caller_scope = vm->current_scope;
    struct ena_scope *new_scope = ena_create_scope(func->scope);
    PUSH_SAVEPOINT();

    int unwind_type;
    ena_value_t ret_value = ENA_UNDEFINED;
    if ((unwind_type = EXEC_SAVEPOINT()) == 0) {
        if (func->flags & FUNC_FLAGS_NATIVE) {
            if (func->flags & FUNC_FLAGS_METHOD) {
                ret_value = func->native_method(vm, self, args, num_args);
            } else {
                ret_value = func->native_func(vm, args, num_args);
            }
        } else {
            int num_params = func->param_names->num_childs;
            if (num_args != num_params) {
                RUNTIME_ERROR("incorrect # of arguments");
            }

            // Evaluate and fill arguments into the newly created scope.
            for (int i = 0; i < num_args; i++) {
                ena_ident_t param_name = ena_cstr2ident(vm, func->param_names->child[i].token->str);
                ena_define_var(vm, new_scope, param_name, args[i]);
            }

            vm->current_scope = new_scope;
            vm->self = self;
            eval_node(vm, func->stmts);
        }
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
    vm->self = ENA_UNDEFINED;
    POP_SAVEPOINT();
    ena_delete_scope(new_scope);
    return ret_value;
}

static ena_value_t call_func(struct ena_vm *vm, struct ena_node *expr_list, ena_value_t self, struct ena_func *func) {
    int num_args = expr_list->num_childs;
    ena_value_t *args = ena_malloc(sizeof(ena_value_t) * num_args);
    for (int i = 0; i < num_args; i++) {
        args[i] = eval_node(vm, &expr_list->child[i]);
    }

    ena_value_t ret_value = do_call_func(vm, args, num_args, self, func);
    ena_free(args);
    return ret_value;
}

static struct ena_func *lookup_method(struct ena_class *cls, ena_ident_t name) {
    struct ena_hash_entry *e = ena_hash_search(&cls->methods, (void *) name);
    if (e) {
        return (struct ena_func *) e->value;
    }

    return NULL;
}

static ena_value_t invoke_method(struct ena_vm *vm, struct ena_node *node) {
    struct ena_node *prop_node = &node->child[0];
    ena_ident_t method_name = ena_cstr2ident(vm, prop_node->token->str);
    ena_value_t instance = eval_node(vm, node->child[0].child);

    struct ena_class *cls;
    ena_value_type_t lhs_type = ena_get_type(instance);
    switch (lhs_type) {
        case ENA_T_INSTANCE:
            cls = ((struct ena_instance *) instance)->cls;
            break;
        case ENA_T_STRING:
        case ENA_T_LIST:
        case ENA_T_MAP:
            cls = get_builtin_class_by_type(vm, lhs_type);
            break;
        default:
            RUNTIME_ERROR("lhs is not class");
    }

    // Invoke the method.
    struct ena_func *method = lookup_method(cls, method_name);
    if (!method) {
        RUNTIME_ERROR("method %s is not defined", ena_ident2cstr(vm, method_name));
    }

    return call_func(vm, &node->child[1], instance, method);
}

static ena_value_t instantiate(struct ena_vm *vm, struct ena_node *node, struct ena_class *cls) {
    struct ena_instance *instance = (struct ena_instance *) ena_alloc_object(vm, ENA_T_INSTANCE);
    instance->cls = cls;
    instance->props = ena_create_scope(NULL);

    // Call the constructor if it exists.
    struct ena_func *new_method = lookup_method(cls, ena_cstr2ident(vm, "new"));
    if (new_method) {
        call_func(vm, &node->child[1], ENA_OBJ2VALUE(instance), new_method);
    }

    return (ena_value_t) instance;
}


#define EVAL_BINOP_NODE(op_name, method_name) \
    EVAL_NODE(op_name) { \
        ena_value_t lhs = eval_node(vm, &node->child[0]); \
        ena_value_t rhs = eval_node(vm, &node->child[1]); \
        ena_value_type_t lhs_type = ena_get_type(lhs); \
        ena_value_type_t rhs_type = ena_get_type(rhs); \
        if (lhs_type != rhs_type) { \
            TYPE_ERROR("type mismatch"); \
        } \
        struct ena_class *cls = get_builtin_class_by_type(vm, lhs_type); \
        struct ena_func *method = lookup_method(cls, ena_cstr2ident(vm, method_name)); \
        if (!method) { \
            RUNTIME_ERROR("method `%s' is not defined", method_name); \
        } \
        ena_value_t args[] = { rhs }; \
        ena_value_t result = do_call_func(vm, (ena_value_t *) &args, 1, lhs, method); \
        return result; \
    }

EVAL_BINOP_NODE(OP_ADD, "+")
EVAL_BINOP_NODE(OP_SUB, "-")
EVAL_BINOP_NODE(OP_MUL, "*")
EVAL_BINOP_NODE(OP_DIV, "/")
EVAL_BINOP_NODE(OP_MOD, "%")
EVAL_BINOP_NODE(OP_EQ,  "==")
EVAL_BINOP_NODE(OP_NEQ, "!=")
EVAL_BINOP_NODE(OP_LT,  "<")
EVAL_BINOP_NODE(OP_LTE, "<=")
EVAL_BINOP_NODE(OP_GT,  ">")
EVAL_BINOP_NODE(OP_GTE, ">=")

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

    ena_define_var(vm, vm->current_scope, name, initial);
    return ENA_UNDEFINED;
}

EVAL_NODE(OP_ASSIGN) {
    ena_ident_t name = ena_cstr2ident(vm, node->child[0].token->str);
    ena_value_t rvalue = eval_node(vm, &node->child[1]);

    if (node->child[0].type == ENA_NODE_PROP) {
        struct ena_node *obj = node->child[0].child;
        // FIXME:
        if (!ena_strcmp(obj->token->str, "self")) {
            if (vm->self == ENA_UNDEFINED) {
                RUNTIME_ERROR("self is not available");
            }

            if (ena_get_type(vm->self) != ENA_T_INSTANCE) {
                RUNTIME_ERROR("self is not an instance");
            }

            struct ena_instance *self = (struct ena_instance *) vm->self;
            if (!ena_set_var_value(self->props, name, rvalue)) {
                ena_define_var(vm, self->props, name, rvalue);
            }
        } else {
            NOT_YET_IMPLEMENTED();
        }
    } else {
        if (!ena_set_var_value(vm->current_scope, name, rvalue)) {
            RUNTIME_ERROR("%s is not defined", ena_ident2cstr(vm, name));
        }
    }

    return rvalue;
}

EVAL_NODE(FUNC) {
    ena_ident_t name = ena_cstr2ident(vm, node->token->str);
    struct ena_node *param_names = &node->child[0];
    struct ena_node *stmts = &node->child[1];
    struct ena_func *func = (struct ena_func *) ena_alloc_object(vm, ENA_T_FUNC);
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
        ena_share_scope(vm->current_scope);
    }

    func->flags = type;
    if (ena_hash_search_or_insert(table, (void *) name, (void *) func)) {
        RUNTIME_ERROR("%s is already defined", ena_ident2cstr(vm, name));
    }

    return ENA_UNDEFINED;
}

EVAL_NODE(CALL) {
    if (node->child[0].type == ENA_NODE_ID) {
        ena_value_t func_name = ena_cstr2ident(vm, node->child[0].token->str);
        ena_value_t func = ena_get_var_value (vm->current_scope, func_name);
        switch (ena_get_type(func)) {
            case ENA_T_FUNC:
                return call_func(vm, &node->child[1], ENA_UNDEFINED, (struct ena_func *) func);
            case ENA_T_CLASS:
                return instantiate(vm, node, (struct ena_class *) func);
            default:
                RUNTIME_ERROR("%s is not a functon or class.", ena_ident2cstr(vm, func_name));
        }
    } else {
        return invoke_method(vm, node);
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
    ena_value_t cls = ena_create_class(vm);

    // Evaluate the content.
    vm->current_class = ena_to_class_object(cls);
    eval_node(vm, stmts);
    vm->current_class = NULL;

    ena_define_var(vm, vm->current_scope, name, cls);
    return ENA_UNDEFINED;
}

EVAL_NODE(ID) {
    ena_value_t name = ena_cstr2ident(vm, node->token->str);
    ena_value_t value = ena_get_var_value(vm->current_scope, name);

    if (value == ENA_UNDEFINED) {
        if (name == ena_cstr2ident(vm, "self")) {
            return vm->self;
        }

        RUNTIME_ERROR("%s is not defined", ena_ident2cstr(vm, name));
    }

    return copy_if_immutable(vm, value);
}

EVAL_NODE(PROP) {
    ena_value_t prop_name = ena_cstr2ident(vm, node->token->str);
    ena_value_t self_value = eval_node(vm, node->child);
    if (ena_get_type(self_value) != ENA_T_INSTANCE) {
        RUNTIME_ERROR("self is not instance");
    }

    struct ena_instance *self = ena_to_instance_object(self_value);
    ena_value_t value = ena_get_var_value(self->props, prop_name);
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

EVAL_NODE(NULL) {
    return ENA_NULL;
}

EVAL_NODE(INT_LIT) {
    int value = ena_str2int(node->token->str);
    return ena_create_int(vm, value);
}

EVAL_NODE(STRING_LIT) {
    // The string in `node->token->str` is enclosed by doublequote. Remove
    // them by tweaking the pointer and size.
    const char *str = node->token->str + 1;
    size_t size = ena_strlen(node->token->str) - 2;
    return ena_create_string(vm, str, size);
}

EVAL_NODE(LIST_LIT) {
    struct ena_node *expr_list = node->child;
    struct ena_list *list = (struct ena_list *) ena_alloc_object(vm, ENA_T_LIST);

    size_t num_elems = expr_list->num_childs;
    list->elems = ena_malloc(sizeof(ena_value_t) * num_elems);
    list->num_elems = num_elems;
    for (size_t i = 0; i < num_elems; i++) {
        list->elems[i] = eval_node(vm, &expr_list->child[i]);
    }

    return ENA_OBJ2VALUE(list);
}

EVAL_NODE(MAP_LIT) {
    struct ena_map *map = (struct ena_map *) ena_alloc_object(vm, ENA_T_MAP);
    ena_hash_init_value_table(&map->entries);

    for (int i = 0; i < node->num_childs; i++) {
        ena_value_t key = eval_node(vm, &node->child[i].child[0]);
        ena_value_t value = eval_node(vm, &node->child[i].child[1]);
        ena_hash_insert(&map->entries, (void *) key, (void *) value);
    }

    return ENA_OBJ2VALUE(map);
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
        EVAL_CASE(OP_DIV);
        EVAL_CASE(OP_MUL);
        EVAL_CASE(OP_MOD);
        EVAL_CASE(OP_EQ);
        EVAL_CASE(OP_NEQ);
        EVAL_CASE(OP_LT);
        EVAL_CASE(OP_LTE);
        EVAL_CASE(OP_GT);
        EVAL_CASE(OP_GTE);
        EVAL_CASE(OP_ASSIGN);
        EVAL_CASE(VAR);
        EVAL_CASE(FUNC);
        EVAL_CASE(NULL);
        EVAL_CASE(INT_LIT);
        EVAL_CASE(STRING_LIT);
        EVAL_CASE(LIST_LIT);
        EVAL_CASE(MAP_LIT);
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
bool ena_eval(struct ena_vm *vm, ena_value_t module, const char *filepath, char *script) {
    struct ena_ast *ast;
    if (ena_setjmp(vm->panic_jmpbuf) == 0) {
        ast = ena_parse(vm, filepath, script);
    } else {
        return false;
    }

    // Append to the vm->ast_list.
    struct ena_ast **ast_entry = &vm->ast_list;
    while (*ast_entry != NULL) {
        ast_entry = &(*ast_entry)->next;
    }
    ast->next = NULL;
    *ast_entry = ast;

    if (ena_setjmp(vm->panic_jmpbuf) == 0) {
        vm->current_scope = ena_to_module_object(module)->scope;
        eval_node(vm, ast->tree);
    } else {
        return false;
    }

    return true;
}
