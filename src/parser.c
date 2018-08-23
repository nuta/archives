/// @file
/// @brief A handmade LL(k) parser.

#include "api.h"
#include "parser.h"
#include "lexer.h"
#include "malloc.h"
#include "internal.h"

#define REALLOC_NODE_ARRAY(array, num) array = ena_realloc(array, sizeof(struct ena_node) * (num))
#define PARSE(rule) parse_##rule(vm)
#define SKIP(num) ena_skip_tokens(vm, num)
#define PUSHBACK(token) ena_pushback_token(vm, token)
#define PARSE_RULE(rule) static struct ena_node *parse_##rule(UNUSED struct ena_vm *vm)
#define NEXT_TYPE() ena_fetch_next_token_type(vm)
#define GET_NEXT() ena_get_next_token(vm)
#define FETCH_NEXT() ena_fetch_next_token(vm)
#define CONSUME(expected_type) ena_consume_token(vm, ENA_TOKEN_##expected_type)

const char *ena_get_node_name(enum ena_node_type type) {
#define DEFINE_NODE_NAME(name) [ENA_NODE_##name] = "NODE_" #name

    static const char *names[ENA_NODE_MAX_NUM] = {
        DEFINE_NODE_NAME(PROGRAM),
        DEFINE_NODE_NAME(STMTS),
        DEFINE_NODE_NAME(STMT),
        DEFINE_NODE_NAME(FUNC),
        DEFINE_NODE_NAME(RETURN),
        DEFINE_NODE_NAME(ID_LIST),
        DEFINE_NODE_NAME(EXPR_LIST),
        DEFINE_NODE_NAME(IF),
        DEFINE_NODE_NAME(VAR),
        DEFINE_NODE_NAME(TRUE),
        DEFINE_NODE_NAME(FALSE),
        DEFINE_NODE_NAME(ID),
        DEFINE_NODE_NAME(INT_LIT),
        DEFINE_NODE_NAME(CALL),
        DEFINE_NODE_NAME(OP_ASSIGN),
        DEFINE_NODE_NAME(OP_ADD),
        DEFINE_NODE_NAME(OP_SUB),
        DEFINE_NODE_NAME(OP_MUL),
        DEFINE_NODE_NAME(OP_LT),
        DEFINE_NODE_NAME(OP_EQ),
        DEFINE_NODE_NAME(PROP),
        DEFINE_NODE_NAME(CLASS),
        DEFINE_NODE_NAME(WHILE),
        DEFINE_NODE_NAME(BREAK),
        DEFINE_NODE_NAME(CONTINUE),
    };

    ENA_ASSERT(type < ENA_NODE_MAX_NUM);
    return names[type];
}

static void do_dump_node(struct ena_node *node, int level) {
    for (int i = 0; i < level; i++) {
        DEBUG_NONL("|  ");
    }

    if (node->token) {
        DEBUG("%s(%s)", ena_get_node_name(node->type), node->token->str);
    } else {
        DEBUG("%s",ena_get_node_name(node->type));
    }

    for (int i = 0; i < node->num_childs; i++) {
        do_dump_node(&node->child[i], level + 1);
    }
}


void ena_dump_node(struct ena_node *node) {
    do_dump_node(node, 0);
}


static struct ena_node *create_node(enum ena_node_type type, struct ena_node *child, int num_childs) {
    struct ena_node *node = ena_malloc(sizeof(*node));
    node->type = type;
    node->token = NULL;
    node->child = child;
    node->num_childs = num_childs;
    return node;
}

static struct ena_node *create_node_with_token(enum ena_node_type type, struct ena_token *token, struct ena_node *child, int num_childs) {
    struct ena_node *node = ena_malloc(sizeof(*node));
    node->type = type;
    node->token = token;
    node->child = child;
    node->num_childs = num_childs;
    return node;
}

static inline void set_nth_child(struct ena_node *array, int nth, struct ena_node *child) {
    ena_memcpy(&array[nth], child, sizeof(*child));
}


