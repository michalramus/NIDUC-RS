#include <Arduino.h>

#include "bch.hpp"

// BCH parameters - must match sender
#define BCH_N 15  // Codeword length
#define BCH_K 7   // Message length
#define BCH_T 2   // Error correction capability

// Create BCH instance
BCH bch(BCH_N, BCH_K, BCH_T);

// Statistics
struct TestStats {
    int totalTests;
    int correctMessages;
    int correctedMessages;
    int failedMessages;
    int errorsByCount[4];  // 0, 1, 2, 3 errors
    int correctedByCount[4];
    int failedByCount[4];
    int correctCorrections;    // Corrections that match original
    int incorrectCorrections;  // Corrections that don't match original
    unsigned long startTime;
    unsigned long endTime;
};

TestStats stats;
bool testingActive = false;

// Store example of incorrect correction
bool hasIncorrectExample = false;
uint8_t exampleOriginal[BCH_K];
uint8_t exampleReceived[BCH_N];
uint8_t exampleDecoded[BCH_K];
int exampleErrorCount = 0;

void resetStats() {
    stats.totalTests = 0;
    stats.correctMessages = 0;
    stats.correctedMessages = 0;
    stats.failedMessages = 0;
    stats.correctCorrections = 0;
    stats.incorrectCorrections = 0;
    for (int i = 0; i < 4; i++) {
        stats.errorsByCount[i] = 0;
        stats.correctedByCount[i] = 0;
        stats.failedByCount[i] = 0;
    }
    stats.startTime = millis();
    stats.endTime = 0;
    hasIncorrectExample = false;
}

