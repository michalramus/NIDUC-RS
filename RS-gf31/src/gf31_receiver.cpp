#include <Arduino.h>
#include <SoftwareSerial.h>

#include "gf31_math.hpp"

SoftwareSerial softSerial(13, 12);  // RX, TX

struct Point {
    int x;
    int y;
};
Point points[6];
int count = 0;

// Statystyki
int total_transmissions = 0;
int successful_corrections = 0;
int failed_corrections = 0;

// Funkcja walidująca zakres współrzędnej x
bool is_valid_x(int x) { return (x >= 0 && x <= 5); }

// Funkcja walidująca zakres współrzędnej y
bool is_valid_y(int y) {
    return (y >= 0 && y < MOD);  // y musi być w zakresie [0, 30]
}

// Funkcja sprawdzająca czy są duplikaty x
bool hasDuplicateX(Point *pts, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (pts[i].x == pts[j].x) {
                Serial.print("⚠️  WYKRYTO DUPLIKAT: x=");
                Serial.print(pts[i].x);
                Serial.print(" w punktach #");
                Serial.print(i);
                Serial.print(" i #");
                Serial.println(j);
                return true;
            }
        }
    }
    return false;
}

// Funkcja zliczająca wystąpienia każdego x
void analyzeXDistribution(Point *pts, int n) {
    int x_count[6] = {0, 0, 0, 0, 0, 0};
    int out_of_range = 0;

    for (int i = 0; i < n; i++) {
        if (pts[i].x >= 0 && pts[i].x <= 5) {
            x_count[pts[i].x]++;
        } else {
            out_of_range++;
        }
    }

    Serial.println("📋 Analiza rozkładu x:");
    for (int x = 0; x < 6; x++) {
        Serial.print("  x=");
        Serial.print(x);
        Serial.print(": ");
        Serial.print(x_count[x]);
        if (x_count[x] == 0) {
            Serial.println(" ❌ BRAK!");
        } else if (x_count[x] > 1) {
            Serial.println(" ⚠️  DUPLIKAT!");
        } else {
            Serial.println(" ✓");
        }
    }

    if (out_of_range > 0) {
        Serial.print("❌ Wartości x poza zakresem [0,5]: ");
        Serial.println(out_of_range);
    }
}

// Funkcja wybierająca unikalne punkty
int selectUniquePoints(Point *pts, int n, Point *unique_pts) {
    int unique_count = 0;

    for (int i = 0; i < n; i++) {
        bool found = false;
        for (int j = 0; j < unique_count; j++) {
            if (unique_pts[j].x == pts[i].x) {
                found = true;
                break;
            }
        }
        if (!found) {
            unique_pts[unique_count].x = pts[i].x;
            unique_pts[unique_count].y = pts[i].y;
            unique_count++;
        }
    }

    return unique_count;
}

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

// Oblicza wartość wielomianu w punkcie x
int evaluate_polynomial(int coeffs[], int degree, int x) {
    int result = 0;
    int x_power = 1;

    for (int i = 0; i <= degree; i++) {
        result = gf_add(result, gf_mul(coeffs[i], x_power));
        x_power = gf_mul(x_power, x);
    }

    return result;
}

// Sprawdza czy wszystkie punkty pasują do wielomianu
bool verify_points(Point *pts, int n, int coeffs[], int degree) {
    for (int i = 0; i < n; i++) {
        int calculated_y = evaluate_polynomial(coeffs, degree, pts[i].x);
        if (calculated_y != pts[i].y) {
            return false;
        }
    }
    return true;
}

// Kopiuje punkty z pominięciem indeksu skip_idx
void copy_points_except(Point *src, int n, Point *dst, int skip_idx) {
    int dst_idx = 0;
    for (int i = 0; i < n; i++) {
        if (i != skip_idx) {
            dst[dst_idx++] = src[i];
        }
    }
}

