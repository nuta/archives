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

int file_main(FILE *f) {
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

#ifdef ENA_DEBUG_BUILD
    DEBUG("Lexer:");
    ena_dump_tokens(vm, script);

    DEBUG("\nParser:");
    ena_dump_node(ena_parse(vm, script)->tree);

    DEBUG("\nEvaluate:");
    if (!ena_eval(vm, script)) {
        fprintf(stderr, "%s", ena_get_error_cstr(vm));
        return 1;
    }

    DEBUG("\nResult:");
    char buf[64];
    ena_stringify(buf, sizeof(buf), get_var_value(vm->current_scope, ena_cstr2ident(vm, "ans")));
    DEBUG("%s", buf);
#else
    if (!ena_eval(vm, script)) {
        fprintf(stderr, "%s", ena_get_error_cstr(vm));
        return 1;
    }
#endif

    ena_destroy_vm(vm);
    return 0;
}

int repl_main(void) {
    struct ena_vm *vm = ena_create_vm();
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
        if (!ena_eval(vm, script)) {
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
            return file_main(stdin);
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

        return file_main(f);
    }

    /* UNREACHABLE */
    return 1;
}
