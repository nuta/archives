#include <resea.h>
#include <resea/logging.h>
#include <resea/exit.h>

void main(void) {
    call_logging_emit(1, (char *) "Hello A!", 8);
    call_logging_emit(1, (char *) "Hello B!", 8);
    call_logging_emit(1, (char *) "Hello C!", 8);
    call_logging_emit(1, (char *) "Hello D!", 8);
    printf("Hi there %d!!!", 123456789);
    call_exit_exit(1, ERROR_NONE);
}
