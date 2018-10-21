#include <stdlib.h>
#include <stdio.h>
#include <ena.h>


int main(int argc, char **argv) {
    struct ena_vm *vm = ena_create_vm();

    ena_value_t main_module = ena_create_module(vm);
    ena_register_module(vm, "main", main_module);

    const char *script =
    "var str = \"Hello World!\";"
    "print(str);"
    ;

    if (!ena_eval(vm, main_module, "(main)", script)) {
        fprintf(stderr, "%s\n", ena_get_error_cstr(vm));
        return 1;
    }

    return 0;
}