#define BIN_OP(token_type, node_type) \
    case ENA_TOKEN_##token_type: { \
        SKIP(1); \
        struct ena_node *rhs = PARSE(expr); \
        struct ena_node *childs = NULL; \
        REALLOC_NODE_ARRAY(childs, 2); \
        set_nth_child(childs, 0, lhs); \
        set_nth_child(childs, 1, rhs); \
        lhs = create_node(ENA_NODE_##node_type, childs, 2); \
        break; \
    }

// Forward declarations.
PARSE_RULE(expr);
PARSE_RULE(expr_list);
PARSE_RULE(stmt);
PARSE_RULE(stmts);
PARSE_RULE(program);

/// expr: boolean_or;
/// boolean_or: boolean_and ('or' expr)? ;
/// boolean_and: equality ('and' expr)? ;
/// equality: comparsion (('!=' | '==') expr)? ;
/// comparison: addition (('>') expr)? ;
/// addition: multiplication (('+' | '-') expr)? ;
/// multiplication: unary (('/' | '*') expr)? ;
/// unary: ('-' | not) unary
///      | primary '(' expr_list ')'
///      | primary
///      ;
/// primary: '(' expr ')'
///        | literal
///        | ID
///        ;
PARSE_RULE(primary) {
    struct ena_token *token = GET_NEXT();
    switch (token->type) {
        case ENA_TOKEN_LPAREN: {
            struct ena_node *expr = PARSE(expr);
            CONSUME(RPAREN);
            return expr;
        }
        case ENA_TOKEN_TRUE:
            return create_node(ENA_NODE_TRUE, NULL, 0);
        case ENA_TOKEN_FALSE:
            return create_node(ENA_NODE_FALSE, NULL, 0);
        case ENA_TOKEN_INT_LIT:
            return create_node_with_token(ENA_NODE_INT_LIT, token, NULL, 0);
        case ENA_TOKEN_ID:
            return create_node_with_token(ENA_NODE_ID, token, NULL, 0);
        default:
            // Reached to the end of expression.
            PUSHBACK(token);
            return NULL;
    }
}

PARSE_RULE(unary) {
    struct ena_node *lhs = PARSE(primary);
    for (;;) {
        switch (NEXT_TYPE()) {
            case ENA_TOKEN_LPAREN: {
                // Function call.
                CONSUME(LPAREN);
                struct ena_node *exprs = PARSE(expr_list);
                CONSUME(RPAREN);
                struct ena_node *childs = NULL;
                REALLOC_NODE_ARRAY(childs, 2);
                set_nth_child(childs, 0, lhs);
                set_nth_child(childs, 1, exprs);
                lhs = create_node(ENA_NODE_CALL, childs, 2);
                break;
            }
            case ENA_TOKEN_DOT: {
                SKIP(1);
                struct ena_token *prop = CONSUME(ID);
                lhs = create_node_with_token(ENA_NODE_PROP, prop, lhs, 1);
                break;
            }
            default:;
                return lhs;
        }
    }
}


PARSE_RULE(multiplication) {
    struct ena_node *lhs = PARSE(unary);
    for (;;) {
        switch (NEXT_TYPE()) {
            BIN_OP(ASTERISK, OP_MUL)
            default:;
                return lhs;
        }
    }
}

PARSE_RULE(addition) {
    struct ena_node *lhs = PARSE(multiplication);
    for (;;) {
        switch (NEXT_TYPE()) {
            BIN_OP(PLUS, OP_ADD)
            BIN_OP(MINUS, OP_SUB)
            default:
                return lhs;
        }
    }
}

PARSE_RULE(comparison) {
    struct ena_node *lhs = PARSE(addition);
    for (;;) {
        switch (NEXT_TYPE()) {
            BIN_OP(LT, OP_LT)
            default:;
                return lhs;
        }
    }
}

PARSE_RULE(equality) {
    struct ena_node *lhs = PARSE(comparison);
    for (;;) {
        switch (NEXT_TYPE()) {
            BIN_OP(DOUBLE_EQ, OP_EQ)
            default:;
                return lhs;
        }
    }
}

PARSE_RULE(expr) {
    struct ena_node *lhs = PARSE(equality);
    for (;;) {
        switch (NEXT_TYPE()) {
            BIN_OP(EQ, OP_ASSIGN)
            default:;
                return lhs;
        }
    }
}

