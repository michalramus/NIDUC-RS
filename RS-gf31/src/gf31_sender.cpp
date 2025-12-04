#include <Arduino.h>
#include <SoftwareSerial.h>

#include "gf31_math.hpp"

// TX -> pin 12, RX -> pin 13
SoftwareSerial softSerial(13, 12);  // RX, TX D7, D6

int coeffs[MAX_COEFFS] = {5, 7, 3, 2};

// Transmission modes:
// 0 = clean (no errors)
// 1 = 1 error in y
// 2 = 2 errors in y
// 3 = 1 error in x
// 4 = 1 error in x and 1 error in y
// 5 = 2 errors in x
int transmission_mode = 0;  // Selected by user
const int MESSAGES_PER_TEST = 1000;
int messages_sent = 0;
bool test_started = false;
bool test_completed = false;

int poly_eval(int x) {
    int result = 0;
    int power = 1;
    for (int i = 0; i < MAX_COEFFS; i++) {
        result = gf_add(result, gf_mul(coeffs[i], power));
        power = gf_mul(power, x);
    }
    return result;
}

void send_point(int x, int y) {
    uint8_t frame = ((x & 0x07) << 5) | (y & 0x1F);
    softSerial.write(frame);
}

// Introduce error to y value
int introduce_y_error(int y) {
    // Add random value from 1 to 30 (in GF(31))
    // Make sure the new value is different from original
    int new_y = y;
    while (new_y == y) {
        int error = random(1, 31);
        new_y = gf_add(y, error);
    }
    return new_y;
}

// Introduce error to x value
int introduce_x_error(int x) {
    // Change x to another random value from range [0, 5]
    int new_x = random(0, 6);
    // Make sure it's different from original
    while (new_x == x) {
        new_x = random(0, 6);
    }
    return new_x;
}

void send_transmission() {
    int x_values[6] = {0, 1, 2, 3, 4, 5};  // Default x values
    int y_values[6];

    // Calculate all correct y values
    for (int x = 0; x < 6; x++) {
        y_values[x] = poly_eval(x);
    }

    // Introduce errors based on mode
    if (transmission_mode == 1) {
        // 1 error in y - at random position
        int error_position = random(0, 6);
        y_values[error_position] = introduce_y_error(y_values[error_position]);

    } else if (transmission_mode == 2) {
        // 2 errors in y - at random positions
        int error_pos1 = random(0, 6);
        int error_pos2 = random(0, 6);

        while (error_pos2 == error_pos1) {
            error_pos2 = random(0, 6);
        }

        y_values[error_pos1] = introduce_y_error(y_values[error_pos1]);
        y_values[error_pos2] = introduce_y_error(y_values[error_pos2]);

    } else if (transmission_mode == 3) {
        // 1 error in x - at random position
        int error_position = random(0, 6);
        x_values[error_position] = introduce_x_error(x_values[error_position]);

    } else if (transmission_mode == 4) {
        // 1 error in x and 1 error in y - at different positions
        int x_error_pos = random(0, 6);
        int y_error_pos = random(0, 6);

        while (y_error_pos == x_error_pos) {
            y_error_pos = random(0, 6);
        }

        x_values[x_error_pos] = introduce_x_error(x_values[x_error_pos]);
        y_values[y_error_pos] = introduce_y_error(y_values[y_error_pos]);

    } else if (transmission_mode == 5) {
        // 2 errors in x - at random positions
        int error_pos1 = random(0, 6);
        int error_pos2 = random(0, 6);

        while (error_pos2 == error_pos1) {
            error_pos2 = random(0, 6);
        }

        x_values[error_pos1] = introduce_x_error(x_values[error_pos1]);
        x_values[error_pos2] = introduce_x_error(x_values[error_pos2]);
    }

    // Send all points (silently)
    for (int i = 0; i < 6; i++) {
        send_point(x_values[i], y_values[i]);
    }
}

void setup() {
    Serial.begin(115200);
    softSerial.begin(9600);
    randomSeed(analogRead(0));
    delay(2000);

    Serial.println("=======================================================");
    Serial.println("    GF(31) SENDER - 1000 MESSAGE TEST");
    Serial.println("=======================================================");
    Serial.println();
    Serial.println("Available test modes:");
    Serial.println("  0: CLEAN (no errors)");
    Serial.println("  1: 1 ERROR in Y");
    Serial.println("  2: 2 ERRORS in Y");
    Serial.println("  3: 1 ERROR in X");
    Serial.println("  4: 1 ERROR in X + 1 ERROR in Y");
    Serial.println("  5: 2 ERRORS in X");
    Serial.println();
    Serial.println("Enter mode number (0-5) and press Enter:");
}

void loop() {
    // Wait for user mode selection
    if (!test_started && Serial.available() > 0) {
        int input = Serial.read();

        // Read rest from buffer
        while (Serial.available() > 0) {
            Serial.read();
        }

        if (input >= '0' && input <= '5') {
            transmission_mode = input - '0';
            test_started = true;

            Serial.println();
            Serial.println(
                "=======================================================");
            Serial.print("Selected mode: ");
            Serial.print(transmission_mode);
            Serial.print(" - ");
            switch (transmission_mode) {
                case 0:
                    Serial.println("CLEAN (no errors)");
                    break;
                case 1:
                    Serial.println("1 ERROR in Y");
                    break;
                case 2:
                    Serial.println("2 ERRORS in Y");
                    break;
                case 3:
                    Serial.println("1 ERROR in X");
                    break;
                case 4:
                    Serial.println("1 ERROR in X + 1 ERROR in Y");
                    break;
                case 5:
                    Serial.println("2 ERRORS in X");
                    break;
            }
            Serial.println(
                "=======================================================");
            Serial.println();
            Serial.println(
                "Waiting 10 seconds before starting transmission...");
            Serial.println();
            delay(10000);

            Serial.println("START - Sending 1000 messages...");
            Serial.println();
        } else {
            Serial.println("Invalid mode! Enter number from 0 to 5:");
        }
    }

    // Send messages if test started and not completed
    if (test_started && !test_completed) {
        send_transmission();
        messages_sent++;

        // Display progress every 100 messages
        if (messages_sent % 100 == 0) {
            Serial.print("Progress: ");
            Serial.print(messages_sent);
            Serial.println("/1000 messages");
        }

        // Complete after 1000 messages
        if (messages_sent >= MESSAGES_PER_TEST) {
            test_completed = true;
            Serial.println();
            Serial.println(
                "=======================================================");
            Serial.println("TEST COMPLETED");
            Serial.print("Sent ");
            Serial.print(messages_sent);
            Serial.println(" messages");
            Serial.println(
                "=======================================================");
            Serial.println();
            Serial.println("Waiting for reset...");
        }
    }
}
