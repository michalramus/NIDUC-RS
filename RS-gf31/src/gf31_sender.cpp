#include <Arduino.h>
#include "gf31_math.hpp"

// Współczynniki wielomianu 3-go stopnia: y = a0 + a1*x + a2*x^2 + a3*x^3 (mod 31)
int coeffs[MAX_COEFFS] = {5, 7, 3, 2};  // przykładowo

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
  // 8-bitowa ramka: [7..5] = x, [4..0] = y
  uint8_t frame = ((x & 0x07) << 5) | (y & 0x1F);
  Serial.write(frame);
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("=== GF(31) SENDER START ===");
}

void loop() {
  for (int x = 0; x < 6; x++) {
    int y = poly_eval(x);
    Serial.print("x=");
    Serial.print(x);
    Serial.print(" y=");
    Serial.print(y);
    Serial.print("  frame=0x");
    Serial.println(((x << 5) | y), HEX);
    send_point(x, y);
    delay(500);
  }
  Serial.println("=== Transmission complete ===\n");
  delay(5000);
}