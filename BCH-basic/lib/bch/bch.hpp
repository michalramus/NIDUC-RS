#pragma once
#include <Arduino.h>

// BCH(15,7,2) parameters
#define BCH_N 15        // Codeword length
#define BCH_K 7         // Message length
#define BCH_T 2         // Error correction capability

// GF(2^4) field operations
class GF16 {
public:
    static uint8_t add(uint8_t a, uint8_t b);
    static uint8_t multiply(uint8_t a, uint8_t b);
    static uint8_t divide(uint8_t a, uint8_t b);
    static uint8_t power(uint8_t base, uint8_t exp);
    static uint8_t inverse(uint8_t a);
};

// BCH encoder/decoder class
class BCH {
public:
    // Encode 7-bit message to 15-bit codeword
    static void encode(uint8_t message[BCH_K], uint8_t codeword[BCH_N]);
    
    // Decode 15-bit received codeword
    static bool decode(uint8_t received[BCH_N], uint8_t message[BCH_K]);
    
    // Calculate syndrome
    static void calculateSyndrome(uint8_t received[BCH_N], uint8_t syndrome[2*BCH_T]);
    
    // Find error locations using Berlekamp-Massey algorithm
    static int findErrorLocations(uint8_t syndrome[2*BCH_T], uint8_t errorLocations[BCH_T]);
    
    // Correct errors in the received codeword
    static void correctErrors(uint8_t received[BCH_N], uint8_t errorLocations[BCH_T], int numErrors);
    
    // Helper functions
    static void printBinary(uint8_t data[], int length);
    static uint8_t computeParity(uint8_t message[BCH_K]);
};
