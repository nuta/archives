#include "makestack.h"


void loop() {
    publish("hello", "built at " __TIME__);
    delay(3000);
}

void setup() {
}
