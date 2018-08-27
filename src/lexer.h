#ifndef __ENA_LEXER_H__
#define __ENA_LEXER_H__

enum ena_token_type {
    ENA_TOKEN_UNKNOWN,         // Used internally.
    ENA_TOKEN_EOF,
    ENA_TOKEN_ID,              // eg. qsort, tmp, index
    ENA_TOKEN_INT_LIT,         // eg. 123, 938129
    ENA_TOKEN_STRING_LIT,      // eg. "Hello, World!"
    ENA_TOKEN_PLUS,            // +
    ENA_TOKEN_MINUS,           // -
    ENA_TOKEN_ASTERISK,        // *
    ENA_TOKEN_SLASH,           // /
    ENA_TOKEN_DOUBLECOLON,     // :
    ENA_TOKEN_SEMICOLON,       // ;
    ENA_TOKEN_LPAREN,          // (
    ENA_TOKEN_RPAREN,          // )
    ENA_TOKEN_LBRACKET,        // [
    ENA_TOKEN_RBRACKET,        // ]
    ENA_TOKEN_LBRACE,          // {
    ENA_TOKEN_RBRACE,          // }
    ENA_TOKEN_DOT,             // .
    ENA_TOKEN_COMMA,           // ,
    ENA_TOKEN_EQ,              // =
    ENA_TOKEN_DOUBLE_EQ,       // ==
    ENA_TOKEN_LT,              // <
    ENA_TOKEN_VAR,             // var
    ENA_TOKEN_FUNC,            // func
    ENA_TOKEN_WHILE,           // while
    ENA_TOKEN_CONTINUE,        // continue
    ENA_TOKEN_BREAK,           // break
    ENA_TOKEN_IF,              // if
    ENA_TOKEN_ELSE,            // else
    ENA_TOKEN_RETURN,          // return
    ENA_TOKEN_TRUE,            // true
    ENA_TOKEN_FALSE,           // false
    ENA_TOKEN_CLASS,           // class

    ENA_TOKEN_MAX_NUM // Must be last one.
};

struct ena_lexer {
    // The next character offset.
    size_t next_pos;
    // The position of `nextc` in `script`.
    int current_line, current_column;
    // The script string (terminated by NULL).
    const char *script;
};

#define REMAINING_TOKEN() DEBUG("REMAINING: %s", &lexer.script[lexer.next_pos])

struct ena_token;
struct ena_vm;
const char *ena_get_token_name(enum ena_token_type type);
void ena_dump_tokens(struct ena_vm *vm, const char *script);
char ena_get_next_char(struct ena_vm *vm);
void ena_pushback_token(struct ena_vm *vm, struct ena_token *token);
struct ena_token *ena_get_next_token(struct ena_vm *vm);
void ena_skip_tokens(struct ena_vm *vm, int num);
enum ena_token_type ena_fetch_next_token_type(struct ena_vm *vm);
struct ena_token *ena_fetch_next_token(struct ena_vm *vm);
struct ena_token *ena_expect_token(struct ena_vm *vm, enum ena_token_type expected_type);
void ena_destroy_token(struct ena_token *token);

#endif
