#pragma once
#include <Arduino.h>

// Maximum supported values for GF(2^4)
#define MAX_N 15  // Max codeword length for GF(2^4)
#define MAX_T 4   // Max error correction capability
#define MAX_K 15  // Max message length

// GF(2^4) field operations
class GF16 {
   public:
    static uint8_t add(uint8_t a, uint8_t b);
    static uint8_t multiply(uint8_t a, uint8_t b);
    static uint8_t divide(uint8_t a, uint8_t b);
    static uint8_t power(uint8_t base, uint8_t exp);
    static uint8_t inverse(uint8_t a);
    static uint8_t log(uint8_t a);      // Discrete logarithm
    static uint8_t antilog(uint8_t i);  // Antilogarithm (alpha^i)
};

// BCH encoder/decoder class with configurable parameters
class BCH {
   private:
    int n;  // Codeword length
    int k;  // Message length
    int t;  // Error correction capability

    uint8_t generatorPoly[MAX_N];  // Generator polynomial coefficients
    int generatorDegree;           // Degree of generator polynomial

    // Build minimal polynomials and generator polynomial
    void buildGeneratorPolynomial();

    // Polynomial operations
    void polyMultiply(uint8_t* result, const uint8_t* poly1, int deg1,
                      const uint8_t* poly2, int deg2);
    int polyDegree(const uint8_t* poly, int maxDeg);

    // Berlekamp-Massey algorithm
    int berlekampMassey(uint8_t syndrome[], uint8_t lambda[], int syndLen);

    // Chien search for error locations
    int chienSearch(uint8_t lambda[], int lambdaDeg, uint8_t errorLocations[]);

    // Forney algorithm for error values (for non-binary codes)
    void forneyAlgorithm(uint8_t syndrome[], uint8_t lambda[], int lambdaDeg,
                         uint8_t errorLocations[], int numErrors,
                         uint8_t errorValues[]);

   public:
    // Constructor
    BCH(int n_val, int k_val, int t_val);

    // Get parameters
    int getN() const { return n; }
    int getK() const { return k; }
    int getT() const { return t; }

    // Encode k-bit message to n-bit codeword
    void encode(uint8_t* message, uint8_t* codeword);

    // Decode n-bit received codeword
    bool decode(uint8_t* received, uint8_t* message);

    // Calculate syndrome
    void calculateSyndrome(uint8_t* received, uint8_t* syndrome);

    // Helper functions
    static void printBinary(uint8_t data[], int length);
};