// Główna funkcja korekcji błędów Reed-Solomon
int reed_solomon_decode(Point *pts, int n, int coeffs[], int *error_idx) {
    // n = 6 punktów, potrzebujemy 4 do interpolacji (wielomian stopnia 3)
    // Mamy 2 punkty nadmiarowe - możemy skorygować 1 błąd

    if (n < 4) {
        Serial.println("❌ Za mało punktów do dekodowania");
        return -1;  // Za mało punktów
    }

    // Krok 1: Spróbuj interpolować z pierwszych 4 punktów
    lagrange_interpolate(pts, 4, coeffs);

    // Krok 2: Sprawdź czy wszystkie punkty pasują do wielomianu
    if (verify_points(pts, n, coeffs, 3)) {
        Serial.println("✓ Brak błędów - wszystkie punkty poprawne!");
        successful_corrections++;
        return 0;  // Brak błędów
    }

    Serial.println("⚠️  Wykryto niezgodności - próba korekcji...");

    // Krok 3: Są błędy - spróbuj znaleźć 1 błędny punkt
    // Testuj wszystkie kombinacje wykluczające po 1 punkcie
    for (int skip = 0; skip < n; skip++) {
        Point test_points[6];
        copy_points_except(pts, n, test_points, skip);

        // Interpoluj z 5 punktów (używając pierwszych 4)
        int test_coeffs[MAX_COEFFS];
        lagrange_interpolate(test_points, 4, test_coeffs);

        // Sprawdź czy wszystkie 5 punktów pasuje do wielomianu
        if (verify_points(test_points, n - 1, test_coeffs, 3)) {
            Serial.print("✓ KOREKCJA: Znaleziono błędny punkt #");
            Serial.println(skip);
            Serial.print("  Błędny punkt: (");
            Serial.print(pts[skip].x);
            Serial.print(", ");
            Serial.print(pts[skip].y);
            Serial.println(")");

            // Oblicz poprawną wartość
            int correct_y = evaluate_polynomial(test_coeffs, 3, pts[skip].x);
            Serial.print("  Poprawna wartość: (");
            Serial.print(pts[skip].x);
            Serial.print(", ");
            Serial.print(correct_y);
            Serial.println(")");

            // Skopiuj poprawne współczynniki
            for (int i = 0; i < MAX_COEFFS; i++) {
                coeffs[i] = test_coeffs[i];
            }

            *error_idx = skip;
            successful_corrections++;
            return 1;  // 1 błąd skorygowany
        }
    }

    // Krok 4: Nie udało się znaleźć 1 błędnego punktu - mamy 2 lub więcej
    // błędów
    Serial.println(
        "❌ BŁĄD: Wykryto 2 lub więcej błędów - nie można skorygować!");
    failed_corrections++;
    return 2;  // 2 lub więcej błędów
}

void setup() {
    Serial.begin(115200);
    softSerial.begin(9600);
    delay(2000);
    Serial.println("=== GF(31) RECEIVER READY (z korekcją błędów) ===");
    Serial.println("Możliwości:");
    Serial.println(
        "  - Wykrywanie błędów w x (duplikaty, wartości poza zakresem)");
    Serial.println("  - Wykrywanie błędów w y (niepoprawnę wartości)");
    Serial.println("  - Korekcja 1 błędu w y");
    Serial.println("  - Wykrywanie 2 lub więcej błędów");
    Serial.println();
}

