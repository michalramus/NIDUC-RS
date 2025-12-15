#include "bch.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

BCHEncoder::BCHEncoder(int m, int t, uint16_t primitivePoly)
    : m(m), t(t), primitivePoly(primitivePoly), n(0), k(0) {
    // Use default primitive polynomial if not provided
    if (primitivePoly == 0) {
        this->primitivePoly = getDefaultPrimitivePoly(m);
    }

    n = (1 << m) - 1;  // n = 2^m - 1
}

uint16_t BCHEncoder::getDefaultPrimitivePoly(int m) {
    // Default primitive polynomials for GF(2^m)
    // Represented as binary: bit i represents coefficient of x^i
    static const std::map<int, uint16_t> defaultPolys = {
        {2, 0b111},       // x^2 + x + 1
        {3, 0b1011},      // x^3 + x + 1
        {4, 0b10011},     // x^4 + x + 1
        {5, 0b100101},    // x^5 + x^2 + 1
        {6, 0b1000011},   // x^6 + x + 1
        {7, 0b10000011},  // x^7 + x + 1
        {8, 0b100011101}  // x^8 + x^4 + x^3 + x^2 + 1
    };

    auto it = defaultPolys.find(m);
    if (it != defaultPolys.end()) {
        return it->second;
    }

    std::cerr << "No default primitive polynomial for m=" << m << std::endl;
    return 0;
}

void BCHEncoder::buildGaloisField() {
    int fieldSize = 1 << m;  // 2^m
    alphaToInt.resize(fieldSize);
    intToAlpha.resize(fieldSize, -1);

    // α^0 = 1
    alphaToInt[0] = 1;
    intToAlpha[1] = 0;

    // Generate field elements using primitive polynomial
    uint16_t value = 1;
    for (int i = 1; i < fieldSize - 1; i++) {
        // Multiply by α (shift left)
        value <<= 1;

        // If overflow (bit m is set), reduce using primitive polynomial
        if (value & (1 << m)) {
            value ^= primitivePoly;
        }

        alphaToInt[i] = value;
        intToAlpha[value] = i;
    }
}

uint16_t BCHEncoder::gfAdd(uint16_t a, uint16_t b) {
    // Addition in GF(2^m) is XOR
    return a ^ b;
}

uint16_t BCHEncoder::gfMultiply(uint16_t a, uint16_t b) {
    if (a == 0 || b == 0) return 0;

    // Use logarithm tables for multiplication
    int logA = intToAlpha[a];
    int logB = intToAlpha[b];
    int logResult = (logA + logB) % (n);  // n = 2^m - 1

    return alphaToInt[logResult];
}

uint16_t BCHEncoder::gfPower(uint16_t alpha, int power) {
    if (power == 0) return 1;
    if (alpha == 0) return 0;

    power = power % n;  // α^n = α^0 = 1 in GF(2^m)
    if (power < 0) power += n;

    return alphaToInt[power];
}

std::vector<std::set<int>> BCHEncoder::generateCyclotomicCosets() {
    std::vector<std::set<int>> cosets;
    std::set<int> used;

    // We need cosets containing 1, 2, ..., 2t
    // (for BCH code with designed distance d = 2t + 1)
    for (int i = 1; i <= 2 * t; i++) {
        if (used.find(i) != used.end()) {
            continue;  // Already in a coset
        }

        std::set<int> coset;
        int element = i;

        // Generate cyclotomic coset by repeated squaring mod n
        do {
            coset.insert(element);
            used.insert(element);
            element = (element * 2) % n;
        } while (coset.find(element) == coset.end());

        cosets.push_back(coset);
    }

    return cosets;
}

std::vector<uint8_t> BCHEncoder::computeMinimalPolynomial(
    const std::set<int>& coset) {
    // Minimal polynomial is the product of (x - α^i) for all i in coset
    std::vector<uint8_t> minPoly = {1};  // Start with 1

    for (int exponent : coset) {
        // Multiply by (x - α^exponent) = (x + α^exponent) in GF(2)
        // Current polynomial: minPoly
        // New term: (x + α^exponent)

        std::vector<uint8_t> term(2);
        term[0] = alphaToInt[exponent];  // Constant term: α^exponent
        term[1] = 1;                     // x coefficient

        minPoly = polyMultiply(minPoly, term);
    }

    return minPoly;
}

