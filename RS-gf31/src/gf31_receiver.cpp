#include <Arduino.h>
#include <SoftwareSerial.h>
#include "gf31_math.hpp"

#define MOD 31
#define MAX_COEFFS 4

SoftwareSerial softSerial(13, 12);  // RX, TX

struct Point { int x; int y; };
Point points[6];
int count = 0;

// GF(31) arytmetyka (zachowana z poprzedniego)
int gf_add(int a, int b) { int r = (a + b) % MOD; if (r < 0) r += MOD; return r; }
int gf_mul(int a, int b) { int r = (a * b) % MOD; if (r < 0) r += MOD; return r; }
int gf_pow(int base, int exp) {
  int res = 1;
  while (exp > 0) {
    if (exp & 1) res = gf_mul(res, base);
    base = gf_mul(base, base);
    exp >>= 1;
  }
  return res;
}
int gf_inv(int a) { return gf_pow(a, MOD - 2); }

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

void setup() {
  Serial.begin(115200);
  softSerial.begin(9600);
  delay(2000);
  Serial.println("=== GF(31) RECEIVER READY ===");
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
    Serial.println(y);

    points[count].x = x;
    points[count].y = y;
    count++;

    if (count == 6) {
      int coeffs[MAX_COEFFS];
      lagrange_interpolate(points, 4, coeffs);

      Serial.println("\nOdebrane punkty:");
      for (int i = 0; i < 6; i++) {
        Serial.print("(");
        Serial.print(points[i].x);
        Serial.print(", ");
        Serial.print(points[i].y);
        Serial.println(")");
      }

      Serial.println("Policzony wielomian:");
      for (int i = 0; i < MAX_COEFFS; i++) {
        Serial.print("a");
        Serial.print(i);
        Serial.print(" = ");
        Serial.println(coeffs[i]);
      }

      Serial.println("=== END FRAME ===\n");
      count = 0;
    }
  }
}