void loop() {
    if (softSerial.available()) {
        uint8_t frame = softSerial.read();
        int x = (frame >> 5) & 0x07;
        int y = frame & 0x1F;

        Serial.print("Frame: 0x");
        Serial.print(frame, HEX);
        Serial.print(" -> x=");
        Serial.print(x);
        Serial.print(", y=");
        Serial.print(y);

        // Walidacja
        if (!is_valid_x(x)) {
            Serial.print(" ⚠️  [X poza zakresem!]");
        }
        if (!is_valid_y(y)) {
            Serial.print(" ⚠️  [Y poza zakresem!]");
        }
        Serial.println();

        points[count].x = x;
        points[count].y = y;
        count++;

        if (count == 6) {
            total_transmissions++;
            Serial.println("\n===============================================");
            Serial.print("📦 DEKODOWANIE TRANSMISJI #");
            Serial.println(total_transmissions);
            Serial.println("===============================================");
            Serial.println("Odebrane punkty:");
            for (int i = 0; i < 6; i++) {
                Serial.print("  [");
                Serial.print(i);
                Serial.print("] (");
                Serial.print(points[i].x);
                Serial.print(", ");
                Serial.print(points[i].y);
                Serial.print(")");

                if (!is_valid_x(points[i].x) || !is_valid_y(points[i].y)) {
                    Serial.print(" ⚠️  NIEPOPRAWNY");
                }
                Serial.println();
            }
            Serial.println();

            // Analiza rozkładu x
            analyzeXDistribution(points, 6);

            // Sprawdzenie czy są duplikaty x
            if (hasDuplicateX(points, 6)) {
                Serial.println(
                    "\n⚠️ UWAGA: Wykryto powtarzające się wartości x!");

                Point unique_points[6];
                int unique_count = selectUniquePoints(points, 6, unique_points);

                Serial.print("Wybrano ");
                Serial.print(unique_count);
                Serial.println(" unikalnych punktów:");
                for (int i = 0; i < unique_count; i++) {
                    Serial.print("  (");
                    Serial.print(unique_points[i].x);
                    Serial.print(", ");
                    Serial.print(unique_points[i].y);
                    Serial.println(")");
                }

                if (unique_count >= 4) {
                    int coeffs[MAX_COEFFS];
                    int error_idx;
                    int error_count = reed_solomon_decode(
                        unique_points, unique_count, coeffs, &error_idx);

                    if (error_count <= 1) {
                        Serial.println("\n📊 Policzony wielomian:");
                        for (int i = 0; i < MAX_COEFFS; i++) {
                            Serial.print("  a");
                            Serial.print(i);
                            Serial.print(" = ");
                            Serial.println(coeffs[i]);
                        }
                    }
                } else {
                    Serial.println(
                        "❌ Za mało unikalnych punktów do interpolacji "
                        "(minimum 4)");
                }
            } else {
                // Brak duplikatów - dekodowanie z korekcją błędów
                Serial.println("\n🔍 Analiza poprawności danych...");

                int coeffs[MAX_COEFFS];
                int error_idx;
                int error_count =
                    reed_solomon_decode(points, 6, coeffs, &error_idx);

                if (error_count == 0) {
                    Serial.println("\n📊 Policzony wielomian:");
                    for (int i = 0; i < MAX_COEFFS; i++) {
                        Serial.print("  a");
                        Serial.print(i);
                        Serial.print(" = ");
                        Serial.println(coeffs[i]);
                    }
                } else if (error_count == 1) {
                    Serial.println("\n📊 Policzony wielomian (po korekcji):");
                    for (int i = 0; i < MAX_COEFFS; i++) {
                        Serial.print("  a");
                        Serial.print(i);
                        Serial.print(" = ");
                        Serial.println(coeffs[i]);
                    }
                } else {
                    Serial.println(
                        "\n⛔ TRANSMISJA ODRZUCONA - za dużo błędów");
                }
            }

            Serial.println("=== END FRAME ===");

            // Statystyki
            Serial.println();
            Serial.println("📊 STATYSTYKI:");
            Serial.print("  Całkowite transmisje: ");
            Serial.println(total_transmissions);
            Serial.print("  Poprawne/skorygowane: ");
            Serial.println(successful_corrections);
            Serial.print("  Nieudane korekcje: ");
            Serial.println(failed_corrections);
            if (total_transmissions > 0) {
                Serial.print("  Współczynnik sukcesu: ");
                Serial.print((successful_corrections * 100) /
                             total_transmissions);
                Serial.println("%");
            }
            Serial.println();

            count = 0;
        }
    }
}
