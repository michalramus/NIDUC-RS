#include <Arduino.h>

#include "bch.hpp"

bool printFailures = true;  // Set to false to only show summary

// Statistics structure
struct TestStats {
    int singleBit_correctedCorrectly = 0;
    int singleBit_correctedIncorrectly = 0;
    int singleBit_detectedOnly = 0;
    int singleBit_undetected = 0;

    int doubleBit_correctedCorrectly = 0;
    int doubleBit_correctedIncorrectly = 0;
    int doubleBit_detectedOnly = 0;
    int doubleBit_undetected = 0;

    int tripleBit_correctedCorrectly = 0;
    int tripleBit_correctedIncorrectly = 0;
    int tripleBit_detectedOnly = 0;
    int tripleBit_undetected = 0;

    int totalMessages = 0;
    int totalSingleBitTests = 0;
    int totalDoubleBitTests = 0;
    int totalTripleBitTests = 0;
};

// Convert vector to string for printing
String vectorToString(const std::vector<uint8_t>& vec) {
    String result = "";
    for (size_t i = 0; i < vec.size(); i++) {
        result += String((int)vec[i]);
    }
    return result;
}

// Generate all k-bit messages
void generateMessage(int k, int index, std::vector<uint8_t>& message) {
    for (int i = 0; i < k; i++) {
        message[i] = (index >> i) & 1;
    }
}

// Test single bit error
void testSingleBitError(BCHEncoder& encoder, BCHDecoder& decoder,
                        const std::vector<uint8_t>& message,
                        const std::vector<uint8_t>& codeword, int errorPos,
                        TestStats& stats, bool printFailures) {
    // Introduce single bit error
    std::vector<uint8_t> received = codeword;
    received[errorPos] ^= 1;

    // Try to decode
    std::vector<uint8_t> decoded;
    int errorsFound = decoder.decode(received, decoded);

    if (errorsFound < 0) {
        // Decoding failed - detected but not corrected
        stats.singleBit_detectedOnly++;
    } else if (decoded == message) {
        // Correctly corrected
        stats.singleBit_correctedCorrectly++;
    } else {
        // Incorrectly corrected
        stats.singleBit_correctedIncorrectly++;

        if (printFailures) {
            Serial.print("SINGLE BIT ERROR - INCORRECT CORRECTION:\n");
            Serial.print("  Message:  ");
            Serial.println(vectorToString(message));
            Serial.print("  Codeword: ");
            Serial.println(vectorToString(codeword));
            Serial.print("  Error at position: ");
            Serial.println(errorPos);
            Serial.print("  Decoded:  ");
            Serial.println(vectorToString(decoded));
            Serial.println();
        }
    }

    stats.totalSingleBitTests++;
}

// Test double bit error
void testDoubleBitError(BCHEncoder& encoder, BCHDecoder& decoder,
                        const std::vector<uint8_t>& message,
                        const std::vector<uint8_t>& codeword, int errorPos1,
                        int errorPos2, TestStats& stats, bool printFailures) {
    // Introduce double bit error
    std::vector<uint8_t> received = codeword;
    received[errorPos1] ^= 1;
    received[errorPos2] ^= 1;

    // Try to decode
    std::vector<uint8_t> decoded;
    int errorsFound = decoder.decode(received, decoded);

    if (errorsFound < 0) {
        // Decoding failed - detected but not corrected
        stats.doubleBit_detectedOnly++;
    } else if (decoded == message) {
        // Correctly corrected
        stats.doubleBit_correctedCorrectly++;
    } else {
        // Incorrectly corrected
        stats.doubleBit_correctedIncorrectly++;

        if (printFailures) {
            Serial.print("DOUBLE BIT ERROR - INCORRECT CORRECTION:\n");
            Serial.print("  Message:  ");
            Serial.println(vectorToString(message));
            Serial.print("  Codeword: ");
            Serial.println(vectorToString(codeword));
            Serial.print("  Errors at positions: ");
            Serial.print(errorPos1);
            Serial.print(", ");
            Serial.println(errorPos2);
            Serial.print("  Decoded:  ");
            Serial.println(vectorToString(decoded));
            Serial.println();
        }
    }

    stats.totalDoubleBitTests++;
}

