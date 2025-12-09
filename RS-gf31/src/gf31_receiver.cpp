#include <Arduino.h>
#include <SoftwareSerial.h>

#include "gf31_math.hpp"

SoftwareSerial softSerial(13, 12);  // RX, TX  -  D6, D7

struct Point {
    int x;
    int y;
};
Point points[6];
int count = 0;
int original_data[MAX_COEFFS];  // Original data coefficients
int data_count = 0;

// Statistics
int total_transmissions = 0;
int clean_transmissions = 0;      // No errors
int corrected_transmissions = 0;  // 1 error corrected
int failed_corrections = 0;       // 2+ errors, cannot correct
int correct_corrections = 0;      // Successfully corrected to original data
int incorrect_corrections = 0;    // Corrected but to wrong data

// Store one example of incorrect correction
bool has_incorrect_example = false;
int example_original[MAX_COEFFS];
Point example_points[6];
int example_decoded[MAX_COEFFS];

const int MESSAGES_PER_TEST = 1000;
bool test_in_progress = false;
bool test_completed = false;
unsigned long first_message_time = 0;

void print_test_summary() {
    unsigned long test_duration =
        (millis() - first_message_time) / 1000;  // in seconds

    Serial.println("\n\n");
    Serial.println("=======================================================");
    Serial.println("           TEST SUMMARY - 1000 MESSAGES");
    Serial.println("=======================================================");
    Serial.println();

    // Calculate percentages
    float clean_percent = (clean_transmissions * 100.0) / MESSAGES_PER_TEST;
    float corrected_percent =
        (corrected_transmissions * 100.0) / MESSAGES_PER_TEST;
    float failed_percent = (failed_corrections * 100.0) / MESSAGES_PER_TEST;
    float success_percent =
        ((clean_transmissions + corrected_transmissions) * 100.0) /
        MESSAGES_PER_TEST;

    // Results
    Serial.println("RESULTS:");
    Serial.print("  OK (no errors):        ");
    Serial.print(clean_transmissions);
    Serial.print("/1000  (");
    Serial.print(clean_percent, 1);
    Serial.println("%)");

    Serial.print("  CORRECTED (1 error):   ");
    Serial.print(corrected_transmissions);
    Serial.print("/1000  (");
    Serial.print(corrected_percent, 1);
    Serial.println("%)");

    Serial.print("  DETECTED (2+ errors):  ");
    Serial.print(failed_corrections);
    Serial.print("/1000  (");
    Serial.print(failed_percent, 1);
    Serial.println("%)");

    Serial.println();
    Serial.print("SUCCESS RATE:            ");
    int successful = clean_transmissions + corrected_transmissions;
    Serial.print(successful);
    Serial.print("/1000  (");
    Serial.print(success_percent, 1);
    Serial.println("%)");

    // Correction accuracy
    Serial.println();
    Serial.println("CORRECTION ACCURACY:");
    Serial.print("  Correctly corrected:   ");
    Serial.print(correct_corrections);
    Serial.print("/");
    Serial.print(corrected_transmissions);
    if (corrected_transmissions > 0) {
        float correct_percent =
            (correct_corrections * 100.0) / corrected_transmissions;
        Serial.print("  (");
        Serial.print(correct_percent, 1);
        Serial.println("%)");
    } else {
        Serial.println();
    }

    Serial.print("  Incorrectly corrected: ");
    Serial.print(incorrect_corrections);
    Serial.print("/");
    Serial.print(corrected_transmissions);
    if (corrected_transmissions > 0) {
        float incorrect_percent =
            (incorrect_corrections * 100.0) / corrected_transmissions;
        Serial.print("  (");
        Serial.print(incorrect_percent, 1);
        Serial.println("%)");
    } else {
        Serial.println();
    }

    // Show example of incorrect correction
    if (has_incorrect_example) {
        Serial.println();
        Serial.println("EXAMPLE OF INCORRECT CORRECTION:");
        Serial.print("  Original data:    [");
        for (int i = 0; i < MAX_COEFFS; i++) {
            Serial.print(example_original[i]);
            if (i < MAX_COEFFS - 1) Serial.print(", ");
        }
        Serial.println("]");

        Serial.println("  Received points:");
        for (int i = 0; i < 6; i++) {
            Serial.print("    (");
            Serial.print(example_points[i].x);
            Serial.print(", ");
            Serial.print(example_points[i].y);
            Serial.println(")");
        }

        Serial.print("  Decoded data:     [");
        for (int i = 0; i < MAX_COEFFS; i++) {
            Serial.print(example_decoded[i]);
            if (i < MAX_COEFFS - 1) Serial.print(", ");
        }
        Serial.println("]");
    }

    Serial.println();
    Serial.println("TEST INFO:");
    Serial.print("  Total test time: ");
    Serial.print(test_duration);
    Serial.println(" seconds");

    if (test_duration > 0) {
        Serial.print("  Speed: ");
        Serial.print(MESSAGES_PER_TEST / test_duration);
        Serial.println(" messages/s");
    }

    Serial.println();
    Serial.println("=======================================================");
    Serial.println("Waiting for reset...");
    Serial.println();
}

