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
int clean_transmissions = 0;      // Bez błędów
int corrected_transmissions = 0;  // 1 błąd skorygowany
int failed_corrections = 0;       // 2+ błędy, nie można skorygować

const int MESSAGES_PER_TEST = 1000;
bool test_in_progress = false;
bool test_completed = false;
unsigned long first_message_time = 0;

void print_test_summary() {
    unsigned long test_duration =
        (millis() - first_message_time) / 1000;  // w sekundach

    Serial.println("\n\n");
    Serial.println(
        "═════════════════════════════════════════════════════════════════════"
        "═");
    Serial.println("              PODSUMOWANIE TESTU - 1000 WIADOMOŚCI");
    Serial.println(
        "═════════════════════════════════════════════════════════════════════"
        "═");
    Serial.println();

    Serial.println(
        "╔═══════════════════════════════════════════════════════════════════"
        "╗");
    Serial.println(
        "║                         WYNIKI TESTU                              "
        "║");
    Serial.println(
        "╠═════════════════════════════════╦═════════════════════════════════"
        "╣");
    Serial.println(
        "║ Kategoria                       ║  Liczba        Procent      ║");
    Serial.println(
        "╠═════════════════════════════════╬═════════════════════════════════"
        "╣");

    // Oblicz procenty
    float clean_percent = (clean_transmissions * 100.0) / MESSAGES_PER_TEST;
    float corrected_percent =
        (corrected_transmissions * 100.0) / MESSAGES_PER_TEST;
    float failed_percent = (failed_corrections * 100.0) / MESSAGES_PER_TEST;
    float success_percent =
        ((clean_transmissions + corrected_transmissions) * 100.0) /
        MESSAGES_PER_TEST;

    // OK (czyste)
    Serial.print("║ ✓ OK (bez błędów)             ║  ");
    if (clean_transmissions < 10)
        Serial.print("  ");
    else if (clean_transmissions < 100)
        Serial.print(" ");
    Serial.print(clean_transmissions);
    Serial.print("/1000    ");
    if (clean_percent < 10) Serial.print(" ");
    Serial.print(clean_percent, 1);
    Serial.println("%      ║");

    // CORRECTED (skorygowane)
    Serial.print("║ ✓ CORRECTED (1 błąd)          ║  ");
    if (corrected_transmissions < 10)
        Serial.print("  ");
    else if (corrected_transmissions < 100)
        Serial.print(" ");
    Serial.print(corrected_transmissions);
    Serial.print("/1000    ");
    if (corrected_percent < 10) Serial.print(" ");
    Serial.print(corrected_percent, 1);
    Serial.println("%      ║");

    // DETECTED (nie skorygowane)
    Serial.print("║ ❌ DETECTED (2+ błędy)         ║  ");
    if (failed_corrections < 10)
        Serial.print("  ");
    else if (failed_corrections < 100)
        Serial.print(" ");
    Serial.print(failed_corrections);
    Serial.print("/1000    ");
    if (failed_percent < 10) Serial.print(" ");
    Serial.print(failed_percent, 1);
    Serial.println("%      ║");

    Serial.println(
        "╠═════════════════════════════════╬═════════════════════════════════"
        "╣");

    // Całkowity sukces
    int successful = clean_transmissions + corrected_transmissions;
    Serial.print("║ 🎯 WSPÓŁCZYNNIK SUKCESU         ║  ");
    if (successful < 10)
        Serial.print("  ");
    else if (successful < 100)
        Serial.print(" ");
    Serial.print(successful);
    Serial.print("/1000    ");
    if (success_percent < 10) Serial.print(" ");
    Serial.print(success_percent, 1);
    Serial.println("%      ║");

    Serial.println(
        "╚═════════════════════════════════╩═════════════════════════════════"
        "╝");
    Serial.println();

    // Dodatkowe informacje
    Serial.println("🕒 Informacje o teście:");
    Serial.print("   Całkowity czas testu: ");
    Serial.print(test_duration);
    Serial.println(" sekund");

    if (test_duration > 0) {
        Serial.print("   Prędkość: ");
        Serial.print(MESSAGES_PER_TEST / test_duration);
        Serial.println(" wiadomości/s");
    }

    Serial.println();
    Serial.println(
        "═════════════════════════════════════════════════════════════════════"
        "═");
    Serial.println("💤 Oczekiwanie na resetowanie...");
    Serial.println();
}

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
        clean_transmissions++;
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
            corrected_transmissions++;
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

    Serial.println(
        "═════════════════════════════════════════════════════════════════════"
        "═");
    Serial.println("       GF(31) RECEIVER - TEST 1000 WIADOMOŚCI");
    Serial.println(
        "═════════════════════════════════════════════════════════════════════"
        "═");
    Serial.println();
    Serial.println("🔍 Funkcje receivera:");
    Serial.println(
        "  - Wykrywanie błędów w x (duplikaty, wartości poza zakresem)");
    Serial.println("  - Wykrywanie błędów w y (niepoprawnę wartości)");
    Serial.println("  - Korekcja 1 błędu w y (Reed-Solomon)");
    Serial.println("  - Wykrywanie 2 lub więcej błędów");
    Serial.println();
    Serial.println("⏳ Oczekiwanie na pierwszą transmisję...");
    Serial.println();
}

void loop() {
    if (test_completed) {
        // Test zakończony, czekaj na reset
        return;
    }

    if (softSerial.available()) {
        uint8_t frame = softSerial.read();
        int x = (frame >> 5) & 0x07;
        int y = frame & 0x1F;

        // Rozpocznij test przy pierwszej wiadomości
        if (!test_in_progress) {
            test_in_progress = true;
            first_message_time = millis();
            Serial.println("🚀 ROZPOCZĘTO ODBIERANIE WIADOMOŚCI");
            Serial.println();
        }

        points[count].x = x;
        points[count].y = y;
        count++;

        if (count == 6) {
            total_transmissions++;

            // Wyświetl postęp co 100 wiadomości
            if (total_transmissions % 100 == 0) {
                Serial.print("📊 Postęp: ");
                Serial.print(total_transmissions);
                Serial.println("/1000 wiadomości");
            }

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

                    if (error_count == 0) {
                        clean_transmissions++;
                    } else if (error_count == 1) {
                        corrected_transmissions++;
                    } else {
                        failed_corrections++;
                    }
                } else {
                    failed_corrections++;
                }
            } else {
                // Brak duplikatów - dekodowanie z korekcją błędów
                int coeffs[MAX_COEFFS];
                int error_idx;
                int error_count =
                    reed_solomon_decode(points, 6, coeffs, &error_idx);

                if (error_count == 0) {
                    clean_transmissions++;
                } else if (error_count == 1) {
                    corrected_transmissions++;
                } else {
                    failed_corrections++;
                }
            }

            count = 0;

            // Sprawdź czy test się zakończył
            if (total_transmissions >= MESSAGES_PER_TEST) {
                test_completed = true;
                print_test_summary();
            }
        }
    }
}
