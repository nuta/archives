#include "api.h"
#include "parser.h"
#include "eval.h"
#include "internal.h"
#include "malloc.h"
#include "gc.h"

#define EVAL_NODE(name) static ena_value_t eval_##name(UNUSED struct ena_vm *vm, UNUSED struct ena_node *node)
static ena_value_t eval_node(struct ena_vm *vm, struct ena_node *node);

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
    switch (ena_get_type(vm, value)) {
        case ENA_T_INT:
            return ena_create_int(vm, ena_to_int(vm, value));
        default:
            return value;
    }
}

static ena_value_t call_func(struct ena_vm *vm, ena_value_t self, struct ena_func *func, ena_value_t *args, int num_args) {
    // Enther the scope and execute the body.
    struct ena_scope *caller_scope = vm->current_scope;
    struct ena_scope *new_scope = ena_create_scope(vm, func->scope);
    PUSH_UNWIND_POINT();
    PUSH_FRAME(func);

    int unwind_type;
    ena_value_t ret_value = ENA_UNDEFINED;
    if ((unwind_type = EXEC_UNWIND_POINT()) == 0) {
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
        // UNWIND_UNWIND_POINT() is invoked in the callee.
        switch (unwind_type) {
            case ENA_UNWIND_RETURN:
                ret_value = vm->current_unwind_point->ret_value;
                break;
        }
    }

    // Returned from the function.
    vm->current_scope = caller_scope;
    vm->self = ENA_UNDEFINED;
    POP_FRAME();
    POP_UNWIND_POINT();
    return ret_value;
}

static struct ena_func *lookup_method(struct ena_vm *vm, struct ena_class *cls, ena_ident_t name) {
    struct ena_hash_entry *e = ena_hash_search(vm, &cls->methods, (void *) name);
    if (e) {
        return (struct ena_func *) e->value;
    }

    return NULL;
}

