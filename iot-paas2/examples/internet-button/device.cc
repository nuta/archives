#include "makestack.h"

#define BUTTON_PIN 12
int state = 0;

void loop() {
    int new_state = digitalRead(BUTTON_PIN);
    if (new_state != state) {
        state = new_state;

        if (new_state == 1) {
            publish("button", "pressed");
        }
    }

    delay(10);
}

void setup() {
    printf("Starting the Internet button!\n");
    pinMode(BUTTON_PIN, INPUT);
}