int BCHEncoder::polyDegree(const std::vector<uint8_t>& poly) const {
    for (int i = poly.size() - 1; i >= 0; i--) {
        if (poly[i] != 0) return i;
    }
    return -1;
}

std::vector<uint8_t> BCHEncoder::polyMultiply(const std::vector<uint8_t>& a,
                                              const std::vector<uint8_t>& b) {
    if (a.empty() || b.empty()) return {0};

    std::vector<uint8_t> result(a.size() + b.size() - 1, 0);

    for (size_t i = 0; i < a.size(); i++) {
        for (size_t j = 0; j < b.size(); j++) {
            uint16_t prod = gfMultiply(a[i], b[j]);
            result[i + j] = gfAdd(result[i + j], prod);
        }
    }

    return result;
}

std::vector<uint8_t> BCHEncoder::polyGCD(const std::vector<uint8_t>& a,
                                         const std::vector<uint8_t>& b) {
    std::vector<uint8_t> u = a;
    std::vector<uint8_t> v = b;

    while (polyDegree(v) >= 0) {
        std::vector<uint8_t> remainder;
        polyDivide(u, v, remainder);
        u = v;
        v = remainder;
    }

    return u;
}

std::vector<uint8_t> BCHEncoder::polyLCM(const std::vector<uint8_t>& a,
                                         const std::vector<uint8_t>& b) {
    std::vector<uint8_t> gcd = polyGCD(a, b);
    std::vector<uint8_t> product = polyMultiply(a, b);
    std::vector<uint8_t> remainder;
    return polyDivide(product, gcd, remainder);
}

std::vector<uint8_t> BCHEncoder::polyDivide(
    const std::vector<uint8_t>& dividend, const std::vector<uint8_t>& divisor,
    std::vector<uint8_t>& remainder) {
    remainder = dividend;
    int degDivisor = polyDegree(divisor);

    if (degDivisor < 0) {
        std::cerr << "Division by zero polynomial" << std::endl;
        return {0};
    }

    std::vector<uint8_t> quotient;
    uint16_t leadCoeff = divisor[degDivisor];

    while (polyDegree(remainder) >= degDivisor) {
        int degDiff = polyDegree(remainder) - degDivisor;

        // Calculate quotient coefficient
        uint16_t coeff = remainder[polyDegree(remainder)];

        // In GF(2^m), division by leadCoeff
        if (leadCoeff != 1) {
            // Find multiplicative inverse
            int logLead = intToAlpha[leadCoeff];
            int logCoeff = intToAlpha[coeff];
            int logQuot = (logCoeff - logLead + n) % n;
            coeff = alphaToInt[logQuot];
        }

        // Extend quotient if needed
        if (quotient.size() < (size_t)(degDiff + 1)) {
            quotient.resize(degDiff + 1, 0);
        }
        quotient[degDiff] = coeff;

        // Subtract divisor * coeff * x^degDiff from remainder
        for (int i = 0; i <= degDivisor; i++) {
            uint16_t prod = gfMultiply(divisor[i], coeff);
            remainder[i + degDiff] = gfAdd(remainder[i + degDiff], prod);
        }
    }

    if (quotient.empty()) quotient = {0};
    return quotient;
}

void BCHEncoder::generateGeneratorPolynomial() {
    // Generate cyclotomic cosets
    auto cosets = generateCyclotomicCosets();

    std::cout << "Cyclotomic cosets for roots α^1 to α^" << (2 * t) << ":"
              << std::endl;
    for (size_t i = 0; i < cosets.size(); i++) {
        std::cout << "  Coset " << i << ": {";
        bool first = true;
        for (int elem : cosets[i]) {
            if (!first) std::cout << ", ";
            std::cout << elem;
            first = false;
        }
        std::cout << "}" << std::endl;
    }

    // Start with polynomial 1
    generatorPoly = {1};

    // Compute LCM of all minimal polynomials
    for (const auto& coset : cosets) {
        std::vector<uint8_t> minPoly = computeMinimalPolynomial(coset);

        std::cout << "  Minimal polynomial for coset: ";
        for (int i = polyDegree(minPoly); i >= 0; i--) {
            if (minPoly[i] != 0) {
                if (i < polyDegree(minPoly)) std::cout << " + ";
                if (i == 0) {
                    std::cout << (int)minPoly[i];
                } else if (i == 1) {
                    if (minPoly[i] == 1)
                        std::cout << "x";
                    else
                        std::cout << (int)minPoly[i] << "x";
                } else {
                    if (minPoly[i] == 1)
                        std::cout << "x^" << i;
                    else
                        std::cout << (int)minPoly[i] << "x^" << i;
                }
            }
        }
        std::cout << std::endl;

        generatorPoly = polyLCM(generatorPoly, minPoly);
    }

    // Calculate k (message length)
    k = n - polyDegree(generatorPoly);
}

