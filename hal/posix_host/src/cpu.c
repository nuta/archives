#include <hal.h>
void exit(int);

void hal_panic(void) {

    exit(0);
}
