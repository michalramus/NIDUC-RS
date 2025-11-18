#include <Arduino.h>
#include <SoftwareSerial.h>
#include "gf31_math.hpp"

// TX -> pin 12, RX -> pin 13
SoftwareSerial softSerial(13, 12);  // RX, TX D7, D6

int coeffs[MAX_COEFFS] = {5, 7, 3, 2};

int transmission_mode = 0;  // 0 = dobra, 1 = 1 błąd, 2 = 2 błędy

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
int introduce_error(int y) {
  // Dodaj losową wartość od 1 do 30 (w GF(31))
  int error = (random(1, 31));
  return gf_add(y, error);
}

void send_transmission() {
  int y_values[6];
  bool is_error[6] = {false, false, false, false, false, false};
  
  // Oblicz wszystkie poprawne wartości
  for (int x = 0; x < 6; x++) {
    y_values[x] = poly_eval(x);
  }
  
  // Wprowadź błędy w zależności od trybu
  if (transmission_mode == 1) {
    // 1 błąd - w losowej pozycji
    int error_position = random(0, 6);
    int original_y = y_values[error_position];
    y_values[error_position] = introduce_error(y_values[error_position]);
    is_error[error_position] = true;
    
    Serial.println("⚠️  WPROWADZAM 1 BŁĄD");
    Serial.print("   Błąd w punkcie x=");
    Serial.print(error_position);
    Serial.print(" (poprawne y=");
    Serial.print(original_y);
    Serial.print(", błędne y=");
    Serial.print(y_values[error_position]);
    Serial.println(")");
    
  } else if (transmission_mode == 2) {
    // 2 błędy - w losowych pozycjach
    int error_pos1 = random(0, 6);
    int error_pos2 = random(0, 6);
    
    // Upewnij się że pozycje są różne
    while (error_pos2 == error_pos1) {
      error_pos2 = random(0, 6);
    }
    
    int original_y1 = y_values[error_pos1];
    int original_y2 = y_values[error_pos2];
    
    y_values[error_pos1] = introduce_error(y_values[error_pos1]);
    y_values[error_pos2] = introduce_error(y_values[error_pos2]);
    is_error[error_pos1] = true;
    is_error[error_pos2] = true;
    
    Serial.println("❌❌ WPROWADZAM 2 BŁĘDY");
    Serial.print("   Błąd 1 w punkcie x=");
    Serial.print(error_pos1);
    Serial.print(" (poprawne y=");
    Serial.print(original_y1);
    Serial.print(", błędne y=");
    Serial.print(y_values[error_pos1]);
    Serial.println(")");
    
    Serial.print("   Błąd 2 w punkcie x=");
    Serial.print(error_pos2);
    Serial.print(" (poprawne y=");
    Serial.print(original_y2);
    Serial.print(", błędne y=");
    Serial.print(y_values[error_pos2]);
    Serial.println(")");
  }
  
  Serial.println();
  
  // Wyślij wszystkie punkty
  for (int x = 0; x < 6; x++) {
    Serial.print("Wysyłam: x="); 
    Serial.print(x);
    Serial.print(" y="); 
    Serial.print(y_values[x]);
    
    if (is_error[x]) {
      Serial.print(" 🔴 [BŁĄD]");
    } else {
      Serial.print(" ✓");
    }
    
    Serial.print(" frame=0x"); 
    Serial.println(((x << 5) | y_values[x]), HEX);
    
    send_point(x, y_values[x]);
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);
  softSerial.begin(9600);
  randomSeed(analogRead(0));  // Inicjalizacja generatora liczb losowych
  delay(2000);
  Serial.println("=== GF(31) SENDER START ===");
  Serial.println("Cykl transmisji co 10s: DOBRA -> 1 BŁĄD -> 2 BŁĘDY");
  Serial.println();
}

void loop() {
  // Wyświetl nagłówek dla aktualnej transmisji
  Serial.println("=====================================");
  switch (transmission_mode) {
    case 0:
      Serial.println("✓✓✓ TRANSMISJA POPRAWNA (bez błędów)");
      break;
    case 1:
      Serial.println("⚠️  TRANSMISJA Z 1 BŁĘDEM");
      break;
    case 2:
      Serial.println("❌❌ TRANSMISJA Z 2 BŁĘDAMI");
      break;
  }
  Serial.println("=====================================");
  
  // Wyślij transmisję
  send_transmission();
  
  Serial.println("\n=== Transmission complete ===");
  
  // Przełącz na kolejny tryb (0 -> 1 -> 2 -> 0 -> ...)
  transmission_mode = (transmission_mode + 1) % 3;
  
  // Czekaj 10 sekund przed następną transmisją
  Serial.print("\n⏳ Następna transmisja za 10 sekund (tryb: ");
  switch (transmission_mode) {
    case 0:
      Serial.println("DOBRA)...");
      break;
    case 1:
      Serial.println("1 BŁĄD)...");
      break;
    case 2:
      Serial.println("2 BŁĘDY)...");
      break;
  }
  Serial.println();
  
  delay(10000);  // 10 sekund
}