#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ena.h>
#include "lexer.h"
#include "parser.h"

#ifdef ENA_WITH_TEST
int ena_unittests(void);
#endif

int failed = 0;
static ena_value_t assert_eq_handler(UNUSED struct ena_vm *vm, ena_value_t *args, int num_args) {
    ena_check_args(vm, "assert_eq()", "xx", args, num_args);
    ena_value_t x = args[0];
    ena_value_t y = args[1];
    if (!ena_is_equal(x, y)) {
        char buf_x[64], buf_y[64];
        ena_stringify(buf_x, sizeof(buf_x), x);
        ena_stringify(buf_y, sizeof(buf_y), y);
        fprintf(stderr, "assert_eq: %s != %s\n", buf_x, buf_y);
        failed++;
        return ENA_NULL;
    }

    return ENA_NULL;
}

static int file_main(const char *filepath, FILE *f) {
    size_t script_len = 0;
    char *script = realloc(NULL, 1);
    int ch;
    while ((ch = getc(f)) != EOF) {
        script = (char *) realloc(script, script_len + 1);
        script[script_len] = ch;
        script_len++;
    }
    script[script_len - 1] = '\0';

    struct ena_vm *vm = ena_create_vm();
    ena_value_t main_module = ena_create_module(vm);
#ifdef ENA_DEBUG_BUILD
    DEBUG("Lexer:");
    ena_dump_tokens(vm, script);

    DEBUG("\nParser:");
    struct ena_ast *ast = ena_parse(vm, filepath, script);
    if (!ast) {
        fprintf(stderr, "%s", ena_get_error_cstr(vm));
        free(script);
        return 1;
    }
    ena_dump_node(ast->tree);
    ena_destroy_ast(ast);
    ena_value_t assert_eq = ena_create_func(vm, assert_eq_handler);
    ena_add_to_module(vm, main_module, "assert_eq", assert_eq);
    ena_register_module(vm, "main", main_module);

    DEBUG("\nEvaluate:");
    if (!ena_eval(vm, main_module, filepath, script)) {
        fprintf(stderr, "%s", ena_get_error_cstr(vm));
        return 1;
    }
#else
    ena_register_module(vm, "main", main_module);
    if (!ena_eval(vm, main_module, filepath, script)) {
        fprintf(stderr, "%s", ena_get_error_cstr(vm));
        return 1;
    }
#endif

    ena_destroy_vm(vm);
    free(script);
    fclose(f);

    if (failed > 0) {
        return 1;
    }

    return 0;
}

static int repl_main(void) {
    struct ena_vm *vm = ena_create_vm();
    ena_value_t main_module = ena_create_module(vm);
    ena_register_module(vm, "main", main_module);
    for (;;) {
        printf(">>> ");
        fflush(stdout);

        // Read a line.
        char *script = realloc(NULL, 1);
        size_t script_len = 0;
        int ch;
        for (;;) {
            ch = getchar();
            if (ch == '\n' || ch == EOF) {
                break;
            }

            script = (char *) realloc(script, script_len + 1);
            script[script_len] = ch;
            script_len++;
        }

        script[script_len - 1] = '\0';
        if (!ena_eval(vm, main_module, "(stdin)", script)) {
            fprintf(stderr, "%s", ena_get_error_cstr(vm));
            return 1;
        }

        if (ch == EOF) {
            return 0;
        }

        free(script);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        if (isatty(0)) {
            // $ ena
            return repl_main();
        } else {
            // $ cat program.ena | ena
            return file_main("(stdin)", stdin);
        }
    } else {
#ifdef ENA_WITH_TEST
        if (!strcmp(argv[1], "--test")) {
            if (ena_unittests() != 0) {
                return 2;
            }

            return 0;
        }
#endif

        if (!strcmp(argv[1], "-v")) {
            printf("%s\n", ENA_VERSION);
            return 0;
        }

        FILE *f = fopen(argv[1], "r");
        if (!f) {
            perror("ena");
            return 1;
        }

        return file_main(argv[1], f);
    }

    /* UNREACHABLE */
    return 1;
}