// Test triple bit error
void testTripleBitError(BCHEncoder& encoder, BCHDecoder& decoder,
                        const std::vector<uint8_t>& message,
                        const std::vector<uint8_t>& codeword, int errorPos1,
                        int errorPos2, int errorPos3, TestStats& stats,
                        bool printFailures) {
    // Introduce triple bit error
    std::vector<uint8_t> received = codeword;
    received[errorPos1] ^= 1;
    received[errorPos2] ^= 1;
    received[errorPos3] ^= 1;

    // Try to decode
    std::vector<uint8_t> decoded;
    int errorsFound = decoder.decode(received, decoded);

    if (errorsFound < 0) {
        // Decoding failed - detected but not corrected
        stats.tripleBit_detectedOnly++;
    } else if (decoded == message) {
        // Correctly corrected
        stats.tripleBit_correctedCorrectly++;
    } else {
        // Incorrectly corrected
        stats.tripleBit_correctedIncorrectly++;

        if (printFailures) {
            Serial.print("TRIPLE BIT ERROR - INCORRECT CORRECTION:\n");
            Serial.print("  Message:  ");
            Serial.println(vectorToString(message));
            Serial.print("  Codeword: ");
            Serial.println(vectorToString(codeword));
            Serial.print("  Errors at positions: ");
            Serial.print(errorPos1);
            Serial.print(", ");
            Serial.print(errorPos2);
            Serial.print(", ");
            Serial.println(errorPos3);
            Serial.print("  Decoded:  ");
            Serial.println(vectorToString(decoded));
            Serial.println();
        }
    }

    stats.totalTripleBitTests++;
}

