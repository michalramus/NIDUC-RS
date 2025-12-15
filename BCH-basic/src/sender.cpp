#include <Arduino.h>

#include "bch.hpp"

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    Serial.println("\n=== BCH Encoder Demo (Sender) ===\n");

    // Create BCH encoder for GF(2^4) with t=2 error correction
    BCHEncoder encoder(4, 2);  // m=4, t=2

    // Initialize the encoder
    if (!encoder.initialize()) {
        Serial.println("Failed to initialize BCH encoder");
        return;
    }

    // Print code information
    encoder.printCodeInfo();

    // Example message (k bits)
    int k = encoder.getK();
    std::vector<uint8_t> message(k, 0);

    // Create a test message: alternating pattern
    Serial.print("Original message (");
    Serial.print(k);
    Serial.println(" bits):");
    Serial.print("  Binary: ");
    for (int i = 0; i < k; i++) {
        message[i] = (i % 3 == 0) ? 1 : 0;  // Pattern: 100100100...
        Serial.print((int)message[i]);
    }
    Serial.println();

    // Encode the message
    Serial.println("\nEncoding...");
    std::vector<uint8_t> codeword = encoder.encode(message);

    if (codeword.empty()) {
        Serial.println("Encoding failed!");
        return;
    }

    // Display the encoded codeword
    Serial.print("Encoded codeword (");
    Serial.print(encoder.getN());
    Serial.println(" bits):");
    Serial.print("  Binary: ");
    for (size_t i = 0; i < codeword.size(); i++) {
        Serial.print((int)codeword[i]);
        if ((i + 1) % 5 == 0 && i < codeword.size() - 1) {
            Serial.print(" ");  // Space every 5 bits for readability
        }
    }
    Serial.println();

    // Show parity bits and message bits separately
    int parityBits = encoder.getN() - k;
    Serial.print("\n  Parity bits (");
    Serial.print(parityBits);
    Serial.print("): ");
    for (int i = 0; i < parityBits; i++) {
        Serial.print((int)codeword[i]);
    }
    Serial.println();

    Serial.print("  Message bits (");
    Serial.print(k);
    Serial.print("): ");
    for (int i = parityBits; i < encoder.getN(); i++) {
        Serial.print((int)codeword[i]);
    }
    Serial.println();

    Serial.println("\n=== Encoding Complete ===");
}

void loop() {
    // Nothing to do in loop
    delay(1000);
}
