#include <Arduino.h>
#include "bch.hpp"

// Test message: 7 bits
uint8_t message[BCH_K] = {1, 0, 1, 1, 0, 0, 1};

void transmitCorrect() {
    Serial.println("CORRECT");
    
    uint8_t codeword[BCH_N];
    BCH::encode(message, codeword);
    
    Serial.print("Message:  ");
    BCH::printBinary(message, BCH_K);
    Serial.println();
    
    Serial.print("Codeword: ");
    BCH::printBinary(codeword, BCH_N);
    Serial.println();
    
    // Transmit codeword
    for (int i = 0; i < BCH_N; i++) {
        Serial.println(codeword[i]);
    }
    Serial.println();
}

void transmitOneError() {
    Serial.println("ONE_ERROR");
    
    uint8_t codeword[BCH_N];
    BCH::encode(message, codeword);
    
    // Introduce error at position 5
    codeword[5] ^= 1;
    
    Serial.print("Message:  ");
    BCH::printBinary(message, BCH_K);
    Serial.println();
    
    Serial.print("Corrupted: ");
    BCH::printBinary(codeword, BCH_N);
    Serial.print(" (error at position 5)");
    Serial.println();
    
    // Transmit corrupted codeword
    for (int i = 0; i < BCH_N; i++) {
        Serial.println(codeword[i]);
    }
    Serial.println();
}

void transmitTwoErrors() {
    Serial.println("TWO_ERRORS");
    
    uint8_t codeword[BCH_N];
    BCH::encode(message, codeword);
    
    // Introduce errors at positions 3 and 10
    codeword[3] ^= 1;
    codeword[10] ^= 1;
    
    Serial.print("Message:  ");
    BCH::printBinary(message, BCH_K);
    Serial.println();
    
    Serial.print("Corrupted: ");
    BCH::printBinary(codeword, BCH_N);
    Serial.print(" (errors at positions 3, 10)");
    Serial.println();
    
    // Transmit corrupted codeword
    for (int i = 0; i < BCH_N; i++) {
        Serial.println(codeword[i]);
    }
    Serial.println();
}

void transmitMoreErrors() {
    Serial.println("THREE_ERRORS");
    
    uint8_t codeword[BCH_N];
    BCH::encode(message, codeword);
    
    // Introduce errors at positions 2, 7, and 12 (beyond correction capability)
    codeword[2] ^= 1;
    codeword[7] ^= 1;
    codeword[12] ^= 1;
    
    Serial.print("Message:  ");
    BCH::printBinary(message, BCH_K);
    Serial.println();
    
    Serial.print("Corrupted: ");
    BCH::printBinary(codeword, BCH_N);
    Serial.print(" (errors at positions 2, 7, 12)");
    Serial.println();
    
    // Transmit corrupted codeword
    for (int i = 0; i < BCH_N; i++) {
        Serial.println(codeword[i]);
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== BCH SENDER ===");
    Serial.println("BCH(15,7,2) - can correct up to 2 errors");
    Serial.println();
}

void loop() {
    delay(3000);
    transmitCorrect();
    
    delay(3000);
    transmitOneError();
    
    delay(3000);
    transmitTwoErrors();
    
    delay(3000);
    transmitMoreErrors();
}
