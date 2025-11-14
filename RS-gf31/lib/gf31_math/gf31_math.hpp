#pragma once
#include <Arduino.h>

#define MOD 31
#define MAX_COEFFS 4

// Dodawanie w GF(31)
inline int gf_add(int a, int b) {
  int r = (a + b) % MOD;
  if (r < 0) r += MOD;
  return r;
}

// Mnożenie w GF(31)
inline int gf_mul(int a, int b) {
  int r = (a * b) % MOD;
  if (r < 0) r += MOD;
  return r;
}

// Szybkie potęgowanie (x^p mod 31)
inline int gf_pow(int base, int exp) {
  int res = 1;
  while (exp > 0) {
    if (exp & 1) res = gf_mul(res, base);
    base = gf_mul(base, base);
    exp >>= 1;
  }
  return res;
}

// Odwrotność w GF(31)
inline int gf_inv(int a) {
  // ponieważ 31 to liczba pierwsza → a^(p−2) mod p
  return gf_pow(a, MOD - 2);
}