static ena_value_t invoke_method(struct ena_vm *vm, ena_ident_t method_name, ena_value_t self, ena_value_t *args, int num_args) {
    struct ena_class *cls;
    ena_value_type_t lhs_type = ena_get_type(vm, self);
    switch (lhs_type) {
        case ENA_T_INSTANCE:
            cls = ((struct ena_instance *) self)->cls;
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
    struct ena_func *method = lookup_method(vm, cls, method_name);
    if (!method) {
        RUNTIME_ERROR("method %s is not defined", ena_ident2cstr(vm, method_name));
    }

    return call_func(vm, self, method, args, num_args);
}

static ena_value_t instantiate(struct ena_vm *vm, struct ena_node *node, struct ena_class *cls) {
    struct ena_scope *scope = ena_create_scope(vm, NULL);
    struct ena_instance *instance = (struct ena_instance *) ena_alloc_object(vm, ENA_T_INSTANCE);
    instance->cls = cls;
    instance->props = scope;

    // Call the constructor if it exists.
    struct ena_func *new_method = lookup_method(vm, cls, ena_cstr2ident(vm, "new"));
    if (new_method) {
        // Use ena_alloca() instead of ena_malloc() or GC would destroys args.
        struct ena_node *expr_list = &node->child[1];
        int num_args = expr_list->num_childs;
        ena_value_t *args = ena_alloca(sizeof(ena_value_t) * num_args);
        for (int i = 0; i < num_args; i++) {
            args[i] = eval_node(vm, &expr_list->child[i]);
        }

        call_func(vm, ENA_OBJ2VALUE(instance), new_method, args, num_args);
    }

    return (ena_value_t) instance;
}


#define EVAL_BINOP_NODE(op_name, method_name) \
    EVAL_NODE(op_name) { \
        ena_value_t lhs = eval_node(vm, &node->child[0]); \
        ena_value_t rhs = eval_node(vm, &node->child[1]); \
        ena_value_type_t lhs_type = ena_get_type(vm, lhs); \
        ena_value_type_t rhs_type = ena_get_type(vm, rhs); \
        if (lhs_type != rhs_type) { \
            TYPE_ERROR("%s: type mismatch", method_name); \
        } \
        struct ena_class *cls = get_builtin_class_by_type(vm, lhs_type); \
        struct ena_func *method = lookup_method(vm, cls, ena_cstr2ident(vm, method_name)); \
        if (!method) { \
            RUNTIME_ERROR("method `%s' is not defined", method_name); \
        } \
        ena_value_t args[] = { rhs }; \
        ena_value_t result = call_func(vm, lhs, method, (ena_value_t *) &args, 1); \
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
    ena_value_t rvalue = eval_node(vm, &node->child[1]);
    ena_node_type_t lvalue_type = node->child[0].type;

    switch (lvalue_type) {
        case ENA_NODE_PROP: {
            ena_ident_t name = ena_cstr2ident(vm, node->child[0].token->str);
            struct ena_node *obj = node->child[0].child;
            // FIXME:
            if (!ena_strcmp(obj->token->str, "self")) {
                if (vm->self == ENA_UNDEFINED) {
                    RUNTIME_ERROR("self is not available");
                }

                if (ena_get_type(vm, vm->self) != ENA_T_INSTANCE) {
                    RUNTIME_ERROR("self is not an instance");
                }

                struct ena_instance *self = (struct ena_instance *) vm->self;
                if (!ena_set_var_value(vm, self->props, name, rvalue)) {
                    ena_define_var(vm, self->props, name, rvalue);
                }
            } else {
                NOT_YET_IMPLEMENTED();
            }
            break;
        }
        case ENA_NODE_INDEX: {
            ena_value_t self = eval_node(vm, &node->child[0].child[0]);
            ena_value_t index = eval_node(vm, &node->child[0].child[1]);
            ena_ident_t method_name = ena_cstr2ident(vm, "[]=");
            ena_value_t args[] = { index, rvalue };
            invoke_method(vm, method_name, self, (ena_value_t *) &args, 2);
            break;
        }
        case ENA_NODE_ID: {
            ena_ident_t name = ena_cstr2ident(vm, node->child[0].token->str);
            if (!ena_set_var_value(vm, vm->current_scope, name, rvalue)) {
                RUNTIME_ERROR("%s is not defined", ena_ident2cstr(vm, name));
            }
            break;
        }
        default:;
            RUNTIME_ERROR("Invalid lvalue.");
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
    }

    func->flags = type;
    if (ena_hash_search_or_insert(vm, table, (void *) name, (void *) func)) {
        RUNTIME_ERROR("%s is already defined", ena_ident2cstr(vm, name));
    }

    return ENA_UNDEFINED;
}

EVAL_NODE(CALL) {
    if (node->child[0].type == ENA_NODE_ID) {
        ena_value_t func_name = ena_cstr2ident(vm, node->child[0].token->str);
        ena_value_t func = ena_get_var_value(vm, vm->current_scope, func_name);
        switch (ena_get_type(vm, func)) {
            case ENA_T_FUNC: {
                // Use ena_alloca() instead of ena_malloc() or GC would destroys args.
                struct ena_node *expr_list = &node->child[1];
                int num_args = expr_list->num_childs;
                ena_value_t *args = ena_alloca(sizeof(ena_value_t) * num_args);
                for (int i = 0; i < num_args; i++) {
                    args[i] = eval_node(vm, &expr_list->child[i]);
                }
                return call_func(vm, ENA_UNDEFINED, (struct ena_func *) func, args, num_args);
            }
            case ENA_T_CLASS:
                return instantiate(vm, node, (struct ena_class *) func);
            default:
                RUNTIME_ERROR("%s is not a functon or class.", ena_ident2cstr(vm, func_name));
        }
    } else {
        struct ena_node *prop_node = &node->child[0];
        struct ena_node *expr_list = &node->child[1];
        ena_ident_t method_name = ena_cstr2ident(vm, prop_node->token->str);
        ena_value_t self = eval_node(vm, node->child[0].child);

        // Use ena_alloca() instead of ena_malloc() or GC would destroys args.
        int num_args = expr_list->num_childs;
        ena_value_t *args = ena_alloca(sizeof(ena_value_t) * num_args);
        for (int i = 0; i < num_args; i++) {
            args[i] = eval_node(vm, &expr_list->child[i]);
        }

        return invoke_method(vm, method_name, self, args, num_args);
    }
}

EVAL_NODE(RETURN) {
    ena_value_t ret_value = node->child ? eval_node(vm, node->child) : ENA_NULL;
    if (vm->current_scope == NULL) {
        RUNTIME_ERROR("return from the top level");
        return ENA_UNDEFINED;
    }

    vm->current_scope = vm->current_scope->parent;
    vm->current_unwind_point->ret_value = ret_value;
    UNWIND_UNWIND_POINT(ENA_UNWIND_RETURN);

    /* UNREACHABLE */
}

EVAL_NODE(CLASS) {
    ena_value_t name = ena_cstr2ident(vm, node->token->str);
    struct ena_node *stmts = node->child;

    // Create a new class.
    ena_value_t cls = ena_create_class(vm);

    // Evaluate the content.
    vm->current_class = ena_to_class_object(vm, cls);
    eval_node(vm, stmts);
    vm->current_class = NULL;

    ena_define_var(vm, vm->current_scope, name, cls);
    return ENA_UNDEFINED;
}

EVAL_NODE(ID) {
    ena_value_t name = ena_cstr2ident(vm, node->token->str);
    ena_value_t value = ena_get_var_value(vm, vm->current_scope, name);

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
    if (ena_get_type(vm, self_value) != ENA_T_INSTANCE) {
        RUNTIME_ERROR("self is not instance");
    }

    struct ena_instance *self = ena_to_instance_object(vm, self_value);
    ena_value_t value = ena_get_var_value(vm, self->props, prop_name);
    if (value == ENA_UNDEFINED) {
        RUNTIME_ERROR("%s is not defined", ena_ident2cstr(vm, prop_name));
    }

    return value;
}

EVAL_NODE(INDEX) {
    ena_value_t self = eval_node(vm, &node->child[0]);
    ena_value_t index = eval_node(vm, &node->child[1]);
    ena_ident_t method_name = ena_cstr2ident(vm, "[]");
    ena_value_t args[] = { index };
    return invoke_method(vm, method_name, self, (ena_value_t *) &args, 1);
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
    PUSH_UNWIND_POINT();
    int unwind_type;
    if ((unwind_type = EXEC_UNWIND_POINT()) == 0) {
        while (ENA_TEST(eval_node(vm, condition_part))) {
            eval_node(vm, stmts);
while_continue:;
        }
    } else {
        // UNWIND_UNWIND_POINT() is invoked in the callee.
        switch (unwind_type) {
            case ENA_UNWIND_BREAK:
                break;
            case ENA_UNWIND_CONTINUE:
                goto while_continue;
            case ENA_UNWIND_RETURN:
                // Continue unwinding.
                UNWIND_UNWIND_POINT(unwind_type);
        }
    }

    POP_UNWIND_POINT();
    return ENA_UNDEFINED;
}

EVAL_NODE(CONTINUE) {
    UNWIND_UNWIND_POINT(ENA_UNWIND_CONTINUE);
    return ENA_UNDEFINED;
}

EVAL_NODE(BREAK) {
    UNWIND_UNWIND_POINT(ENA_UNWIND_BREAK);
    return ENA_UNDEFINED;
}

EVAL_NODE(NULL) {
    return ENA_NULL;
}

EVAL_NODE(INT_LIT) {
    return ena_create_int(vm, node->literal);
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
    ena_hash_init_value_table(vm, &map->entries);

    for (int i = 0; i < node->num_childs; i++) {
        ena_value_t key = eval_node(vm, &node->child[i].child[0]);
        ena_value_t value = eval_node(vm, &node->child[i].child[1]);
        ena_hash_insert(vm, &map->entries, (void *) key, (void *) value);
    }

    return ENA_OBJ2VALUE(map);
}

EVAL_NODE(TRUE) {
    return ENA_TRUE;
}

EVAL_NODE(FALSE) {
    return ENA_FALSE;
}

EVAL_NODE(OP_NOT) {
    bool value = eval_node(vm, node->child) == ENA_TRUE;
    return value ? ENA_FALSE : ENA_TRUE;
}

EVAL_NODE(OP_AND) {
    bool lhs = eval_node(vm, &node->child[0]) == ENA_TRUE;
    bool rhs = eval_node(vm, &node->child[1]) == ENA_TRUE;
    return (lhs && rhs) ? ENA_TRUE : ENA_FALSE;
}

EVAL_NODE(OP_OR) {
    bool lhs = eval_node(vm, &node->child[0]) == ENA_TRUE;
    bool rhs = eval_node(vm, &node->child[1]) == ENA_TRUE;
    return (lhs || rhs) ? ENA_TRUE : ENA_FALSE;
}

static ena_value_t eval_node(struct ena_vm *vm, struct ena_node *node) {
    DEBUG("eval: %s, L%d", ena_get_node_name(node->type), node->lineno);
    vm->current_node = node;

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
        EVAL_CASE(OP_NOT);
        EVAL_CASE(OP_OR);
        EVAL_CASE(OP_AND);
        EVAL_CASE(INDEX);
        default:
            NOT_YET_IMPLEMENTED();
    }

    return ENA_UNDEFINED;
}

/// @returns true on success or false on panic (e.g. syntax error).
bool ena_eval(struct ena_vm *vm, ena_value_t module, const char *filepath, char *script) {
    vm->stack_end = arch_get_stack_bottom();
    vm->error.type = ENA_ERROR_NONE;

    struct ena_ast *ast;
    if ((ast = ena_parse(vm, filepath, script)) == NULL) {
        vm->stack_end = 0;
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
        vm->current_scope = ena_to_module_object(vm, module)->scope;
        PUSH_UNWIND_POINT();
        PUSH_FRAME(NULL);
        int unwind_type;
        if ((unwind_type = EXEC_UNWIND_POINT()) == 0) {
            eval_node(vm, ast->tree);
        } else {
            RUNTIME_ERROR("uncaught unwind %d", unwind_type);
        }
#ifdef __EMSCRIPTEN__
        /* XXX: In emscripten longjmp() jumps into here. */
        if (vm->error.type != ENA_ERROR_NONE) {
            return false;
        }
#endif
    } else {
        vm->stack_end = 0;
        return false;
    }

    vm->stack_end = 0;
    return true;
}
