#include <Arduino.h>
#include <SoftwareSerial.h>
#include "gf31_math.hpp"

SoftwareSerial softSerial(13, 12); // RX, TX

struct Point { int x; int y; };
Point points[6];
int count = 0;

// Funkcja sprawdzająca czy są duplikaty x
bool hasDuplicateX(Point *pts, int n) {
  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      if (pts[i].x == pts[j].x) {
        return true;
      }
    }
  }
  return false;
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
  if (n < 4) {
    return -1;
  }
  
  lagrange_interpolate(pts, 4, coeffs);
  
  if (verify_points(pts, n, coeffs, 3)) {
    Serial.println("✓ Brak błędów - wszystkie punkty poprawne!");
    return 0;
  }
  
  Serial.println("⚠️ Wykryto niezgodności - próba korekcji...");
  
  for (int skip = 0; skip < n; skip++) {
    Point test_points[6];
    copy_points_except(pts, n, test_points, skip);
    
    int test_coeffs[MAX_COEFFS];
    lagrange_interpolate(test_points, 4, test_coeffs);
    
    if (verify_points(test_points, n - 1, test_coeffs, 3)) {
      Serial.print("✓ KOREKCJA: Znaleziono błędny punkt #");
      Serial.println(skip);
      Serial.print("  Błędny punkt: (");
      Serial.print(pts[skip].x);
      Serial.print(", ");
      Serial.print(pts[skip].y);
      Serial.println(")");
      
      int correct_y = evaluate_polynomial(test_coeffs, 3, pts[skip].x);
      Serial.print("  Poprawna wartość: (");
      Serial.print(pts[skip].x);
      Serial.print(", ");
      Serial.print(correct_y);
      Serial.println(")");
      
      for (int i = 0; i < MAX_COEFFS; i++) {
        coeffs[i] = test_coeffs[i];
      }
      
      *error_idx = skip;
      return 1;
    }
  }
  
  Serial.println("❌ BŁĄD: Wykryto 2 lub więcej błędów - nie można skorygować!");
  return 2;
}

void setup() {
  Serial.begin(115200);
  softSerial.begin(9600);
  delay(2000);
  Serial.println("=== GF(31) RECEIVER READY (z korekcją błędów) ===");
}

void loop() {
  if (softSerial.available()) {
    uint8_t frame = softSerial.read();
    int x = (frame >> 5) & 0x07;
    int y = frame & 0x1F;

    Serial.print("Frame: 0x");
    Serial.print(frame, HEX);
    Serial.print(" -> x="); Serial.print(x);
    Serial.print(", y="); Serial.println(y);

    points[count].x = x;
    points[count].y = y;
    count++;

    if (count == 6) {
      Serial.println("\n=== ROZPOCZĘCIE DEKODOWANIA ===");
      Serial.println("Odebrane punkty:");
      for (int i = 0; i < 6; i++) {
        Serial.print("  ["); Serial.print(i);
        Serial.print("] ("); Serial.print(points[i].x);
        Serial.print(", "); Serial.print(points[i].y);
        Serial.println(")");
      }

      // Sprawdzenie czy są duplikaty x
      if (hasDuplicateX(points, 6)) {
        Serial.println("\n⚠️ UWAGA: Wykryto powtarzające się wartości x!");
        
        Point unique_points[6];
        int unique_count = selectUniquePoints(points, 6, unique_points);
        
        Serial.print("Wybrano "); Serial.print(unique_count);
        Serial.println(" unikalnych punktów:");
        for (int i = 0; i < unique_count; i++) {
          Serial.print("  ("); Serial.print(unique_points[i].x);
          Serial.print(", "); Serial.print(unique_points[i].y);
          Serial.println(")");
        }
        
        if (unique_count >= 4) {
          int coeffs[MAX_COEFFS];
          int error_idx;
          int error_count = reed_solomon_decode(unique_points, unique_count, coeffs, &error_idx);
          
          if (error_count <= 1) {
            Serial.println("\n📊 Policzony wielomian:");
            for (int i = 0; i < MAX_COEFFS; i++) {
              Serial.print("  a"); Serial.print(i);
              Serial.print(" = "); Serial.println(coeffs[i]);
            }
          }
        } else {
          Serial.println("❌ Za mało unikalnych punktów do interpolacji (minimum 4)");
        }
      } else {
        // Brak duplikatów - dekodowanie z korekcją błędów
        Serial.println("\n🔍 Analiza poprawności danych...");
        
        int coeffs[MAX_COEFFS];
        int error_idx;
        int error_count = reed_solomon_decode(points, 6, coeffs, &error_idx);
        
        if (error_count == 0) {
          Serial.println("\n📊 Policzony wielomian:");
          for (int i = 0; i < MAX_COEFFS; i++) {
            Serial.print("  a"); Serial.print(i);
            Serial.print(" = "); Serial.println(coeffs[i]);
          }
        } else if (error_count == 1) {
          Serial.println("\n📊 Policzony wielomian (po korekcji):");
          for (int i = 0; i < MAX_COEFFS; i++) {
            Serial.print("  a"); Serial.print(i);
            Serial.print(" = "); Serial.println(coeffs[i]);
          }
        } else {
          Serial.println("\n⛔ TRANSMISJA ODRZUCONA - za dużo błędów");
        }
      }

      Serial.println("=== END FRAME ===\n");
      count = 0;
    }
  }
}