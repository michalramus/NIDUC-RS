#include <Arduino.h>

double x[6] = {0, 1, 2, 3, 4, 5};
double y[6] = {2, 1, -4, -7, -2, 17};

void transmitCorrect() {}

void transmitOneError() {
    for (int i = 0; i < 6; i++) {
        Serial.print(y[i]);
    }
    Serial.println();
}

void transmitMoreErrors() {}

void transmit() {
    delay(1000);
    transmitCorrect();
    delay(1000);
    transmitOneError();
    delay(1000);
    transmitMoreErrors();
}