// expr_list: ( expr (',' expr) * ) ?
// Childs: expr ...
PARSE_RULE(expr_list) {
    struct ena_node *exprs = NULL;
    int num_exprs = 0;
    for (;;) {
        REALLOC_NODE_ARRAY(exprs, num_exprs + 1);
        struct ena_node *expr = PARSE(expr);
        if (expr == NULL) {
            break;
        }

        set_nth_child(exprs, num_exprs, expr);
        num_exprs++;

        if (NEXT_TYPE() == ENA_TOKEN_COMMA) {
            CONSUME(COMMA);
        }
    }

    return create_node_with_token(ENA_NODE_EXPR_LIST, NULL, exprs, num_exprs);
}

// id_list: (id (',' id_list)*)?
// Childs: id ...
PARSE_RULE(id_list) {
    struct ena_node *childs = NULL;
    int num_childs = 0;

    for (;;) {
        if (NEXT_TYPE() != ENA_TOKEN_ID) {
            break;
        }

        REALLOC_NODE_ARRAY(childs, num_childs + 1);
        struct ena_token *token = CONSUME(ID);
        struct ena_node *id = create_node_with_token(ENA_NODE_ID, token, NULL, 0);
        set_nth_child(childs, num_childs, id);
        num_childs++;

        if (NEXT_TYPE() == ENA_TOKEN_COMMA) {
            CONSUME(COMMA);
        }
    }

    return create_node_with_token(ENA_NODE_ID_LIST, NULL, childs, num_childs);
}

// Token: The class name. (ENA_NODE_ID)
// Childs: stmts (stmts)
PARSE_RULE(class_stmt) {
    CONSUME(CLASS);
    struct ena_token *name_token = CONSUME(ID);
    CONSUME(LBRACKET);
    struct ena_node *stmts = PARSE(stmts);
    CONSUME(RBRACKET);
    return create_node_with_token(ENA_NODE_CLASS, name_token, stmts, 1);
}

// Childs: args (id_list), stmts (stmts)
PARSE_RULE(func_stmt) {
    CONSUME(FUNC);
    struct ena_token *name_token = CONSUME(ID);

    CONSUME(LPAREN);
    struct ena_node *args = PARSE(id_list);
    CONSUME(RPAREN);

    CONSUME(LBRACKET);
    struct ena_node *stmts = PARSE(stmts);
    CONSUME(RBRACKET);

    struct ena_node *childs = NULL;
    REALLOC_NODE_ARRAY(childs, 2);
    set_nth_child(childs, 0, args);
    set_nth_child(childs, 1, stmts);

    return create_node_with_token(ENA_NODE_FUNC, name_token, childs, 2);
}

/// Token: The variable name. (ENA_NODE_ID)
/// Childs:
///   initializer? : expr
PARSE_RULE(var_stmt) {
    CONSUME(VAR);
    struct ena_token *name_token = CONSUME(ID);

    if (NEXT_TYPE() == ENA_TOKEN_EQ) {
        SKIP(1);
        struct ena_node *initializer = PARSE(expr);
        CONSUME(SEMICOLON);
        return create_node_with_token(ENA_NODE_VAR, name_token, initializer, 1);
    } else {
        CONSUME(SEMICOLON);
        return create_node_with_token(ENA_NODE_VAR, name_token, NULL, 0);
    }
}

// Childs: return value (expr)
PARSE_RULE(return_stmt) {
    CONSUME(RETURN);
    struct ena_node *expr = PARSE(expr);
    CONSUME(SEMICOLON);
    return create_node_with_token(ENA_NODE_RETURN, NULL, expr, 1);
}

