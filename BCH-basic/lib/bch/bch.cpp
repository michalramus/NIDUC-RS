#include "bch.hpp"

// GF(2^4) primitive polynomial: x^4 + x + 1 (0x13 in binary)
#define PRIMITIVE_POLY 0x13
#define GF16_SIZE 16
#define ALPHA 2  // Primitive element of GF(16)

// Precomputed log and antilog tables for GF(16)
static uint8_t logTable[GF16_SIZE];
static uint8_t antilogTable[GF16_SIZE];
static bool tablesInitialized = false;

// Initialize log and antilog tables
static void initTables() {
    if (tablesInitialized) return;

    uint8_t x = 1;
    for (int i = 0; i < GF16_SIZE - 1; i++) {
        antilogTable[i] = x;
        logTable[x] = i;

        // Multiply by alpha (primitive element)
        x <<= 1;
        if (x & 0x10) {
            x ^= PRIMITIVE_POLY;
        }
    }
    antilogTable[GF16_SIZE - 1] = 1;
    logTable[0] = 0;  // Undefined, but set to 0

    tablesInitialized = true;
}

// GF(2^4) operations
uint8_t GF16::add(uint8_t a, uint8_t b) {
    return a ^ b;  // XOR for addition in GF(2)
}

uint8_t GF16::multiply(uint8_t a, uint8_t b) {
    if (a == 0 || b == 0) return 0;
    initTables();
    return antilogTable[(logTable[a] + logTable[b]) % (GF16_SIZE - 1)];
}

uint8_t GF16::power(uint8_t base, uint8_t exp) {
    if (exp == 0) return 1;
    if (base == 0) return 0;
    initTables();
    return antilogTable[(logTable[base] * exp) % (GF16_SIZE - 1)];
}

uint8_t GF16::inverse(uint8_t a) {
    if (a == 0) return 0;
    initTables();
    return antilogTable[(GF16_SIZE - 1) - logTable[a]];
}

uint8_t GF16::divide(uint8_t a, uint8_t b) {
    if (b == 0) return 0;
    if (a == 0) return 0;
    return multiply(a, inverse(b));
}

uint8_t GF16::log(uint8_t a) {
    initTables();
    return logTable[a];
}

uint8_t GF16::antilog(uint8_t i) {
    initTables();
    return antilogTable[i % (GF16_SIZE - 1)];
}

// BCH Constructor
BCH::BCH(int n_val, int k_val, int t_val) : n(n_val), k(k_val), t(t_val) {
    initTables();

    // Validate parameters for GF(2^4)
    if (n > MAX_N || k > n || t > MAX_T || k <= 0 || t <= 0) {
        Serial.println("Invalid BCH parameters!");
        n = 15;
        k = 7;
        t = 2;  // Default to BCH(15,7,2)
    }

    // Check if n-k >= 2*t (necessary condition)
    if (n - k < 2 * t) {
        Serial.println("Warning: n-k must be >= 2*t for error correction!");
    }

    buildGeneratorPolynomial();
}

// Build generator polynomial from minimal polynomials
void BCH::buildGeneratorPolynomial() {
    // Initialize generator polynomial to 1
    memset(generatorPoly, 0, sizeof(generatorPoly));
    generatorPoly[0] = 1;
    generatorDegree = 0;

    // For binary BCH codes, we need minimal polynomials for α^i where i = 1, 2,
    // ..., 2t For GF(2^4), the minimal polynomial of α is: m1(x) = x^4 + x + 1
    // This is a simplified approach - build from consecutive roots

    // For each root α^i (i = 1 to 2t), multiply by (x - α^i)
    // But for binary codes, we use minimal polynomials instead
    // Simplified: use a fixed generator for common codes

    // Pre-computed generators for common BCH codes over GF(16):
    if (n == 15 && k == 11 && t == 1) {
        // BCH(15,11,1): g(x) = x^4 + x + 1
        uint8_t g[] = {1, 1, 0, 0, 1};
        memcpy(generatorPoly, g, 5);
        generatorDegree = 4;
    } else if (n == 15 && k == 7 && t == 2) {
        // BCH(15,7,2): g(x) = x^8 + x^7 + x^6 + x^4 + 1
        uint8_t g[] = {1, 0, 0, 1, 0, 1, 1, 1, 1};
        memcpy(generatorPoly, g, 9);
        generatorDegree = 8;
    } else if (n == 15 && k == 5 && t == 3) {
        // BCH(15,5,3): g(x) = x^10 + x^8 + x^5 + x^4 + x^2 + x + 1
        uint8_t g[] = {1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1};
        memcpy(generatorPoly, g, 11);
        generatorDegree = 10;
    } else {
        // Generic construction using minimal polynomials
        // Build generator by multiplying minimal polynomials
        uint8_t temp[MAX_N] = {1};
        int tempDeg = 0;

        for (int i = 1; i <= 2 * t; i++) {
            // Minimal polynomial: (x - α^i)
            // For binary BCH, use actual minimal polynomials
            uint8_t minPoly[MAX_N] = {0};
            minPoly[0] = GF16::power(ALPHA, i);
            minPoly[1] = 1;  // x + α^i
            int minDeg = 1;

            uint8_t result[MAX_N] = {0};
            polyMultiply(result, temp, tempDeg, minPoly, minDeg);
            tempDeg = polyDegree(result, tempDeg + minDeg);
            memcpy(temp, result, sizeof(temp));
        }

        memcpy(generatorPoly, temp, sizeof(generatorPoly));
        generatorDegree = tempDeg;
    }

    Serial.print("Generator polynomial degree: ");
    Serial.println(generatorDegree);
}

