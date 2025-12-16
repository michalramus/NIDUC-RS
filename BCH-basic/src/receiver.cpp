#include <Arduino.h>

#include "bch.hpp"

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    Serial.println("\n=== BCH Decoder Demo (Receiver) ===\n");

    // Create BCH encoder and decoder for GF(2^4) with t=2 error correction
    BCHEncoder encoder(4, 2);  // m=4, t=2

    // Initialize the encoder
    if (!encoder.initialize()) {
        Serial.println("Failed to initialize BCH encoder");
        return;
    }

    encoder.printCodeInfo();

    // Create decoder
    BCHDecoder decoder(encoder);

    // Example: Encode a message
    int k = encoder.getK();
    int n = encoder.getN();
    std::vector<uint8_t> message(k, 0);

    // Create test message
    Serial.print("\nOriginal message (");
    Serial.print(k);
    Serial.println(" bits):");
    Serial.print("  Binary: ");
    for (int i = 0; i < k; i++) {
        message[i] = (i % 3 == 0) ? 1 : 0;
        Serial.print((int)message[i]);
    }
    Serial.println();

    // Encode
    std::vector<uint8_t> codeword = encoder.encode(message);

    Serial.print("\nEncoded codeword (");
    Serial.print(n);
    Serial.println(" bits):");
    Serial.print("  Binary: ");
    for (size_t i = 0; i < codeword.size(); i++) {
        Serial.print((int)codeword[i]);
    }
    Serial.println("\n");

    // ========== Test Case 1: No errors ==========
    Serial.println("--- Test 1: No Errors ---");
    std::vector<uint8_t> received1 = codeword;
    std::vector<uint8_t> decoded1;

    int errors1 = decoder.decode(received1, decoded1);

    Serial.print("Errors corrected: ");
    Serial.println(errors1);
    Serial.print("Decoded message: ");
    for (size_t i = 0; i < decoded1.size(); i++) {
        Serial.print((int)decoded1[i]);
    }

    bool correct1 = (decoded1 == message);
    Serial.print("\nDecoding result: ");
    Serial.println(correct1 ? " CORRECT" : " INCORRECT");
    Serial.println();

    // ========== Test Case 2: Single error ==========
    Serial.println("--- Test 2: Single Error at Position 5 ---");
    std::vector<uint8_t> received2 = codeword;
    received2[5] ^= 1;  // Introduce error at position 5

    Serial.print("Received (corrupted): ");
    for (size_t i = 0; i < received2.size(); i++) {
        Serial.print((int)received2[i]);
    }
    Serial.println();

    std::vector<uint8_t> decoded2;
    int errors2 = decoder.decode(received2, decoded2);

    Serial.print("Errors corrected: ");
    Serial.println(errors2);
    Serial.print("Decoded message: ");
    for (size_t i = 0; i < decoded2.size(); i++) {
        Serial.print((int)decoded2[i]);
    }

    bool correct2 = (decoded2 == message);
    Serial.print("\nDecoding result: ");
    Serial.println(correct2 ? " CORRECT" : " INCORRECT");
    Serial.println();

    // ========== Test Case 3: Double error (maximum correctable) ==========
    Serial.println("--- Test 3: Double Error at Positions 3 and 10 ---");
    std::vector<uint8_t> received3 = codeword;
    received3[3] ^= 1;   // Error at position 3
    received3[10] ^= 1;  // Error at position 10

    Serial.print("Received (corrupted): ");
    for (size_t i = 0; i < received3.size(); i++) {
        Serial.print((int)received3[i]);
    }
    Serial.println();

    std::vector<uint8_t> decoded3;
    int errors3 = decoder.decode(received3, decoded3);

    Serial.print("Errors corrected: ");
    Serial.println(errors3);
    Serial.print("Decoded message: ");
    for (size_t i = 0; i < decoded3.size(); i++) {
        Serial.print((int)decoded3[i]);
    }

    bool correct3 = (decoded3 == message);
    Serial.print("\nDecoding result: ");
    Serial.println(correct3 ? " CORRECT" : " INCORRECT");
    Serial.println();

    // ========== Test Case 4: Triple error (uncorrectable) ==========
    Serial.println(
        "--- Test 4: Triple Error (Beyond Correction Capability) ---");
    std::vector<uint8_t> received4 = codeword;
    received4[2] ^= 1;   // Error at position 2
    received4[7] ^= 1;   // Error at position 7
    received4[12] ^= 1;  // Error at position 12

    Serial.print("Received (corrupted): ");
    for (size_t i = 0; i < received4.size(); i++) {
        Serial.print((int)received4[i]);
    }
    Serial.println();

    std::vector<uint8_t> decoded4;
    int errors4 = decoder.decode(received4, decoded4);

    if (errors4 < 0) {
        Serial.println("Decoding failed: too many errors");
    } else {
        Serial.print("Errors corrected: ");
        Serial.println(errors4);
        Serial.print("Decoded message: ");
        for (size_t i = 0; i < decoded4.size(); i++) {
            Serial.print((int)decoded4[i]);
        }
        bool correct4 = (decoded4 == message);
        Serial.print("\nDecoding result: ");
        Serial.println(correct4 ? " CORRECT" : " INCORRECT (expected)");
    }
    Serial.println();

    Serial.println("=== All Tests Complete ===");
}

void loop() { delay(1000); }
