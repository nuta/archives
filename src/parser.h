#ifndef __ENA_PARSER_H__
#define __ENA_PARSER_H__

#include "internal.h"

struct ena_token {
    // The type of token.
    enum ena_token_type type;
    // The token string (terminated by NUL).
    char *str;
    // The position in `script`.
    int line;
    int column;
};

enum ena_node_type {
    ENA_NODE_UNKNOWN, // used internally
    ENA_NODE_PROGRAM,
    ENA_NODE_STMTS,
    ENA_NODE_STMT,
    ENA_NODE_VAR,
    ENA_NODE_FUNC,
    ENA_NODE_CALL,
    ENA_NODE_ID_LIST,
    ENA_NODE_EXPR_LIST,
    ENA_NODE_IF,
    ENA_NODE_WHILE,
    ENA_NODE_BREAK,
    ENA_NODE_CONTINUE,
    ENA_NODE_RETURN,
    ENA_NODE_TRUE,
    ENA_NODE_FALSE,
    ENA_NODE_INT_LIT,
    ENA_NODE_ID,
    ENA_NODE_OP_ASSIGN,
    ENA_NODE_OP_ADD,
    ENA_NODE_OP_SUB,
    ENA_NODE_OP_MUL,
    ENA_NODE_OP_LT,
    ENA_NODE_OP_EQ,
    ENA_NODE_CLASS,
    ENA_NODE_PROP,

    ENA_NODE_MAX_NUM // Must be last one.
};

struct ena_node {
    enum ena_node_type type;
    struct ena_token *token;
    struct ena_node *child;
    int num_childs;
};

struct ena_ast {
    struct ena_node *tree;
};

struct ena_vm;
struct ena_ast *ena_parse(struct ena_vm *vm, const char *script);
void ena_destroy_ast(struct ena_ast *ast);
const char *ena_get_node_name(enum ena_node_type type);
void ena_dump_node(struct ena_node *node);

#endif