// Polynomial multiplication
void BCH::polyMultiply(uint8_t* result, const uint8_t* poly1, int deg1,
                       const uint8_t* poly2, int deg2) {
    memset(result, 0, MAX_N);
    for (int i = 0; i <= deg1; i++) {
        for (int j = 0; j <= deg2; j++) {
            result[i + j] ^= GF16::multiply(poly1[i], poly2[j]);
        }
    }
}

// Get polynomial degree
int BCH::polyDegree(const uint8_t* poly, int maxDeg) {
    for (int i = maxDeg; i >= 0; i--) {
        if (poly[i] != 0) return i;
    }
    return 0;
}

// BCH encoding using systematic encoding
void BCH::encode(uint8_t* message, uint8_t* codeword) {
    // Systematic encoding: [message | parity]
    // Copy message to first k bits
    for (int i = 0; i < k; i++) {
        codeword[i] = message[i];
    }

    // Compute parity bits by dividing x^(n-k) * message(x) by g(x)
    uint8_t temp[MAX_N] = {0};

    // Shift message by (n-k) positions
    for (int i = 0; i < k; i++) {
        temp[i + (n - k)] = message[i];
    }

    // Polynomial division: compute remainder
    for (int i = n - 1; i >= n - k; i--) {
        if (temp[i]) {
            for (int j = 0; j <= generatorDegree && (i - j) >= 0; j++) {
                temp[i - j] ^= generatorPoly[j];
            }
        }
    }

    // Copy parity bits to codeword
    for (int i = k; i < n; i++) {
        codeword[i] = temp[i - k];
    }
}

// Calculate syndrome
void BCH::calculateSyndrome(uint8_t* received, uint8_t* syndrome) {
    // Evaluate received polynomial at α^1, α^2, ..., α^(2t)
    for (int i = 0; i < 2 * t; i++) {
        syndrome[i] = 0;
        uint8_t alphaI = GF16::power(ALPHA, i + 1);

        // Horner's method for polynomial evaluation
        for (int j = n - 1; j >= 0; j--) {
            syndrome[i] = GF16::multiply(syndrome[i], alphaI);
            if (received[j]) {
                syndrome[i] ^= GF16::power(alphaI, j);
            }
        }
    }
}

