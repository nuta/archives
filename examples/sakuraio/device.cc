#include "makestack.h"


void loop() {
    publish("hello", "built at " __TIME__);
    digitalWrite(13, HIGH);
    delay(3000);
}

void setup() {
    pinMode(13, OUTPUT);
}
