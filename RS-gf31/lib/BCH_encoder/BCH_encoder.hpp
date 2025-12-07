#ifndef BCH_ENCODER_H
#define BCH_ENCODER_H

#include <stdint.h>

// Parametry BCH(31, 21, t=2)
#define M 5                 // GF(2^5)
#define N 31                // długość kodu
#define K 21                // liczba bitów informacji
#define T 2                 // liczba korygowanych błędów
#define GPOLY 0b100101      // x^5 + x^2 + 1 (wielomian pierwotny)

void bch_init();
uint32_t bch_encode(uint32_t data);  // 21 bitów wejście -> 31 bitów wyjście

#endif