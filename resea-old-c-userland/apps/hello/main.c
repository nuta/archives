#include <app.h>
#include <idl_stubs.h>

int main(void) {
    api_console_write("Hello World!\n");
    call_api_exit_app(1 /* appmgr */, 0);
    return 0;
}