// if_stmt: 'if' expr block ('elseif' expr block)* ('else' block)?;
// Childs: condition (expr), then part (stmts), else part (stmts)
PARSE_RULE(if_stmt) {
    CONSUME(IF);
    struct ena_node *condition = PARSE(expr);
    CONSUME(LBRACKET);

    struct ena_node *then_stmts = PARSE(stmts);
    CONSUME(RBRACKET);

    int num_childs;
    struct ena_node *childs = NULL;
    if (NEXT_TYPE() == ENA_TOKEN_ELSE) {
        // Found else part.
        SKIP(1);
        CONSUME(LBRACKET);
        struct ena_node *else_stmts = PARSE(stmts);
        CONSUME(RBRACKET);
        num_childs = 3; // condition, then part, and else part
        REALLOC_NODE_ARRAY(childs, num_childs);
        set_nth_child(childs, 2, else_stmts);
    } else {
        num_childs = 2; // condition and then part
        REALLOC_NODE_ARRAY(childs, num_childs);
    }

    set_nth_child(childs, 0, condition);
    set_nth_child(childs, 1, then_stmts);
    return create_node(ENA_NODE_IF, childs, num_childs);
}

// while_stmt: 'while' expr block
// Childs: condition (expr), body (stmts)
PARSE_RULE(while_stmt) {
    CONSUME(WHILE);
    struct ena_node *condition = PARSE(expr);
    CONSUME(LBRACKET);
    struct ena_node *stmts = PARSE(stmts);
    CONSUME(RBRACKET);

    struct ena_node *childs = NULL;
    REALLOC_NODE_ARRAY(childs, 2);
    set_nth_child(childs, 0, condition);
    set_nth_child(childs, 1, stmts);
    return create_node(ENA_NODE_WHILE, childs, 2);
}

PARSE_RULE(break_stmt) {
    CONSUME(BREAK);
    CONSUME(SEMICOLON);
    return create_node(ENA_NODE_BREAK, NULL, 0);
}

PARSE_RULE(continue_stmt) {
    CONSUME(CONTINUE);
    CONSUME(SEMICOLON);
    return create_node(ENA_NODE_CONTINUE, NULL, 0);
}


// stmt
//     : if_stmt
//     | for_stmt
//     | while_stmt
//     | break_stmt
//     | continue_stmt
//     | return_stmt
//     | var_def
//     | func_def
//     | class_def
//     | expr ';'
//     ;
PARSE_RULE(stmt) {
    struct ena_token *token = FETCH_NEXT();

    switch (token->type) {
        case ENA_TOKEN_CLASS:    return PARSE(class_stmt);
        case ENA_TOKEN_FUNC:     return PARSE(func_stmt);
        case ENA_TOKEN_IF:       return PARSE(if_stmt);
        case ENA_TOKEN_WHILE:    return PARSE(while_stmt);
        case ENA_TOKEN_VAR:      return PARSE(var_stmt);
        case ENA_TOKEN_RETURN:   return PARSE(return_stmt);
        case ENA_TOKEN_BREAK:    return PARSE(break_stmt);
        case ENA_TOKEN_CONTINUE: return PARSE(continue_stmt);
        case ENA_TOKEN_EOF:
        case ENA_TOKEN_RBRACKET:
            return NULL;
        default: {
            // Maybe an expression.
            struct ena_node *expr = PARSE(expr);
            CONSUME(SEMICOLON);
            return expr;
        }
    }
}


// stmts: stmt*
PARSE_RULE(stmts) {
    int num_stmts = 0;
    struct ena_node *childs = NULL;
    for (;;) {
        struct ena_node *stmt = PARSE(stmt);
        if (!stmt) {
            break;
        }

        REALLOC_NODE_ARRAY(childs, num_stmts + 1);
        set_nth_child(childs, num_stmts, stmt);
        num_stmts++;
    }

    return create_node(ENA_NODE_STMTS, childs, num_stmts);
}


// program: stmts EOF
PARSE_RULE(program) {
    struct ena_node *child = PARSE(stmts);
    return create_node(ENA_NODE_PROGRAM, child, 1);
}


struct ena_ast *ena_parse(struct ena_vm *vm, const char *script) {
    struct ena_ast *ast = ena_malloc(sizeof(*ast));

    // Initialize lexer state.
    vm->lexer.next_pos = 0;
    vm->lexer.current_line = 1;
    vm->lexer.current_column = 1;
    vm->lexer.script = ena_strdup(script);

    ast->tree = parse_program(vm);
    return ast;
}

void ena_destroy_ast(struct ena_ast *ast) {
    ena_free(ast);
}
