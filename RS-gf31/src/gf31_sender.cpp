#include <Arduino.h>
#include <SoftwareSerial.h>

#include "gf31_math.hpp"

// TX -> pin 12, RX -> pin 13
SoftwareSerial softSerial(13, 12);  // RX, TX D7, D6

int coeffs[MAX_COEFFS] = {5, 7, 3, 2};

// Tryby transmisji:
// 0 = dobra (bez błędów)
// 1 = 1 błąd w y
// 2 = 2 błędy w y
// 3 = 1 błąd w x
// 4 = 1 błąd w x i 1 błąd w y
// 5 = 2 błędy w x
int transmission_mode = 0;

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

// Funkcja wprowadzająca błąd do wartości y
int introduce_y_error(int y) {
    // Dodaj losową wartość od 1 do 30 (w GF(31))
    int error = (random(1, 31));
    return gf_add(y, error);
}

// Funkcja wprowadzająca błąd do wartości x
int introduce_x_error(int x) {
    // Zmień x na inną losową wartość z zakresu [0, 5]
    int new_x = random(0, 6);
    // Upewnij się, że jest inna niż oryginalna
    while (new_x == x) {
        new_x = random(0, 6);
    }
    return new_x;
}

void send_transmission() {
    int x_values[6] = {0, 1, 2, 3, 4, 5};  // Domyślne wartości x
    int y_values[6];
    bool is_x_error[6] = {false, false, false, false, false, false};
    bool is_y_error[6] = {false, false, false, false, false, false};

    // Oblicz wszystkie poprawne wartości y
    for (int x = 0; x < 6; x++) {
        y_values[x] = poly_eval(x);
    }

    // Wprowadź błędy w zależności od trybu
    if (transmission_mode == 1) {
        // 1 błąd w y - w losowej pozycji
        int error_position = random(0, 6);
        int original_y = y_values[error_position];
        y_values[error_position] = introduce_y_error(y_values[error_position]);
        is_y_error[error_position] = true;

        Serial.println("⚠️  WPROWADZAM 1 BŁĄD W Y");
        Serial.print("   Błąd w punkcie #");
        Serial.print(error_position);
        Serial.print(" x=");
        Serial.print(x_values[error_position]);
        Serial.print(" (poprawne y=");
        Serial.print(original_y);
        Serial.print(", błędne y=");
        Serial.print(y_values[error_position]);
        Serial.println(")");

    } else if (transmission_mode == 2) {
        // 2 błędy w y - w losowych pozycjach
        int error_pos1 = random(0, 6);
        int error_pos2 = random(0, 6);

        while (error_pos2 == error_pos1) {
            error_pos2 = random(0, 6);
        }

        int original_y1 = y_values[error_pos1];
        int original_y2 = y_values[error_pos2];

        y_values[error_pos1] = introduce_y_error(y_values[error_pos1]);
        y_values[error_pos2] = introduce_y_error(y_values[error_pos2]);
        is_y_error[error_pos1] = true;
        is_y_error[error_pos2] = true;

        Serial.println("❌❌ WPROWADZAM 2 BŁĘDY W Y");
        Serial.print("   Błąd 1 w punkcie #");
        Serial.print(error_pos1);
        Serial.print(" x=");
        Serial.print(x_values[error_pos1]);
        Serial.print(" (poprawne y=");
        Serial.print(original_y1);
        Serial.print(", błędne y=");
        Serial.print(y_values[error_pos1]);
        Serial.println(")");

        Serial.print("   Błąd 2 w punkcie #");
        Serial.print(error_pos2);
        Serial.print(" x=");
        Serial.print(x_values[error_pos2]);
        Serial.print(" (poprawne y=");
        Serial.print(original_y2);
        Serial.print(", błędne y=");
        Serial.print(y_values[error_pos2]);
        Serial.println(")");

    } else if (transmission_mode == 3) {
        // 1 błąd w x - w losowej pozycji
        int error_position = random(0, 6);
        int original_x = x_values[error_position];
        x_values[error_position] = introduce_x_error(x_values[error_position]);
        is_x_error[error_position] = true;

        Serial.println("⚠️  WPROWADZAM 1 BŁĄD W X");
        Serial.print("   Błąd w punkcie #");
        Serial.print(error_position);
        Serial.print(" (poprawne x=");
        Serial.print(original_x);
        Serial.print(", błędne x=");
        Serial.print(x_values[error_position]);
        Serial.print(", y=");
        Serial.print(y_values[error_position]);
        Serial.println(")");
        Serial.println(
            "   ⚠️  UWAGA: To spowoduje duplikat x lub niepoprawną wartość!");

    } else if (transmission_mode == 4) {
        // 1 błąd w x i 1 błąd w y - w różnych pozycjach
        int x_error_pos = random(0, 6);
        int y_error_pos = random(0, 6);

        while (y_error_pos == x_error_pos) {
            y_error_pos = random(0, 6);
        }

        int original_x = x_values[x_error_pos];
        int original_y = y_values[y_error_pos];

        x_values[x_error_pos] = introduce_x_error(x_values[x_error_pos]);
        y_values[y_error_pos] = introduce_y_error(y_values[y_error_pos]);
        is_x_error[x_error_pos] = true;
        is_y_error[y_error_pos] = true;

        Serial.println("❌❌ WPROWADZAM 1 BŁĄD W X i 1 BŁĄD W Y");
        Serial.print("   Błąd X w punkcie #");
        Serial.print(x_error_pos);
        Serial.print(" (poprawne x=");
        Serial.print(original_x);
        Serial.print(", błędne x=");
        Serial.print(x_values[x_error_pos]);
        Serial.println(")");

        Serial.print("   Błąd Y w punkcie #");
        Serial.print(y_error_pos);
        Serial.print(" x=");
        Serial.print(x_values[y_error_pos]);
        Serial.print(" (poprawne y=");
        Serial.print(original_y);
        Serial.print(", błędne y=");
        Serial.print(y_values[y_error_pos]);
        Serial.println(")");

    } else if (transmission_mode == 5) {
        // 2 błędy w x - w losowych pozycjach
        int error_pos1 = random(0, 6);
        int error_pos2 = random(0, 6);

        while (error_pos2 == error_pos1) {
            error_pos2 = random(0, 6);
        }

        int original_x1 = x_values[error_pos1];
        int original_x2 = x_values[error_pos2];

        x_values[error_pos1] = introduce_x_error(x_values[error_pos1]);
        x_values[error_pos2] = introduce_x_error(x_values[error_pos2]);
        is_x_error[error_pos1] = true;
        is_x_error[error_pos2] = true;

        Serial.println("❌❌❌ WPROWADZAM 2 BŁĘDY W X");
        Serial.print("   Błąd 1 w punkcie #");
        Serial.print(error_pos1);
        Serial.print(" (poprawne x=");
        Serial.print(original_x1);
        Serial.print(", błędne x=");
        Serial.print(x_values[error_pos1]);
        Serial.println(")");

        Serial.print("   Błąd 2 w punkcie #");
        Serial.print(error_pos2);
        Serial.print(" (poprawne x=");
        Serial.print(original_x2);
        Serial.print(", błędne x=");
        Serial.print(x_values[error_pos2]);
        Serial.println(")");
        Serial.println("   ⚠️  UWAGA: To spowoduje poważne błędy w x!");
    }

    Serial.println();

    // Wyślij wszystkie punkty
    for (int i = 0; i < 6; i++) {
        Serial.print("Wysyłam punkt #");
        Serial.print(i);
        Serial.print(": x=");
        Serial.print(x_values[i]);
        Serial.print(" y=");
        Serial.print(y_values[i]);

        if (is_x_error[i] && is_y_error[i]) {
            Serial.print(" 🔴🔴 [BŁĄD X+Y]");
        } else if (is_x_error[i]) {
            Serial.print(" 🔴 [BŁĄD X]");
        } else if (is_y_error[i]) {
            Serial.print(" 🔴 [BŁĄD Y]");
        } else {
            Serial.print(" ✓");
        }

        Serial.print(" frame=0x");
        Serial.println(((x_values[i] << 5) | y_values[i]), HEX);

        send_point(x_values[i], y_values[i]);
        delay(500);
    }
}