// Berlekamp-Massey algorithm to find error locator polynomial
int BCH::berlekampMassey(uint8_t syndrome[], uint8_t lambda[], int syndLen) {
    uint8_t C[MAX_T + 1] = {0};  // Current polynomial
    uint8_t B[MAX_T + 1] = {0};  // Previous polynomial
    uint8_t T[MAX_T + 1] = {0};  // Temporary

    C[0] = 1;
    B[0] = 1;
    int L = 0;      // Degree of C(x)
    int m = 1;      // Length of feedback shift register
    uint8_t b = 1;  // Discrepancy

    for (int n = 0; n < syndLen; n++) {
        // Calculate discrepancy
        uint8_t d = syndrome[n];
        for (int i = 1; i <= L; i++) {
            d ^= GF16::multiply(C[i], syndrome[n - i]);
        }

        if (d == 0) {
            m++;
        } else {
            if (2 * L <= n) {
                // Copy C to T
                memcpy(T, C, sizeof(C));

                // C(x) = C(x) - (d/b) * x^m * B(x)
                uint8_t factor = GF16::divide(d, b);
                for (int i = 0; i <= MAX_T; i++) {
                    if (i >= m && B[i - m] != 0) {
                        C[i] ^= GF16::multiply(factor, B[i - m]);
                    }
                }

                L = n + 1 - L;
                memcpy(B, T, sizeof(B));
                b = d;
                m = 1;
            } else {
                // C(x) = C(x) - (d/b) * x^m * B(x)
                uint8_t factor = GF16::divide(d, b);
                for (int i = 0; i <= MAX_T; i++) {
                    if (i >= m && B[i - m] != 0) {
                        C[i] ^= GF16::multiply(factor, B[i - m]);
                    }
                }
                m++;
            }
        }
    }

    // Copy result
    memcpy(lambda, C, (L + 1) * sizeof(uint8_t));
    return L;
}

// Chien search to find error locations
int BCH::chienSearch(uint8_t lambda[], int lambdaDeg,
                     uint8_t errorLocations[]) {
    int numErrors = 0;

    // Test each position in the codeword
    for (int i = 0; i < n; i++) {
        // Evaluate lambda at α^(-i)
        uint8_t sum = 0;
        for (int j = 0; j <= lambdaDeg; j++) {
            if (lambda[j] != 0) {
                sum ^= GF16::multiply(
                    lambda[j], GF16::power(ALPHA, (GF16_SIZE - 1 - i) * j));
            }
        }

        if (sum == 0) {
            // Found an error at position i
            errorLocations[numErrors++] = i;
            if (numErrors >= t) break;
        }
    }

    return numErrors;
}

// Forney algorithm for error values (binary case - always 1)
void BCH::forneyAlgorithm(uint8_t syndrome[], uint8_t lambda[], int lambdaDeg,
                          uint8_t errorLocations[], int numErrors,
                          uint8_t errorValues[]) {
    // For binary BCH codes, error values are always 1 (bit flip)
    for (int i = 0; i < numErrors; i++) {
        errorValues[i] = 1;
    }
}

// Decode received codeword
bool BCH::decode(uint8_t* received, uint8_t* message) {
    uint8_t syndrome[2 * MAX_T];
    calculateSyndrome(received, syndrome);

    // Check if all syndromes are zero (no errors)
    bool hasError = false;
    for (int i = 0; i < 2 * t; i++) {
        if (syndrome[i] != 0) {
            hasError = true;
            break;
        }
    }

    if (!hasError) {
        // No errors, extract message
        for (int i = 0; i < k; i++) {
            message[i] = received[i];
        }
        return true;
    }

    // Use Berlekamp-Massey to find error locator polynomial
    uint8_t lambda[MAX_T + 1] = {0};
    int lambdaDeg = berlekampMassey(syndrome, lambda, 2 * t);

    if (lambdaDeg == 0 || lambdaDeg > t) {
        Serial.println("Decoding failed: too many errors");
        // Extract message anyway (uncorrected)
        for (int i = 0; i < k; i++) {
            message[i] = received[i];
        }
        return false;
    }

    // Use Chien search to find error locations
    uint8_t errorLocations[MAX_T];
    int numErrors = chienSearch(lambda, lambdaDeg, errorLocations);

    if (numErrors != lambdaDeg) {
        Serial.println("Decoding failed: error location mismatch");
        for (int i = 0; i < k; i++) {
            message[i] = received[i];
        }
        return false;
    }

    // Get error values (always 1 for binary)
    uint8_t errorValues[MAX_T];
    forneyAlgorithm(syndrome, lambda, lambdaDeg, errorLocations, numErrors,
                    errorValues);

    // Correct errors
    for (int i = 0; i < numErrors; i++) {
        if (errorLocations[i] < n) {
            received[errorLocations[i]] ^= errorValues[i];
        }
    }

    // Extract corrected message
    for (int i = 0; i < k; i++) {
        message[i] = received[i];
    }

    Serial.print("Corrected ");
    Serial.print(numErrors);
    Serial.println(" errors");

    return true;
}

// Helper to print binary
void BCH::printBinary(uint8_t data[], int length) {
    for (int i = 0; i < length; i++) {
        Serial.print(data[i] ? "1" : "0");
    }
}
