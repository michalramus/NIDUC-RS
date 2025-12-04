#include <Arduino.h>
#include "bch.hpp"

void setup() {
    Serial.begin(115200);
    while (!Serial);
    delay(1000);
    
    Serial.println("=== BCH RECEIVER ===");
    Serial.println("Waiting for data...");
    Serial.println();
}

void loop() {
    // Receiver implementation will be added later
    // This will receive BCH codewords and perform decoding/error correction
    
    delay(1000);
}
