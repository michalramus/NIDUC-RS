#include <Arduino.h>
#include "rs.hpp"



// Globalne tablice
double receivedX[MAX_POINTS];
double receivedY[MAX_POINTS];
int n = 6;  // ilość punktów

// Odczyt danych (6 linii z "x,y")
void readPointsFromSerial() {
    Serial.println("Oczekiwanie na dane...");
    for (int i = 0; i < n; i++) {
        while (!Serial.available()) delay(10);
        String line = Serial.readStringUntil('\n');
        line.trim();
        int comma = line.indexOf(',');
        if (comma > 0) {
            receivedX[i] = line.substring(0, comma).toDouble();
            receivedY[i] = line.substring(comma + 1).toDouble();
        }
    }
    Serial.println("Dane odebrane.\n");
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    delay(1000);

    Serial.println("=== RECEIVER READY ===");

}

void loop() {
       // Odczyt nagłówka transmisji
    String header = "";
    while (header.length() == 0) {
        if (Serial.available()) {
            header = Serial.readStringUntil('\n');
            header.trim();
        }
        delay(10);
    }

    Serial.print("Tryb transmisji: ");
    Serial.println(header);

    // Wczytanie punktów
    readPointsFromSerial();

    // Liczba kombinacji C(6,4)=15
    int combinations[15][4];
    int combIndex = 0;

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

    double allCoeffs[15][4];

    Serial.println("Liczenie współczynników dla wszystkich kombinacji...\n");

    for (int c = 0; c < 15; c++) {
        double subX[4], subY[4];
        for (int i = 0; i < 4; i++) {
            int idx = combinations[c][i];
            subX[i] = receivedX[idx];
            subY[i] = receivedY[idx];
        }

        lagrangeInterpolation(subX, subY, 4, allCoeffs[c]);
    }

    // Sprawdzenie czy wszystkie współczynniki równe
    bool allEqual = true;
    for (int c = 1; c < 15; c++) {
        if (!coefficientsEqual(allCoeffs[0], allCoeffs[c], 4)) {
            allEqual = false;
            break;
        }
    }

    if (allEqual) {
        Serial.println("TRANSMISSION SUCCESSFUL!");
        Serial.println("Wszystkie kombinacje dają te same współczynniki.\n");
    } else {
        Serial.println("ERRORS DETECTED!");
        Serial.println("Rozpoczynam próbę korekcji...\n");

        int matchCounts[15] = {0};
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 15; j++) {
                if (coefficientsEqual(allCoeffs[i], allCoeffs[j], 4))
                    matchCounts[i]++;
            }
        }

        int maxMatches = 0;
        int bestIndex = 0;
        for (int i = 0; i < 15; i++) {
            if (matchCounts[i] > maxMatches) {
                maxMatches = matchCounts[i];
                bestIndex = i;
            }
        }

        if (maxMatches >= 1) {
            Serial.println("ERROR CORRECTION POSSIBLE!");
            Serial.print("Znaleziono ");
            Serial.print(maxMatches);
            Serial.println(" zgodnych kombinacji.\n");

            Serial.print("Poprawny wielomian: [");
            for (int i = 0; i < 4; i++) {
                Serial.print(allCoeffs[bestIndex][i], 6);
                if (i < 3) Serial.print(", ");
            }
            Serial.println("]");

            // Oznaczanie dobrych/złych punktów
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
                Serial.println("\nBrak błędów (fałszywy alarm).");
            } else if (errorCount == 1) {
                Serial.println("\nERROR CORRECTED!");
                int idx = errorIndices[0];
                double correctY = 0.0;
                double xVal = receivedX[idx];

                for (int j = 0; j < 4; j++) {
                    double term = allCoeffs[bestIndex][j];
                    for (int p = 0; p < j; p++) term *= xVal;
                    correctY += term;
                }

                Serial.print("Poprawiono punkt ");
                Serial.print(idx);
                Serial.print(": (");
                Serial.print(xVal, 2);
                Serial.print(", ");
                Serial.print(receivedY[idx], 2);
                Serial.print(") → (");
                Serial.print(xVal, 2);
                Serial.print(", ");
                Serial.print(correctY, 2);
                Serial.println(")");
            } else {
                Serial.print("\nERROR CORRECTION IMPOSSIBLE! ");
                Serial.print("Wykryto ");
                Serial.print(errorCount);
                Serial.println(" błędy (za dużo by skorygować).");
            }
        } else {
            Serial.println("ERROR CORRECTION IMPOSSIBLE!");
            Serial.println("Zbyt dużo błędów, brak zgodności wielomianów.");
        }
    }

    Serial.println("\n=== KONIEC TRANSMISJI ===");
}
