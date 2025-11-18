#include <Arduino.h>
#include <SoftwareSerial.h>
#include "gf31_math.hpp"

// TX -> pin 12, RX -> pin 13
SoftwareSerial softSerial(13, 12);  // RX, TX D7, D6

int coeffs[MAX_COEFFS] = {5, 7, 3, 2};

// ========== PARAMETRY SYMULACJI BŁĘDÓW ==========
int ERROR_X_COUNT = 0;        // Ilość błędów w części x ramki (0-3 bity)
int ERROR_Y_COUNT = 1;        // Ilość błędów w części y ramki (0-5 bitów)
int ERROR_FRAME_COUNT = 1;    // W ilu ramkach wprowadzić błędy (0-6)
// =================================================

bool error_frames[6] = {false, false, false, false, false, false};

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

// Inicjalizacja ramek z błędami
void initialize_error_frames() {
  // Wyzeruj tablicę
  for (int i = 0; i < 6; i++) {
    error_frames[i] = false;
  }
  
  // Losowo wybierz ramki które będą zawierać błędy
  if (ERROR_FRAME_COUNT > 0 && ERROR_FRAME_COUNT <= 6) {
    int selected = 0;
    while (selected < ERROR_FRAME_COUNT) {
      int idx = random(0, 6);
      if (!error_frames[idx]) {
        error_frames[idx] = true;
        selected++;
      }
    }
  }
  
  Serial.println("\n🎲 KONFIGURACJA BŁĘDÓW DLA TRANSMISJI:");
  Serial.print("   Błędów w części X: "); Serial.println(ERROR_X_COUNT);
  Serial.print("   Błędów w części Y: "); Serial.println(ERROR_Y_COUNT);
  Serial.print("   Ramki z błędami: ");
  
  bool first = true;
  for (int i = 0; i < 6; i++) {
    if (error_frames[i]) {
      if (!first) Serial.print(", ");
      Serial.print(i);
      first = false;
    }
  }
  if (first) Serial.print("brak");
  Serial.println();
}

// Wprowadza błędy do ramki
void introduce_frame_errors(int frame_idx, int &x, int &y) {
  if (!error_frames[frame_idx]) {
    return; // Ta ramka nie ma mieć błędów
  }
  
  int original_x = x;
  int original_y = y;
  bool x_error = false;
  bool y_error = false;
  
  // Wprowadź błędy w x
  if (ERROR_X_COUNT > 0) {
    for (int i = 0; i < ERROR_X_COUNT; i++) {
      int bit_to_flip = random(0, 3); // x ma 3 bity
      x ^= (1 << bit_to_flip);
      x &= 0x07; // Upewnij się że pozostaje w zakresie 0-7
    }
    if (x != original_x) {
      x_error = true;
    }
  }
  
  // Wprowadź błędy w y
  if (ERROR_Y_COUNT > 0) {
    for (int i = 0; i < ERROR_Y_COUNT; i++) {
      int bit_to_flip = random(0, 5); // y ma 5 bitów
      y ^= (1 << bit_to_flip);
      y &= 0x1F; // Upewnij się że pozostaje w zakresie 0-31
    }
    if (y != original_y) {
      y_error = true;
    }
  }
  
  // Wyświetl informacje o błędach
  if (x_error || y_error) {
    Serial.print("   🔴 WPROWADZAM BŁĘDY w ramce #");
    Serial.print(frame_idx);
    Serial.println(":");
    
    if (x_error) {
      Serial.print("      X: ");
      Serial.print(original_x);
      Serial.print(" → ");
      Serial.print(x);
      Serial.print(" (flipowano ");
      Serial.print(ERROR_X_COUNT);
      Serial.print(" bit");
      if (ERROR_X_COUNT > 1) Serial.print("y");
      Serial.println(")");
    }
    
    if (y_error) {
      Serial.print("      Y: ");
      Serial.print(original_y);
      Serial.print(" → ");
      Serial.print(y);
      Serial.print(" (flipowano ");
      Serial.print(ERROR_Y_COUNT);
      Serial.print(" bit");
      if (ERROR_Y_COUNT > 1) Serial.print("y/ów");
      Serial.println(")");
    }
  }
}

void send_transmission() {
  initialize_error_frames();
  
  Serial.println("\n📤 ROZPOCZYNAM TRANSMISJĘ:");
  Serial.println("=====================================");
  
  for (int x = 0; x < 6; x++) {
    int y = poly_eval(x);
    int original_x = x;
    int original_y = y;
    
    // Wprowadź błędy jeśli to ramka z błędami
    introduce_frame_errors(x, x, y);
    
    Serial.print("Ramka #"); Serial.print(original_x);
    Serial.print(": x="); Serial.print(x);
    Serial.print(" y="); Serial.print(y);
    
    if (original_x != x || original_y != y) {
      Serial.print(" (oryg: x=");
      Serial.print(original_x);
      Serial.print(" y=");
      Serial.print(original_y);
      Serial.print(") 🔴");
    } else {
      Serial.print(" ✓");
    }
    
    Serial.print(" | frame=0x"); 
    Serial.println(((x << 5) | y), HEX);
    
    send_point(x, y);
    delay(500);
  }
  
  Serial.println("=====================================");
  Serial.println("=== Transmission complete ===\n");
}

void setup() {
  Serial.begin(115200);
  softSerial.begin(9600);
  randomSeed(analogRead(0));
  delay(2000);
  
  Serial.println("=== GF(31) SENDER START ===");
  Serial.println("\n📋 PARAMETRY POCZĄTKOWE:");
  Serial.print("   Błędy w X: "); Serial.println(ERROR_X_COUNT);
  Serial.print("   Błędy w Y: "); Serial.println(ERROR_Y_COUNT);
  Serial.print("   Liczba ramek z błędami: "); Serial.println(ERROR_FRAME_COUNT);
  Serial.println("\n🔄 Transmisja co 10 sekund\n");
}

void loop() {
  send_transmission();
  
  Serial.println("⏳ Następna transmisja za 10 sekund...\n");
  delay(10000);  // 10 sekund
}