bool BCHEncoder::initialize() {
    std::cout << "Initializing BCH(" << n << ", k, " << (2 * t + 1)
              << ") over GF(2^" << m << ")" << std::endl;
    std::cout << "Primitive polynomial: 0x" << std::hex << primitivePoly
              << std::dec << std::endl;

    // Step 1: Build Galois Field
    buildGaloisField();
    std::cout << "✓ Galois Field GF(2^" << m << ") constructed" << std::endl;

    // Step 2: Generate generator polynomial
    generateGeneratorPolynomial();
    std::cout << "✓ Generator polynomial g(x) constructed" << std::endl;
    std::cout << "  Degree: " << polyDegree(generatorPoly) << std::endl;
    std::cout << "  Message length k: " << k << std::endl;

    return true;
}

std::vector<uint8_t> BCHEncoder::encode(const std::vector<uint8_t>& message) {
    if (message.size() != (size_t)k) {
        std::cerr << "Error: Message length must be " << k << " bits, got "
                  << message.size() << std::endl;
        return {};
    }

    // Step 1: Scale source information by multiplying by x^(n-k)
    // This shifts the message to the left by (n-k) positions
    std::vector<uint8_t> scaledMessage(n, 0);
    int parityBits = n - k;

    for (int i = 0; i < k; i++) {
        scaledMessage[i + parityBits] = message[i];
    }

    // Step 2: Divide scaled message by generator polynomial
    std::vector<uint8_t> remainder;
    polyDivide(scaledMessage, generatorPoly, remainder);

    // Step 3: Create systematic codeword: message + parity
    std::vector<uint8_t> codeword(n, 0);

    // Add parity bits (remainder) to the beginning
    for (size_t i = 0; i < remainder.size() && i < (size_t)parityBits; i++) {
        codeword[i] = remainder[i];
    }

    // Add original message to the end
    for (int i = 0; i < k; i++) {
        codeword[i + parityBits] = message[i];
    }

    return codeword;
}

void BCHEncoder::printCodeInfo() const {
    std::cout << "\n=== BCH Code Information ===" << std::endl;
    std::cout << "Code parameters: BCH(" << n << ", " << k << ", "
              << (2 * t + 1) << ")" << std::endl;
    std::cout << "  n (code length): " << n << std::endl;
    std::cout << "  k (message length): " << k << std::endl;
    std::cout << "  t (error correction): " << t << " errors" << std::endl;
    std::cout << "  d_min (minimum distance): " << (2 * t + 1) << std::endl;
    std::cout << "  Parity bits: " << (n - k) << std::endl;
    std::cout << "  Code rate: " << (float)k / n << std::endl;

    std::cout << "\nGenerator polynomial g(x): ";
    for (int i = polyDegree(generatorPoly); i >= 0; i--) {
        if (generatorPoly[i] != 0) {
            if (i < polyDegree(generatorPoly)) std::cout << " + ";
            if (i == 0) {
                std::cout << (int)generatorPoly[i];
            } else if (i == 1) {
                if (generatorPoly[i] == 1)
                    std::cout << "x";
                else
                    std::cout << (int)generatorPoly[i] << "x";
            } else {
                if (generatorPoly[i] == 1)
                    std::cout << "x^" << i;
                else
                    std::cout << (int)generatorPoly[i] << "x^" << i;
            }
        }
    }
    std::cout << std::endl;

    std::cout << "\nBinary representation: ";
    for (int i = polyDegree(generatorPoly); i >= 0; i--) {
        std::cout << (int)generatorPoly[i];
    }
    std::cout << "\n=========================\n" << std::endl;
}

// ============================================================================
// BCH DECODER IMPLEMENTATION
// ============================================================================

BCHDecoder::BCHDecoder(BCHEncoder& encoder) : encoder(encoder) {}

