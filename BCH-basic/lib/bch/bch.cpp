#include "bch.hpp"

// GF(2^4) primitive polynomial: x^4 + x + 1 (0x13 in binary)
#define PRIMITIVE_POLY 0x13

// GF(2^4) operations
uint8_t GF16::add(uint8_t a, uint8_t b) {
    return a ^ b;  // XOR for addition in GF(2)
}

uint8_t GF16::multiply(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    for (int i = 0; i < 4; i++) {
        if (b & 1) {
            result ^= a;
        }
        bool highBit = a & 0x08;
        a <<= 1;
        if (highBit) {
            a ^= PRIMITIVE_POLY;
        }
        b >>= 1;
    }
    return result & 0x0F;
}

uint8_t GF16::power(uint8_t base, uint8_t exp) {
    if (exp == 0) return 1;
    uint8_t result = 1;
    for (uint8_t i = 0; i < exp; i++) {
        result = multiply(result, base);
    }
    return result;
}

uint8_t GF16::inverse(uint8_t a) {
    if (a == 0) return 0;
    // Use Fermat's little theorem: a^(-1) = a^(q-2) where q = 16
    return power(a, 14);
}

uint8_t GF16::divide(uint8_t a, uint8_t b) {
    if (b == 0) return 0;
    return multiply(a, inverse(b));
}

// BCH encoding
void BCH::encode(uint8_t message[BCH_K], uint8_t codeword[BCH_N]) {
    // Copy message to first k bits
    for (int i = 0; i < BCH_K; i++) {
        codeword[i] = message[i];
    }
    
    // Generator polynomial for BCH(15,7,2): g(x) = x^8 + x^7 + x^6 + x^4 + 1
    // This is a simplified implementation - compute parity bits
    uint8_t g[9] = {1, 0, 0, 1, 0, 1, 1, 1, 1};  // g(x) coefficients
    
    // Systematic encoding: multiply message by x^(n-k) and divide by g(x)
    uint8_t temp[BCH_N] = {0};
    
    // Shift message
    for (int i = 0; i < BCH_K; i++) {
        temp[i + (BCH_N - BCH_K)] = message[i];
    }
    
    // Polynomial division to get remainder (parity bits)
    for (int i = BCH_N - 1; i >= BCH_N - BCH_K; i--) {
        if (temp[i]) {
            for (int j = 0; j < 9; j++) {
                if (i - j >= 0) {
                    temp[i - j] ^= g[j];
                }
            }
        }
    }
    
    // Copy parity bits to codeword
    for (int i = BCH_K; i < BCH_N; i++) {
        codeword[i] = temp[i - BCH_K];
    }
}

// Calculate syndrome
void BCH::calculateSyndrome(uint8_t received[BCH_N], uint8_t syndrome[2*BCH_T]) {
    // Evaluate received polynomial at roots of generator polynomial
    // For BCH(15,7,2), evaluate at α, α^2, α^3, α^4
    uint8_t alpha = 2;  // Primitive element of GF(16)
    
    for (int i = 0; i < 2*BCH_T; i++) {
        syndrome[i] = 0;
        uint8_t alphaI = GF16::power(alpha, i + 1);
        
        for (int j = 0; j < BCH_N; j++) {
            if (received[j]) {
                syndrome[i] ^= GF16::power(alphaI, j);
            }
        }
    }
}

// Find error locations (simplified for demonstration)
int BCH::findErrorLocations(uint8_t syndrome[2*BCH_T], uint8_t errorLocations[BCH_T]) {
    // Check if all syndromes are zero (no errors)
    bool hasError = false;
    for (int i = 0; i < 2*BCH_T; i++) {
        if (syndrome[i] != 0) {
            hasError = true;
            break;
        }
    }
    
    if (!hasError) {
        return 0;  // No errors
    }
    
    // Simplified error location (for demonstration)
    // In a full implementation, use Berlekamp-Massey algorithm
    int numErrors = 0;
    
    // Single error case: S1 = α^i where i is error location
    if (syndrome[1] == 0 && syndrome[0] != 0) {
        // Find position where α^i = S1
        uint8_t alpha = 2;
        for (int i = 0; i < BCH_N; i++) {
            if (GF16::power(alpha, i) == syndrome[0]) {
                errorLocations[0] = i;
                numErrors = 1;
                break;
            }
        }
    }
    
    return numErrors;
}

// Correct errors
void BCH::correctErrors(uint8_t received[BCH_N], uint8_t errorLocations[BCH_T], int numErrors) {
    for (int i = 0; i < numErrors; i++) {
        if (errorLocations[i] < BCH_N) {
            received[errorLocations[i]] ^= 1;  // Flip bit
        }
    }
}

// Decode
bool BCH::decode(uint8_t received[BCH_N], uint8_t message[BCH_K]) {
    uint8_t syndrome[2*BCH_T];
    calculateSyndrome(received, syndrome);
    
    uint8_t errorLocations[BCH_T];
    int numErrors = findErrorLocations(syndrome, errorLocations);
    
    if (numErrors > 0 && numErrors <= BCH_T) {
        correctErrors(received, errorLocations, numErrors);
    }
    
    // Extract message bits
    for (int i = 0; i < BCH_K; i++) {
        message[i] = received[i];
    }
    
    return (numErrors <= BCH_T);
}

// Helper to print binary
void BCH::printBinary(uint8_t data[], int length) {
    for (int i = 0; i < length; i++) {
        Serial.print(data[i] ? "1" : "0");
    }
}

// Compute simple parity (for demonstration)
uint8_t BCH::computeParity(uint8_t message[BCH_K]) {
    uint8_t parity = 0;
    for (int i = 0; i < BCH_K; i++) {
        parity ^= message[i];
    }
    return parity;
}