// Forward declarations
void lagrange_interpolate(Point *pts, int n, int coeffs[]);
bool verify_points(Point *pts, int n, int coeffs[], int degree);

void lagrange_interpolate(Point *pts, int n, int coeffs[]) {
    for (int i = 0; i < n; i++) coeffs[i] = 0;

    for (int i = 0; i < n; i++) {
        int Li_coeffs[MAX_COEFFS] = {1, 0, 0, 0};
        int Li_size = 1;
        int denom = 1;

        for (int j = 0; j < n; j++) {
            if (j == i) continue;

            int xj = pts[j].x;
            int xi = pts[i].x;

            int newLi[MAX_COEFFS] = {0, 0, 0, 0};
            for (int a = 0; a < Li_size; a++) {
                newLi[a] = gf_add(newLi[a], gf_mul(Li_coeffs[a], MOD - xj));
                newLi[a + 1] = gf_add(newLi[a + 1], Li_coeffs[a]);
            }
            for (int a = 0; a <= Li_size; a++) Li_coeffs[a] = newLi[a];
            Li_size++;

            denom = gf_mul(denom, gf_add(xi, MOD - xj));
        }

        int inv_denom = gf_inv(denom);
        int scalar = gf_mul(pts[i].y, inv_denom);
        for (int a = 0; a < Li_size; a++) {
            coeffs[a] = gf_add(coeffs[a], gf_mul(Li_coeffs[a], scalar));
        }
    }
}

// Calculate polynomial value at point x
int evaluate_polynomial(int coeffs[], int degree, int x) {
    int result = 0;
    int x_power = 1;

    for (int i = 0; i <= degree; i++) {
        result = gf_add(result, gf_mul(coeffs[i], x_power));
        x_power = gf_mul(x_power, x);
    }

    return result;
}

// Check if all points fit the polynomial
bool verify_points(Point *pts, int n, int coeffs[], int degree) {
    for (int i = 0; i < n; i++) {
        int calculated_y = evaluate_polynomial(coeffs, degree, pts[i].x);
        if (calculated_y != pts[i].y) {
            return false;
        }
    }
    return true;
}

// Copy points excluding skip_idx
void copy_points_except(Point *src, int n, Point *dst, int skip_idx) {
    int dst_idx = 0;
    for (int i = 0; i < n; i++) {
        if (i != skip_idx) {
            dst[dst_idx++] = src[i];
        }
    }
}

// Main Reed-Solomon error correction function
int reed_solomon_decode(Point *pts, int n, int coeffs[], int *error_idx) {
    // n = 6 points, need 4 for interpolation (degree 3 polynomial)
    // We have 2 redundant points - can correct 1 error

    if (n < 4) {
        return -1;  // Not enough points
    }

    // Step 1: Try interpolation with first 4 points
    lagrange_interpolate(pts, 4, coeffs);

    // Step 2: Check if all points fit the polynomial
    if (verify_points(pts, n, coeffs, 3)) {
        clean_transmissions++;
        return 0;  // No errors
    }

    // Step 3: There are errors - try to find 1 erroneous point
    // Test all combinations excluding 1 point at a time
    for (int skip = 0; skip < n; skip++) {
        Point test_points[6];
        copy_points_except(pts, n, test_points, skip);

        // Interpolate with 5 points (using first 4)
        int test_coeffs[MAX_COEFFS];
        lagrange_interpolate(test_points, 4, test_coeffs);

        // Check if all 5 points fit the polynomial
        if (verify_points(test_points, n - 1, test_coeffs, 3)) {
            // Copy correct coefficients
            for (int i = 0; i < MAX_COEFFS; i++) {
                coeffs[i] = test_coeffs[i];
            }

            *error_idx = skip;
            corrected_transmissions++;
            return 1;  // 1 error corrected
        }
    }

    // Step 4: Could not find 1 erroneous point - we have 2 or more errors
    failed_corrections++;
    return 2;  // 2 or more errors
}

void setup() {
    Serial.begin(115200);
    softSerial.begin(9600);
    delay(2000);

    Serial.println("=======================================================");
    Serial.println("       GF(31) RECEIVER - 1000 MESSAGE TEST");
    Serial.println("=======================================================");
    Serial.println();
    Serial.println("Receiver functions:");
    Serial.println("  - Detect errors in x (duplicates, out of range values)");
    Serial.println("  - Detect errors in y (incorrect values)");
    Serial.println("  - Correct 1 error in y (Reed-Solomon)");
    Serial.println("  - Detect 2 or more errors");
    Serial.println();
    Serial.println("Waiting for first transmission...");
    Serial.println();
}