std::vector<uint16_t> BCHDecoder::calculateSyndromes(
    const std::vector<uint8_t>& received) {
    std::vector<uint16_t> syndromes(2 * encoder.t);

    // Calculate syndromes S_i = r(α^i) for i = 1, 2, ..., 2t
    for (int i = 0; i < 2 * encoder.t; i++) {
        uint16_t syndrome = 0;

        // Evaluate received polynomial at α^(i+1)
        for (size_t j = 0; j < received.size(); j++) {
            if (received[j] != 0) {
                // Calculate α^((i+1)*j)
                int exponent = ((i + 1) * j) % encoder.n;
                uint16_t term = encoder.alphaToInt[exponent];
                syndrome = encoder.gfAdd(syndrome, term);
            }
        }

        syndromes[i] = syndrome;
    }

    return syndromes;
}

uint16_t BCHDecoder::evaluatePolynomial(const std::vector<uint16_t>& poly,
                                        uint16_t point) {
    if (poly.empty()) return 0;

    uint16_t result = 0;
    uint16_t power = 1;

    for (size_t i = 0; i < poly.size(); i++) {
        if (poly[i] != 0) {
            uint16_t term = encoder.gfMultiply(poly[i], power);
            result = encoder.gfAdd(result, term);
        }
        power = encoder.gfMultiply(power, point);
    }

    return result;
}

bool BCHDecoder::findErrorLocatorPolynomial(
    const std::vector<uint16_t>& syndromes, std::vector<uint16_t>& errorLocator,
    int& numErrors) {
    // Try Peterson's algorithm for different error counts
    // Start from t errors down to 1

    for (int v = encoder.t; v >= 1; v--) {
        // Build syndrome matrix for v errors
        // Peterson's algorithm: solve S * Λ = -S_shift
        std::vector<std::vector<uint16_t>> matrix(v, std::vector<uint16_t>(v));
        std::vector<uint16_t> rhs(v);

        // Syndrome matrix (Toeplitz structure):
        // | S_1   S_2   ...  S_v   |   | Λ_v |   | S_{v+1} |
        // | S_2   S_3   ...  S_{v+1}|   | Λ_{v-1}| = | S_{v+2} |
        // | ...   ...  ...   ...   |   | ... |   | ...     |
        // | S_v   S_{v+1} ... S_{2v-1}| | Λ_1 |   | S_{2v}  |

        for (int i = 0; i < v; i++) {
            for (int j = 0; j < v; j++) {
                matrix[i][j] = syndromes[i + j];
            }
            rhs[i] = syndromes[i + v];
        }

        // Check if matrix is singular
        uint16_t det = determinant(matrix);
        if (det == 0) {
            continue;  // Try fewer errors
        }

        // Solve for error locator coefficients
        std::vector<uint16_t> lambda(v);
        if (!solveLinearSystem(matrix, rhs, lambda)) {
            continue;
        }

        // Build error locator polynomial: Λ(x) = 1 + Λ_1*x + ... + Λ_v*x^v
        errorLocator.resize(v + 1);
        errorLocator[0] = 1;
        for (int i = 0; i < v; i++) {
            errorLocator[i + 1] = lambda[i];
        }

        numErrors = v;
        return true;
    }

    // No errors or all syndromes are zero
    if (syndromes[0] == 0) {
        errorLocator = {1};  // No errors
        numErrors = 0;
        return true;
    }

    return false;  // Unable to find error locator polynomial
}

std::vector<int> BCHDecoder::chienSearch(
    const std::vector<uint16_t>& errorLocator) {
    std::vector<int> errorPositions;

    // Chien search: evaluate Λ(α^-i) for i = 0, 1, ..., n-1
    // If Λ(α^-i) = 0, then error is at position i
    for (int i = 0; i < encoder.n; i++) {
        // Evaluate at α^-i = α^(n-i)
        int exponent = (encoder.n - i) % encoder.n;
        uint16_t alphaInv = encoder.alphaToInt[exponent];

        uint16_t result = evaluatePolynomial(errorLocator, alphaInv);

        if (result == 0) {
            errorPositions.push_back(i);
        }
    }

    return errorPositions;
}

