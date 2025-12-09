#include <Arduino.h>
#include <SoftwareSerial.h>

#include "bch.hpp"

// SoftwareSerial for communication with receiver
// RX, TX pins (TX of sender connects to RX of receiver)
SoftwareSerial softSerial(D5, D6);  // RX=D6, TX=D5

// BCH parameters - can be changed to any valid values for GF(2^4)
#define BCH_N 15  // Codeword length
#define BCH_K 7   // Message length
#define BCH_T 2   // Error correction capability

// Test configuration
#define NUM_TESTS 1000
#define TEST_DELAY 50  // ms between transmissions

// Create BCH instance
BCH bch(BCH_N, BCH_K, BCH_T);

// Test mode selection
enum TestMode {
    MODE_NO_ERROR = 0,
    MODE_ONE_ERROR = 1,
    MODE_TWO_ERRORS = 2,
    MODE_THREE_ERRORS = 3,
    MODE_RANDOM = 4,
    MODE_MIXED = 5
};

TestMode currentMode = MODE_MIXED;
int testCount = 0;
bool testsRunning = false;

// Generate random message
void generateRandomMessage(uint8_t* message) {
    for (int i = 0; i < BCH_K; i++) {
        message[i] = random(0, 2);
    }
}

// Introduce errors based on mode
void introduceErrors(uint8_t* codeword, TestMode mode, int testNum) {
    int numErrors = 0;

    switch (mode) {
        case MODE_NO_ERROR:
            numErrors = 0;
            break;

        case MODE_ONE_ERROR:
            numErrors = 1;
            break;

        case MODE_TWO_ERRORS:
            numErrors = 2;
            break;

        case MODE_THREE_ERRORS:
            numErrors = 3;
            break;

        case MODE_RANDOM:
            numErrors = random(0, 4);  // 0-3 errors
            break;

        case MODE_MIXED:
            // Distribute: 25% no error, 25% one error, 25% two errors, 25%
            // three errors
            numErrors = testNum % 4;
            break;
    }

    // Introduce random errors
    if (numErrors > 0) {
        bool errorPositions[BCH_N] = {false};
        int errorsAdded = 0;

        while (errorsAdded < numErrors) {
            int pos = random(0, BCH_N);
            if (!errorPositions[pos]) {
                codeword[pos] ^= 1;
                errorPositions[pos] = true;
                errorsAdded++;
            }
        }
    }
}

// Transmit a test message
void transmitTest(int testNum) {
    uint8_t message[BCH_K];
    generateRandomMessage(message);

    uint8_t codeword[BCH_N];
    bch.encode(message, codeword);

    // Determine error count before introducing errors
    TestMode effectiveMode = currentMode;
    if (currentMode == MODE_MIXED) {
        effectiveMode = (TestMode)(testNum % 4);
    } else if (currentMode == MODE_RANDOM) {
        effectiveMode = (TestMode)random(0, 4);
    }

    int errorCount = 0;
    switch (effectiveMode) {
        case MODE_NO_ERROR:
            errorCount = 0;
            break;
        case MODE_ONE_ERROR:
            errorCount = 1;
            break;
        case MODE_TWO_ERRORS:
            errorCount = 2;
            break;
        case MODE_THREE_ERRORS:
            errorCount = 3;
            break;
        default:
            errorCount = 0;
    }

    // Introduce errors
    introduceErrors(codeword, currentMode, testNum);

    // Send test header via SoftwareSerial: TEST:<test_num>:<error_count>
    softSerial.print("TEST:");
    softSerial.print(testNum);
    softSerial.print(":");
    softSerial.println(errorCount);

    // Send message (for verification)
    for (int i = 0; i < BCH_K; i++) {
        softSerial.println(message[i]);
    }

    // Send codeword
    for (int i = 0; i < BCH_N; i++) {
        softSerial.println(codeword[i]);
    }
}

void printMenu() {
    Serial.println();
    Serial.println("=== BCH AUTOMATED TEST SENDER ===");
    Serial.print("BCH(");
    Serial.print(bch.getN());
    Serial.print(",");
    Serial.print(bch.getK());
    Serial.print(",");
    Serial.print(bch.getT());
    Serial.print(") - can correct up to ");
    Serial.print(bch.getT());
    Serial.println(" errors");
    Serial.println();
    Serial.println("Select test mode:");
    Serial.println("0 - No errors (all correct)");
    Serial.println("1 - One error per message");
    Serial.println("2 - Two errors per message");
    Serial.println("3 - Three errors per message (beyond capability)");
    Serial.println("4 - Random errors (0-3 per message)");
    Serial.println("5 - Mixed mode (25% each: 0, 1, 2, 3 errors)");
    Serial.println();
    Serial.print("Enter mode (0-5): ");
}

void setup() {
    Serial.begin(115200);
    softSerial.begin(9600);  // Initialize SoftwareSerial at 9600 baud
    delay(2000);

    randomSeed(analogRead(0));  // Initialize random seed

    printMenu();
}

void loop() {
    if (!testsRunning) {
        // Wait for user input
        if (Serial.available()) {
            int input = Serial.parseInt();

            if (input >= 0 && input <= 5) {
                currentMode = (TestMode)input;
                Serial.println(input);
                Serial.println();
                Serial.print("Starting ");
                Serial.print(NUM_TESTS);
                Serial.print(" tests in mode ");
                Serial.print(input);
                Serial.println("...");

                // Send start signal to receiver via SoftwareSerial
                softSerial.println("TESTS_START");
                Serial.println("TESTS_START");

                testsRunning = true;
                testCount = 0;
                delay(1000);
            } else {
                Serial.println("Invalid input!");
                printMenu();
            }
        }
    } else {
        // Run tests
        if (testCount < NUM_TESTS) {
            transmitTest(testCount);
            testCount++;

            // Progress indicator every 100 tests
            if (testCount % 100 == 0) {
                softSerial.print("PROGRESS:");
                softSerial.print(testCount);
                softSerial.print("/");
                softSerial.println(NUM_TESTS);

                Serial.print("PROGRESS:");
                Serial.print(testCount);
                Serial.print("/");
                Serial.println(NUM_TESTS);
            }

            delay(TEST_DELAY);
        } else {
            // Tests complete
            softSerial.println("TESTS_COMPLETE");
            Serial.println("TESTS_COMPLETE");
            Serial.println();
            Serial.println("All tests transmitted!");
            Serial.println();

            testsRunning = false;
            testCount = 0;

            // Wait a bit and show menu again
            delay(2000);
            printMenu();
        }
    }
}