void printStats() {
    unsigned long duration = (stats.endTime - stats.startTime) / 1000;

    Serial.println("\n\n");
    Serial.println("=======================================================");
    Serial.println("           TEST SUMMARY - 1000 MESSAGES");
    Serial.println("=======================================================");
    Serial.println();

    // Calculate percentages
    float clean_percent = (stats.correctMessages * 100.0) / stats.totalTests;
    float corrected_percent =
        (stats.correctedMessages * 100.0) / stats.totalTests;
    float failed_percent = (stats.failedMessages * 100.0) / stats.totalTests;
    float success_percent =
        ((stats.correctMessages + stats.correctedMessages) * 100.0) /
        stats.totalTests;

    // Results
    Serial.println("RESULTS:");
    Serial.print("  OK (no errors):        ");
    Serial.print(stats.correctMessages);
    Serial.print("/1000  (");
    Serial.print(clean_percent, 1);
    Serial.println("%)");

    Serial.print("  CORRECTED (with errors): ");
    Serial.print(stats.correctedMessages);
    Serial.print("/1000  (");
    Serial.print(corrected_percent, 1);
    Serial.println("%)");

    Serial.print("  DETECTED (uncorrectable): ");
    Serial.print(stats.failedMessages);
    Serial.print("/1000  (");
    Serial.print(failed_percent, 1);
    Serial.println("%)");

    Serial.println();
    Serial.print("SUCCESS RATE:            ");
    int successful = stats.correctMessages + stats.correctedMessages;
    Serial.print(successful);
    Serial.print("/1000  (");
    Serial.print(success_percent, 1);
    Serial.println("%)");

    // Breakdown by error count
    Serial.println();
    Serial.println("BREAKDOWN BY ERROR COUNT:");
    for (int i = 0; i < 4; i++) {
        if (stats.errorsByCount[i] > 0) {
            Serial.print("  ");
            Serial.print(i);
            Serial.print(" error(s): ");
            Serial.print(stats.errorsByCount[i]);
            Serial.print(" messages");
            Serial.println();

            // Corrected
            if (stats.correctedByCount[i] > 0) {
                Serial.print("    Corrected:   ");
                Serial.print(stats.correctedByCount[i]);
                Serial.print("/");
                Serial.print(stats.errorsByCount[i]);
                Serial.print("  (");
                Serial.print((stats.correctedByCount[i] * 100.0) /
                                 stats.errorsByCount[i],
                             1);
                Serial.println("%)");
            }

            // Failed
            if (stats.failedByCount[i] > 0) {
                Serial.print("    Failed:      ");
                Serial.print(stats.failedByCount[i]);
                Serial.print("/");
                Serial.print(stats.errorsByCount[i]);
                Serial.print("  (");
                Serial.print(
                    (stats.failedByCount[i] * 100.0) / stats.errorsByCount[i],
                    1);
                Serial.println("%)");
            }
        }
    }

    // Correction accuracy for messages with errors
    Serial.println();
    Serial.println("CORRECTION ACCURACY:");
    int totalWithErrors = stats.totalTests - stats.correctMessages;
    if (totalWithErrors > 0) {
        Serial.print("  Messages with errors:  ");
        Serial.print(totalWithErrors);
        Serial.println();

        Serial.print("  Successfully corrected: ");
        Serial.print(stats.correctedMessages);
        Serial.print("/");
        Serial.print(totalWithErrors);
        Serial.print("  (");
        Serial.print((stats.correctedMessages * 100.0) / totalWithErrors, 1);
        Serial.println("%)");

        Serial.print("  Failed to correct:     ");
        Serial.print(stats.failedMessages);
        Serial.print("/");
        Serial.print(totalWithErrors);
        Serial.print("  (");
        Serial.print((stats.failedMessages * 100.0) / totalWithErrors, 1);
        Serial.println("%)");

        // Breakdown of corrections
        if (stats.correctedMessages > 0) {
            Serial.println();
            Serial.print("  Correctly corrected:   ");
            Serial.print(stats.correctCorrections);
            Serial.print("/");
            Serial.print(stats.correctedMessages);
            Serial.print("  (");
            Serial.print(
                (stats.correctCorrections * 100.0) / stats.correctedMessages,
                1);
            Serial.println("%)");

            Serial.print("  Incorrectly corrected: ");
            Serial.print(stats.incorrectCorrections);
            Serial.print("/");
            Serial.print(stats.correctedMessages);
            Serial.print("  (");
            Serial.print(
                (stats.incorrectCorrections * 100.0) / stats.correctedMessages,
                1);
            Serial.println("%)");
        }
    } else {
        Serial.println("  No messages with errors received");
    }

    // Show example of incorrect correction
    if (hasIncorrectExample) {
        Serial.println();
        Serial.println("EXAMPLE OF INCORRECT CORRECTION:");
        Serial.print("  Original message:  [");
        for (int i = 0; i < BCH_K; i++) {
            Serial.print(exampleOriginal[i]);
            if (i < BCH_K - 1) Serial.print(", ");
        }
        Serial.println("]");

        Serial.print("  Received codeword: [");
        for (int i = 0; i < BCH_N; i++) {
            Serial.print(exampleReceived[i]);
            if (i < BCH_N - 1) Serial.print(", ");
        }
        Serial.print("]  (");
        Serial.print(exampleErrorCount);
        Serial.println(" errors)");

        Serial.print("  Decoded message:   [");
        for (int i = 0; i < BCH_K; i++) {
            Serial.print(exampleDecoded[i]);
            if (i < BCH_K - 1) Serial.print(", ");
        }
        Serial.println("]");
    }

    // Test info
    Serial.println();
    Serial.println("TEST INFO:");
    Serial.print("  Total test time: ");
    Serial.print(duration);
    Serial.println(" seconds");

    if (duration > 0) {
        Serial.print("  Speed: ");
        Serial.print(stats.totalTests / duration);
        Serial.println(" messages/s");
    }

    Serial.print("  BCH code: BCH(");
    Serial.print(bch.getN());
    Serial.print(",");
    Serial.print(bch.getK());
    Serial.print(",");
    Serial.print(bch.getT());
    Serial.print(") - can correct up to ");
    Serial.print(bch.getT());
    Serial.println(" errors");

    Serial.println();
    Serial.println("=======================================================");
    Serial.println("Waiting for reset...");
    Serial.println();
}

bool compareMessages(uint8_t* msg1, uint8_t* msg2, int length) {
    for (int i = 0; i < length; i++) {
        if (msg1[i] != msg2[i]) return false;
    }
    return true;
}