uint16_t BCHDecoder::determinant(
    const std::vector<std::vector<uint16_t>>& matrix) {
    int n = matrix.size();
    if (n == 0) return 0;
    if (n == 1) return matrix[0][0];
    if (n == 2) {
        uint16_t a = encoder.gfMultiply(matrix[0][0], matrix[1][1]);
        uint16_t b = encoder.gfMultiply(matrix[0][1], matrix[1][0]);
        return encoder.gfAdd(a, b);
    }

    // For larger matrices, use expansion (simplified for small t)
    uint16_t det = 0;
    for (int j = 0; j < n; j++) {
        // Create submatrix
        std::vector<std::vector<uint16_t>> submatrix(
            n - 1, std::vector<uint16_t>(n - 1));
        for (int i = 1; i < n; i++) {
            int col = 0;
            for (int k = 0; k < n; k++) {
                if (k == j) continue;
                submatrix[i - 1][col++] = matrix[i][k];
            }
        }

        uint16_t cofactor =
            encoder.gfMultiply(matrix[0][j], determinant(submatrix));
        det = encoder.gfAdd(det, cofactor);  // In GF(2), no sign alternation
    }

    return det;
}

bool BCHDecoder::solveLinearSystem(
    const std::vector<std::vector<uint16_t>>& matrix,
    const std::vector<uint16_t>& rhs, std::vector<uint16_t>& solution) {
    int n = matrix.size();
    solution.resize(n);

    // Use Cramer's rule for small systems
    uint16_t det = determinant(matrix);
    if (det == 0) return false;

    // Find multiplicative inverse of determinant
    int logDet = encoder.intToAlpha[det];
    int logDetInv = (encoder.n - logDet) % encoder.n;
    uint16_t detInv = encoder.alphaToInt[logDetInv];

    for (int i = 0; i < n; i++) {
        // Replace column i with rhs
        std::vector<std::vector<uint16_t>> matrixI = matrix;
        for (int j = 0; j < n; j++) {
            matrixI[j][i] = rhs[j];
        }

        uint16_t detI = determinant(matrixI);
        solution[i] = encoder.gfMultiply(detI, detInv);
    }

    return true;
}

int BCHDecoder::decode(const std::vector<uint8_t>& received,
                       std::vector<uint8_t>& correctedMessage) {
    int errorCount;
    std::vector<uint8_t> corrected = decodeCodeword(received, errorCount);

    if (errorCount < 0) {
        return -1;  // Decoding failed
    }

    // Extract message from corrected codeword (systematic code)
    int parityBits = encoder.n - encoder.k;
    correctedMessage.resize(encoder.k);
    for (int i = 0; i < encoder.k; i++) {
        correctedMessage[i] = corrected[i + parityBits];
    }

    return errorCount;
}

std::vector<uint8_t> BCHDecoder::decodeCodeword(
    const std::vector<uint8_t>& received, int& errorCount) {
    errorCount = 0;

    if (received.size() != (size_t)encoder.n) {
        std::cerr << "Error: Received codeword length must be " << encoder.n
                  << " bits, got " << received.size() << std::endl;
        errorCount = -1;
        return {};
    }

    // Step 1: Calculate syndromes
    std::vector<uint16_t> syndromes = calculateSyndromes(received);

    // Check if all syndromes are zero (no errors)
    bool allZero = true;
    for (auto s : syndromes) {
        if (s != 0) {
            allZero = false;
            break;
        }
    }

    if (allZero) {
        std::cout << "No errors detected" << std::endl;
        return received;  // No errors
    }

    // Step 2: Find error locator polynomial
    std::vector<uint16_t> errorLocator;
    int numErrors;

    if (!findErrorLocatorPolynomial(syndromes, errorLocator, numErrors)) {
        std::cerr << "Unable to find error locator polynomial - too many errors"
                  << std::endl;
        errorCount = -1;
        return received;  // Return uncorrected
    }

    std::cout << "Detected " << numErrors << " error(s)" << std::endl;

    // Step 3: Find error locations using Chien search
    std::vector<int> errorPositions = chienSearch(errorLocator);

    if (errorPositions.size() != (size_t)numErrors) {
        std::cerr << "Error location mismatch: expected " << numErrors
                  << " errors, found " << errorPositions.size() << std::endl;
        errorCount = -1;
        return received;
    }

    std::cout << "Error positions: ";
    for (auto pos : errorPositions) {
        std::cout << pos << " ";
    }
    std::cout << std::endl;

    // Step 4: Correct errors (flip bits for binary BCH)
    std::vector<uint8_t> corrected = received;
    for (int pos : errorPositions) {
        corrected[pos] ^= 1;  // Flip bit
    }

    errorCount = numErrors;
    return corrected;
}
