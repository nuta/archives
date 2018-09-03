#ifndef __ENA_PARSER_H__
#define __ENA_PARSER_H__

#include "internal.h"

struct ena_token {
    // The type of token.
    ena_token_type_t type;
    // The token string (terminated by NUL).
    char *str;
    // The position in `script`.
    int line;
};

typedef enum {
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
    ENA_NODE_NULL,
    ENA_NODE_INT_LIT,
    ENA_NODE_STRING_LIT,
    ENA_NODE_ID,
    ENA_NODE_OP_NOT,
    ENA_NODE_OP_OR,
    ENA_NODE_OP_AND,
    ENA_NODE_OP_ASSIGN,
    ENA_NODE_OP_ADD,
    ENA_NODE_OP_SUB,
    ENA_NODE_OP_MUL,
    ENA_NODE_OP_DIV,
    ENA_NODE_OP_MOD,
    ENA_NODE_OP_EQ,
    ENA_NODE_OP_NEQ,
    ENA_NODE_OP_LT,
    ENA_NODE_OP_LTE,
    ENA_NODE_OP_GT,
    ENA_NODE_OP_GTE,
    ENA_NODE_CLASS,
    ENA_NODE_PROP,
    ENA_NODE_LIST_LIT,
    ENA_NODE_MAP_LIT,
    ENA_NODE_MAP_ENTRY,
    ENA_NODE_INDEX,

    ENA_NODE_MAX_NUM // Must be last one.
} ena_node_type_t;

struct ena_node {
    ena_node_type_t type;
    struct ena_token *token;
    struct ena_node *child;
    int num_childs;
    int literal;
    int lineno;
};

struct ena_ast {
    struct ena_ast *next;
    struct ena_node *tree;
    const char *script;
};

struct ena_vm;
struct ena_ast *ena_parse(struct ena_vm *vm, const char *filepath, const char *script);
void ena_destroy_ast(struct ena_ast *ast);
const char *ena_get_node_name(ena_node_type_t type);
void ena_dump_node(struct ena_node *node);

#endif