void setup() {
    Serial.begin(115200);
    softSerial.begin(9600);
    randomSeed(analogRead(0));  // Inicjalizacja generatora liczb losowych
    delay(2000);
    Serial.println("=== GF(31) SENDER START - ROZSZERZONE TESTY ===");
    Serial.println("Cykl transmisji co 10s:");
    Serial.println("  0: DOBRA (bez błędów)");
    Serial.println("  1: 1 BŁĄD w Y");
    Serial.println("  2: 2 BŁĘDY w Y");
    Serial.println("  3: 1 BŁĄD w X");
    Serial.println("  4: 1 BŁĄD w X + 1 BŁĄD w Y");
    Serial.println("  5: 2 BŁĘDY w X");
    Serial.println();
}

void loop() {
    // Wyświetl nagłówek dla aktualnej transmisji
    Serial.println("=============================================");
    Serial.print("TRYB ");
    Serial.print(transmission_mode);
    Serial.print(": ");
    switch (transmission_mode) {
        case 0:
            Serial.println("✓✓✓ TRANSMISJA POPRAWNA (bez błędów)");
            break;
        case 1:
            Serial.println("⚠️  TRANSMISJA Z 1 BŁĘDEM W Y");
            break;
        case 2:
            Serial.println("❌❌ TRANSMISJA Z 2 BŁĘDAMI W Y");
            break;
        case 3:
            Serial.println("⚠️  TRANSMISJA Z 1 BŁĘDEM W X");
            break;
        case 4:
            Serial.println("❌❌ TRANSMISJA Z 1 BŁĘDEM W X i 1 W Y");
            break;
        case 5:
            Serial.println("❌❌❌ TRANSMISJA Z 2 BŁĘDAMI W X");
            break;
    }
    Serial.println("=============================================");

    // Wyślij transmisję
    send_transmission();

    Serial.println("\n=== Transmission complete ===");

    // Przełącz na kolejny tryb (0 -> 1 -> 2 -> 3 -> 4 -> 5 -> 0 -> ...)
    transmission_mode = (transmission_mode + 1) % 6;

    // Czekaj 10 sekund przed następną transmisją
    Serial.print("\n⏳ Następna transmisja za 10 sekund (tryb ");
    Serial.print(transmission_mode);
    Serial.print(": ");
    switch (transmission_mode) {
        case 0:
            Serial.println("DOBRA)...");
            break;
        case 1:
            Serial.println("1 BŁĄD W Y)...");
            break;
        case 2:
            Serial.println("2 BŁĘDY W Y)...");
            break;
        case 3:
            Serial.println("1 BŁĄD W X)...");
            break;
        case 4:
            Serial.println("1 BŁĄD W X + 1 W Y)...");
            break;
        case 5:
            Serial.println("2 BŁĘDY W X)...");
            break;
    }
    Serial.println();

    delay(10000);  // 10 sekund
}