void printSummary(const TestStats& stats) {
    Serial.println("\n========================================");
    Serial.println("           TEST SUMMARY");
    Serial.println("========================================");

    Serial.print("Total messages tested: ");
    Serial.println(stats.totalMessages);
    Serial.println();

    // Single bit errors
    Serial.println("--- SINGLE BIT ERRORS ---");
    Serial.print("Total tests: ");
    Serial.println(stats.totalSingleBitTests);
    Serial.print("  Corrected correctly:   ");
    Serial.print(stats.singleBit_correctedCorrectly);
    Serial.print(" (");
    Serial.print(
        100.0 * stats.singleBit_correctedCorrectly / stats.totalSingleBitTests,
        2);
    Serial.println("%)");
    Serial.print("  Corrected incorrectly: ");
    Serial.print(stats.singleBit_correctedIncorrectly);
    Serial.print(" (");
    Serial.print(100.0 * stats.singleBit_correctedIncorrectly /
                     stats.totalSingleBitTests,
                 2);
    Serial.println("%)");
    Serial.print("  Detected only:         ");
    Serial.print(stats.singleBit_detectedOnly);
    Serial.print(" (");
    Serial.print(
        100.0 * stats.singleBit_detectedOnly / stats.totalSingleBitTests, 2);
    Serial.println("%)");
    Serial.print("  Undetected:            ");
    Serial.print(stats.singleBit_undetected);
    Serial.print(" (");
    Serial.print(100.0 * stats.singleBit_undetected / stats.totalSingleBitTests,
                 2);
    Serial.println("%)");
    Serial.println();

    // Double bit errors
    Serial.println("--- DOUBLE BIT ERRORS ---");
    Serial.print("Total tests: ");
    Serial.println(stats.totalDoubleBitTests);
    Serial.print("  Corrected correctly:   ");
    Serial.print(stats.doubleBit_correctedCorrectly);
    Serial.print(" (");
    Serial.print(
        100.0 * stats.doubleBit_correctedCorrectly / stats.totalDoubleBitTests,
        2);
    Serial.println("%)");
    Serial.print("  Corrected incorrectly: ");
    Serial.print(stats.doubleBit_correctedIncorrectly);
    Serial.print(" (");
    Serial.print(100.0 * stats.doubleBit_correctedIncorrectly /
                     stats.totalDoubleBitTests,
                 2);
    Serial.println("%)");
    Serial.print("  Detected only:         ");
    Serial.print(stats.doubleBit_detectedOnly);
    Serial.print(" (");
    Serial.print(
        100.0 * stats.doubleBit_detectedOnly / stats.totalDoubleBitTests, 2);
    Serial.println("%)");
    Serial.print("  Undetected:            ");
    Serial.print(stats.doubleBit_undetected);
    Serial.print(" (");
    Serial.print(100.0 * stats.doubleBit_undetected / stats.totalDoubleBitTests,
                 2);
    Serial.println("%)");
    Serial.println();

    // Triple bit errors
    Serial.println("--- TRIPLE BIT ERRORS ---");
    Serial.print("Total tests: ");
    Serial.println(stats.totalTripleBitTests);
    Serial.print("  Corrected correctly:   ");
    Serial.print(stats.tripleBit_correctedCorrectly);
    Serial.print(" (");
    Serial.print(
        100.0 * stats.tripleBit_correctedCorrectly / stats.totalTripleBitTests,
        2);
    Serial.println("%)");
    Serial.print("  Corrected incorrectly: ");
    Serial.print(stats.tripleBit_correctedIncorrectly);
    Serial.print(" (");
    Serial.print(100.0 * stats.tripleBit_correctedIncorrectly /
                     stats.totalTripleBitTests,
                 2);
    Serial.println("%)");
    Serial.print("  Detected only:         ");
    Serial.print(stats.tripleBit_detectedOnly);
    Serial.print(" (");
    Serial.print(
        100.0 * stats.tripleBit_detectedOnly / stats.totalTripleBitTests, 2);
    Serial.println("%)");
    Serial.print("  Undetected:            ");
    Serial.print(stats.tripleBit_undetected);
    Serial.print(" (");
    Serial.print(100.0 * stats.tripleBit_undetected / stats.totalTripleBitTests,
                 2);
    Serial.println("%)");

    Serial.println("========================================\n");
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    Serial.println("\n=== BCH Complete Error Correction Test ===\n");

    // Create BCH encoder for GF(2^4) with t=2 error correction
    BCHEncoder encoder(4, 2);  // m=4, t=2

    // Initialize the encoder
    if (!encoder.initialize()) {
        Serial.println("Failed to initialize BCH encoder");
        return;
    }

    encoder.printCodeInfo();

    // Create decoder
    BCHDecoder decoder(encoder);

    int k = encoder.getK();      // Message length
    int n = encoder.getN();      // Codeword length
    int totalMessages = 1 << k;  // 2^k possible messages

    Serial.print("Testing all ");
    Serial.print(totalMessages);
    Serial.print(" possible ");
    Serial.print(k);
    Serial.println("-bit messages");
    Serial.print("Each codeword is ");
    Serial.print(n);
    Serial.println(" bits long\n");

    Serial.print("Single bit error tests per message: ");
    Serial.println(n);
    Serial.print("Double bit error tests per message: ");
    Serial.println(n * (n - 1) / 2);
    Serial.print("Triple bit error tests per message: ");
    Serial.println(n * (n - 1) * (n - 2) / 6);
    Serial.println();

    

    TestStats stats;

    Serial.println("Starting comprehensive test...\n");
    unsigned long startTime = millis();

    // Test all possible messages
    for (int msgIndex = 0; msgIndex < totalMessages; msgIndex++) {
        std::vector<uint8_t> message(k);
        generateMessage(k, msgIndex, message);

        // Encode the message
        std::vector<uint8_t> codeword = encoder.encode(message);

        if (codeword.empty()) {
            Serial.println("Encoding failed!");
            continue;
        }

        stats.totalMessages++;

        // Progress indicator and yield to watchdog every message
        if (msgIndex % 8 == 0) {
            Serial.print("Testing message ");
            Serial.print(msgIndex);
            Serial.print("/");
            Serial.print(totalMessages);
            Serial.println("...");
            yield();  // Feed the watchdog timer
        }

        // Test all single bit errors
        for (int errorPos = 0; errorPos < n; errorPos++) {
            testSingleBitError(encoder, decoder, message, codeword, errorPos,
                               stats, printFailures);
        }

        // Test all double bit errors
        for (int errorPos1 = 0; errorPos1 < n; errorPos1++) {
            for (int errorPos2 = errorPos1 + 1; errorPos2 < n; errorPos2++) {
                testDoubleBitError(encoder, decoder, message, codeword,
                                   errorPos1, errorPos2, stats, printFailures);
            }
        }

        // Test all triple bit errors
        for (int errorPos1 = 0; errorPos1 < n; errorPos1++) {
            for (int errorPos2 = errorPos1 + 1; errorPos2 < n; errorPos2++) {
                for (int errorPos3 = errorPos2 + 1; errorPos3 < n;
                     errorPos3++) {
                    testTripleBitError(encoder, decoder, message, codeword,
                                       errorPos1, errorPos2, errorPos3, stats,
                                       printFailures);
                }
            }
        }

        // Yield to watchdog after each message test
        yield();
    }

    unsigned long endTime = millis();
    unsigned long duration = endTime - startTime;

    Serial.println("\n=== Testing Complete ===");
    Serial.print("Total time: ");
    Serial.print(duration / 1000);
    Serial.print(".");
    Serial.print(duration % 1000);
    Serial.println(" seconds\n");

    printSummary(stats);
}

void loop() {
    // Nothing to do in loop
    delay(1000);
}