void processTest(int testNum, int errorCount, uint8_t* originalMessage,
                 uint8_t* received) {
    uint8_t decoded[BCH_K];
    bool success = bch.decode(received, decoded);

    // Verify if decoded message matches original
    bool messageCorrect = compareMessages(originalMessage, decoded, BCH_K);

    stats.totalTests++;
    stats.errorsByCount[errorCount]++;

    if (errorCount == 0 && messageCorrect) {
        stats.correctMessages++;
        stats.correctedByCount[0]++;
    } else if (messageCorrect) {
        stats.correctedMessages++;
        stats.correctedByCount[errorCount]++;
        stats.correctCorrections++;
    } else {
        // Check if decoder attempted correction but got wrong result
        if (success && errorCount > 0) {
            stats.correctedMessages++;
            stats.correctedByCount[errorCount]++;
            stats.incorrectCorrections++;

            // Store first example of incorrect correction
            if (!hasIncorrectExample) {
                hasIncorrectExample = true;
                for (int i = 0; i < BCH_K; i++) {
                    exampleOriginal[i] = originalMessage[i];
                    exampleDecoded[i] = decoded[i];
                }
                for (int i = 0; i < BCH_N; i++) {
                    exampleReceived[i] = received[i];
                }
                exampleErrorCount = errorCount;
            }
        } else {
            stats.failedMessages++;
            stats.failedByCount[errorCount]++;
        }
    }

    // Optional: print individual test results (comment out for faster tests)
    if (testNum % 100 == 0 || !messageCorrect) {
        Serial.print("Test ");
        Serial.print(testNum);
        Serial.print(": ");
        Serial.print(errorCount);
        Serial.print(" error(s) - ");
        if (messageCorrect) {
            if (errorCount == 0) {
                Serial.println("CORRECT");
            } else {
                Serial.println("CORRECTED OK");
            }
        } else {
            if (success && errorCount > 0) {
                Serial.println("CORRECTED WRONG");
            } else {
                Serial.println("FAILED");
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    delay(1000);

    Serial.println("=======================================================");
    Serial.println("       BCH RECEIVER - 1000 MESSAGE TEST");
    Serial.println("=======================================================");
    Serial.println();
    Serial.print("BCH(");
    Serial.print(bch.getN());
    Serial.print(",");
    Serial.print(bch.getK());
    Serial.print(",");
    Serial.print(bch.getT());
    Serial.println(") configured");
    Serial.println();
    Serial.println("Receiver functions:");
    Serial.println("  - Detect errors in transmitted codewords");
    Serial.print("  - Correct up to ");
    Serial.print(bch.getT());
    Serial.println(" errors using BCH decoding");
    Serial.println("  - Verify decoded message against original");
    Serial.println();
    Serial.println("Waiting for first transmission...");
    Serial.println();

    resetStats();
}

void loop() {
    if (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();

        if (line.length() > 0) {
            // Check for control messages
            if (line.equals("TESTS_START")) {
                Serial.println("STARTED RECEIVING MESSAGES");
                Serial.println();
                resetStats();
                testingActive = true;
            } else if (line.equals("TESTS_COMPLETE")) {
                stats.endTime = millis();
                testingActive = false;
                printStats();
            } else if (line.startsWith("PROGRESS:")) {
                // Extract progress numbers from sender
                int slashPos = line.indexOf('/');
                if (slashPos > 0) {
                    String progressStr =
                        line.substring(9, slashPos);  // After "PROGRESS:"
                    Serial.print("Progress: ");
                    Serial.print(progressStr);
                    Serial.println("/1000 messages");
                }
            } else if (line.startsWith("TEST:")) {
                // Parse: TEST:<test_num>:<error_count>
                int firstColon = line.indexOf(':');
                int secondColon = line.indexOf(':', firstColon + 1);

                int testNum =
                    line.substring(firstColon + 1, secondColon).toInt();
                int errorCount = line.substring(secondColon + 1).toInt();

                // Read original message
                uint8_t originalMessage[BCH_K];
                for (int i = 0; i < BCH_K; i++) {
                    while (!Serial.available());
                    originalMessage[i] = Serial.parseInt();
                }

                // Read received codeword
                uint8_t received[BCH_N];
                for (int i = 0; i < BCH_N; i++) {
                    while (!Serial.available());
                    received[i] = Serial.parseInt();
                }

                // Process the test
                processTest(testNum, errorCount, originalMessage, received);
            }
        }
    }
}
