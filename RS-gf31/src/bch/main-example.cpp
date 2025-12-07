#include <Arduino.h>
#include "BCH_encoder.hpp"

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== BCH(31,21) Encoder ===");
    
    bch_init();
    
    // Przykładowe dane (21 bitów)
    uint32_t data = 0b101101110010110101011;  // 21 bitów
    
    Serial.print("Dane wejściowe (21 bit): ");
    Serial.println(data, BIN);
    
    uint32_t codeword = bch_encode(data);
    
    Serial.print("Słowo kodowe (31 bit):   ");
    for (int i = 30; i >= 0; i--) {
        Serial.print((codeword >> i) & 1);
        if (i % 8 == 0 && i != 0) Serial.print(" ");
    }
    Serial.println();
    
    Serial.print("Hex: 0x");
    Serial.println(codeword, HEX);
}

void loop() {
    // Nic
}