void loop() {
    if (test_completed) {
        // Test completed, wait for reset
        return;
    }

    if (softSerial.available()) {
        uint8_t frame = softSerial.read();

        // Start test on first message
        if (!test_in_progress) {
            test_in_progress = true;
            first_message_time = millis();
            Serial.println("STARTED RECEIVING MESSAGES");
            Serial.println();
        }

        // First receive 4 bytes of original data
        if (data_count < MAX_COEFFS) {
            original_data[data_count] = frame & 0x1F;
            data_count++;
            return;
        }

        // Then receive 6 encoded points
        int x = (frame >> 5) & 0x07;
        int y = frame & 0x1F;

        points[count].x = x;
        points[count].y = y;
        count++;

        if (count == 6) {
            total_transmissions++;

            // Display progress every 100 messages
            if (total_transmissions % 100 == 0) {
                Serial.print("Progress: ");
                Serial.print(total_transmissions);
                Serial.println("/1000 messages");
            }

            int decoded_coeffs[MAX_COEFFS];
            bool is_corrected = false;
            bool is_ok = false;

            // ===== NOWA LOGIKA DETEKCJI DUPLIKATÓW X =====
            
            // Sprawdź czy są duplikaty X
            int x_count[6] = {0, 0, 0, 0, 0, 0};
            int duplicates = 0;
            
            for (int i = 0; i < 6; i++) {
                if (points[i].x >= 0 && points[i].x <= 5) {
                    x_count[points[i].x]++;
                }
            }
            
            for (int x = 0; x < 6; x++) {
                if (x_count[x] > 1) {
                    duplicates += (x_count[x] - 1);
                }
            }
            
            // Jeśli są 2 lub więcej duplikatów - nie da się poprawić
            if (duplicates >= 2) {
                failed_corrections++;
                count = 0;
                data_count = 0;
                
                if (total_transmissions >= MESSAGES_PER_TEST) {
                    test_completed = true;
                    print_test_summary();
                }
                return;
            }
            
            // Jeśli jest 1 duplikat - spróbuj wybrać prawidłowy punkt
            if (duplicates == 1) {
                // Znajdź który X jest zduplikowany
                int dup_x = -1;
                for (int x = 0; x < 6; x++) {
                    if (x_count[x] > 1) {
                        dup_x = x;
                        break;
                    }
                }
                
                // Znajdź oba punkty z tym X
                int dup_indices[2];
                int dup_found = 0;
                for (int i = 0; i < 6; i++) {
                    if (points[i].x == dup_x) {
                        dup_indices[dup_found++] = i;
                    }
                }
                
                // Spróbuj obie możliwości - która daje poprawny wielomian?
                bool found_correct = false;
                
                for (int try_keep = 0; try_keep < 2; try_keep++) {
                    Point test_points[6];
                    int test_idx = 0;
                    
                    // Kopiuj wszystkie punkty poza jednym duplikatem
                    for (int i = 0; i < 6; i++) {
                        if (i != dup_indices[1 - try_keep]) {  // Pomiń drugi duplikat
                            test_points[test_idx++] = points[i];
                        }
                    }
                    
                    // Interpoluj przez 4 punkty
                    int test_coeffs[MAX_COEFFS];
                    lagrange_interpolate(test_points, 4, test_coeffs);
                    
                    // Sprawdź czy wszystkie 5 punktów pasują
                    if (verify_points(test_points, 5, test_coeffs, 3)) {
                        // Znaleziono poprawny wielomian!
                        for (int i = 0; i < MAX_COEFFS; i++) {
                            decoded_coeffs[i] = test_coeffs[i];
                        }
                        found_correct = true;
                        is_corrected = true;
                        corrected_transmissions++;
                        break;
                    }
                }
                
                if (!found_correct) {
                    // Nie udało się znaleźć poprawnego wielomianu
                    failed_corrections++;
                    count = 0;
                    data_count = 0;
                    
                    if (total_transmissions >= MESSAGES_PER_TEST) {
                        test_completed = true;
                        print_test_summary();
                    }
                    return;
                }
            } else {
                // Brak duplikatów - użyj standardowej metody Reed-Solomon
                int error_idx;
                int error_count = reed_solomon_decode(points, 6, decoded_coeffs, &error_idx);
                
                if (error_count == 0) {
                    is_ok = true;
                } else if (error_count == 1) {
                    is_corrected = true;
                }
            }

            // Compare decoded data with original data
            if (is_ok || is_corrected) {
                bool data_matches = true;
                for (int i = 0; i < MAX_COEFFS; i++) {
                    if (decoded_coeffs[i] != original_data[i]) {
                        data_matches = false;
                        break;
                    }
                }

                if (is_corrected) {
                    if (data_matches) {
                        correct_corrections++;
                    } else {
                        incorrect_corrections++;

                        // Save first example of incorrect correction
                        if (!has_incorrect_example) {
                            has_incorrect_example = true;
                            for (int i = 0; i < MAX_COEFFS; i++) {
                                example_original[i] = original_data[i];
                                example_decoded[i] = decoded_coeffs[i];
                            }
                            for (int i = 0; i < 6; i++) {
                                example_points[i] = points[i];
                            }
                        }
                    }
                }
            }

            count = 0;
            data_count = 0;

            // Check if test completed
            if (total_transmissions >= MESSAGES_PER_TEST) {
                test_completed = true;
                print_test_summary();
            }
        }
    }
}