#include <Arduino.h>

#include "rs.hpp"

bool coefficientsEqual(double coeffs1[], double coeffs2[], int size,
                       double tolerance = 0.0001) {
    for (int i = 0; i < size; i++) {
        if (abs(coeffs1[i] - coeffs2[i]) > tolerance) {
            return false;
        }
    }
    return true;
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    int n = 6;  // Total points
    double receivedX[MAX_POINTS] = {0, 1, 2, 3, 4, 5};
    double receivedY[MAX_POINTS] = {2, 1, -4, -7, -2, 17};

    // Calculate C(6,4) = 15 combinations
    int combinations[15][4];
    int combIndex = 0;

    // Generate all combinations of 4 points from 6
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            for (int m = j + 1; m < n; m++) {
                for (int p = m + 1; p < n; p++) {
                    combinations[combIndex][0] = i;
                    combinations[combIndex][1] = j;
                    combinations[combIndex][2] = m;
                    combinations[combIndex][3] = p;
                    combIndex++;
                }
            }
        }
    }

    // Store all coefficient sets
    double allCoeffs[15][4];

    // For each combination, calculate coefficients
    Serial.println(
        "Calculating coefficients for all C(6,4) = 15 combinations:\n");

    for (int c = 0; c < 15; c++) {
        double subX[4], subY[4];

        // Extract the 4 points for this combination
        for (int i = 0; i < 4; i++) {
            int idx = combinations[c][i];
            subX[i] = receivedX[idx];
            subY[i] = receivedY[idx];
        }

        // Calculate interpolating polynomial coefficients
        lagrangeInterpolation(subX, subY, 4, allCoeffs[c]);

        // Print results
        Serial.print("Combination ");
        Serial.print(c + 1);
        Serial.print(": Points [");
        for (int i = 0; i < 4; i++) {
            Serial.print(combinations[c][i]);
            if (i < 3) Serial.print(", ");
        }
        Serial.println("]");

        Serial.print("  Polynomial: P(x) = ");
        for (int i = 0; i < 4; i++) {
            if (i > 0 && allCoeffs[c][i] >= 0) Serial.print("+ ");
            Serial.print(allCoeffs[c][i], 6);
            if (i > 0) {
                Serial.print("*x^");
                Serial.print(i);
            }
            Serial.print(" ");
        }
        Serial.println("\n");
    }

    // Check if all coefficients are equal
    Serial.println("\n========== ERROR CHECKING ==========\n");

    bool allEqual = true;
    for (int c = 1; c < 15; c++) {
        if (!coefficientsEqual(allCoeffs[0], allCoeffs[c], 4)) {
            allEqual = false;
            break;
        }
    }

    if (allEqual) {
        Serial.println("✓ TRANSMISSION SUCCESSFUL!");
        Serial.println("All combinations produce identical coefficients.");
        Serial.println("\nCorrect received data:");
        Serial.print("Polynomial coefficients: [");
        for (int i = 0; i < 4; i++) {
            Serial.print(allCoeffs[0][i], 6);
            if (i < 3) Serial.print(", ");
        }
        Serial.println("]");

        Serial.println("\nReceived points:");
        for (int i = 0; i < 6; i++) {
            Serial.print("  Point ");
            Serial.print(i);
            Serial.print(": (");
            Serial.print(receivedX[i], 1);
            Serial.print(", ");
            Serial.print(receivedY[i], 1);
            Serial.println(")");
        }
    } else {
        Serial.println("✗ ERRORS DETECTED!");
        Serial.println(
            "Different combinations produce different coefficients.");
        Serial.println("\nAttempting error correction...\n");

        // Count which coefficient sets appear most frequently (majority voting)
        int matchCounts[15] = {0};
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 15; j++) {
                if (coefficientsEqual(allCoeffs[i], allCoeffs[j], 4)) {
                    matchCounts[i]++;
                }
            }
        }

        // Find the most common coefficient set
        int maxMatches = 0;
        int bestIndex = 0;
        for (int i = 0; i < 15; i++) {
            if (matchCounts[i] > maxMatches) {
                maxMatches = matchCounts[i];
                bestIndex = i;
            }
        }

        // With 6 points and 4 needed, we can correct up to 2 errors
        // At least C(4,4) = 1 combination should agree if 2 or fewer errors
        if (maxMatches >= 1) {
            Serial.println("✓ ERROR CORRECTION POSSIBLE!");
            Serial.print("Found ");
            Serial.print(maxMatches);
            Serial.println(" combinations agreeing on the correct polynomial.");

            Serial.print("\nCorrect polynomial coefficients: [");
            for (int i = 0; i < 4; i++) {
                Serial.print(allCoeffs[bestIndex][i], 6);
                if (i < 3) Serial.print(", ");
            }
            Serial.println("]");

            // Find which points are erroneous
            Serial.println("\nIdentifying erroneous points...");

            // A point is bad if it doesn't appear in any "good" combination
            bool pointIsGood[6] = {false, false, false, false, false, false};

            for (int c = 0; c < 15; c++) {
                if (coefficientsEqual(allCoeffs[c], allCoeffs[bestIndex], 4)) {
                    for (int i = 0; i < 4; i++) {
                        pointIsGood[combinations[c][i]] = true;
                    }
                }
            }

            int errorCount = 0;
            int errorIndices[6];
            for (int i = 0; i < 6; i++) {
                if (!pointIsGood[i]) {
                    errorIndices[errorCount++] = i;
                }
            }

            if (errorCount == 0) {
                Serial.println("No errors detected (false alarm).");
            } else if (errorCount == 1) {
                Serial.println("✓ ERROR CORRECTED!");
                Serial.print("Corrected 1 error at point: ");
                Serial.println(errorIndices[0]);

                // Calculate correct value using the correct polynomial
                int idx = errorIndices[0];
                double correctY = 0.0;
                double xVal = receivedX[idx];

                for (int j = 0; j < 4; j++) {
                    double term = allCoeffs[bestIndex][j];
                    for (int p = 0; p < j; p++) {
                        term *= xVal;
                    }
                    correctY += term;
                }

                Serial.print("  Point ");
                Serial.print(idx);
                Serial.print(": (");
                Serial.print(xVal, 1);
                Serial.print(", ");
                Serial.print(receivedY[idx], 1);
                Serial.print(") → corrected to (");
                Serial.print(xVal, 1);
                Serial.print(", ");
                Serial.print(correctY, 1);
                Serial.println(")");
            } else {
                Serial.println("✗ ERROR CORRECTION IMPOSSIBLE!");
                Serial.print("Too many errors detected (");
                Serial.print(errorCount);
                Serial.println(" errors).");
                Serial.println(
                    "Reed-Solomon with 6 points and degree 3 can only correct "
                    "up to 1 error.");
                Serial.println(
                    "At least 2 points are wrong - correction is impossible.");
            }
        } else {
            Serial.println("✗ ERROR CORRECTION IMPOSSIBLE!");
            Serial.println(
                "No consistent polynomial found among combinations.");
            Serial.println(
                "Too many errors to correct with current redundancy.");
        }
    }
}

void loop() {
    // Nothing here
}

//  int n = 5;  // Example: 3 points
//   double x[MAX_POINTS] = {1, 2, 3, 4, 5};
//   double y[MAX_POINTS] = {1, -4, -7, -2, 17};
//   double coeffs[MAX_POINTS];

//   lagrangeInterpolation(x, y, n, coeffs);

//   Serial.println("\nInterpolating polynomial coefficients:");
//   for (int i = 0; i < n; i++) {
//     Serial.print("a[");
//     Serial.print(i);
//     Serial.print("] = ");
//     Serial.println(coeffs[i], 6);
//   }

//   Serial.print("\nPolynomial: P(x) = ");
//   for (int i = 0; i < n; i++) {
//     Serial.print(coeffs[i], 6);
//     if (i > 0) {
//       Serial.print("*x^");
//       Serial.print(i);
//     }
//     if (i != n - 1) Serial.print(" + ");
//   }
//   Serial.println();
