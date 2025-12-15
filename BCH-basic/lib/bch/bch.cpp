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
// BCH DECODER IMPLEMENTATION - Hamming Weight Method
// ============================================================================

BCHDecoder::BCHDecoder(BCHEncoder& encoder) : encoder(encoder) {}

std::vector<uint8_t> BCHDecoder::calculateSyndrome(
    const std::vector<uint8_t>& received) {
    // Syndrom = reszta z dzielenia odebranego wektora przez wielomian
    // generujący
    std::vector<uint8_t> remainder;
    encoder.polyDivide(received, encoder.generatorPoly, remainder);

    // Wyrównaj długość syndromu do (n-k) bitów
    int parityBits = encoder.n - encoder.k;
    if (remainder.size() < (size_t)parityBits) {
        remainder.resize(parityBits, 0);
    }

    return remainder;
}

int BCHDecoder::hammingWeight(const std::vector<uint8_t>& vector) {
    int weight = 0;
    for (auto bit : vector) {
        if (bit != 0) weight++;
    }
    return weight;
}

std::vector<uint8_t> BCHDecoder::cyclicShiftRight(
    const std::vector<uint8_t>& vector) {
    if (vector.empty()) return vector;

    std::vector<uint8_t> shifted(vector.size());
    // Przesuń w prawo: ostatni element idzie na początek
    shifted[0] = vector[vector.size() - 1];
    for (size_t i = 1; i < vector.size(); i++) {
        shifted[i] = vector[i - 1];
    }
    return shifted;
}

std::vector<uint8_t> BCHDecoder::cyclicShiftLeft(
    const std::vector<uint8_t>& vector) {
    if (vector.empty()) return vector;

    std::vector<uint8_t> shifted(vector.size());
    // Przesuń w lewo: pierwszy element idzie na koniec
    for (size_t i = 0; i < vector.size() - 1; i++) {
        shifted[i] = vector[i + 1];
    }
    shifted[vector.size() - 1] = vector[0];
    return shifted;
}

int BCHDecoder::decode(const std::vector<uint8_t>& received,
                       std::vector<uint8_t>& correctedMessage) {
    int errorCount;
    std::vector<uint8_t> corrected = decodeCodeword(received, errorCount);

    if (errorCount < 0) {
        return -1;  // Dekodowanie nieudane
    }

    // Wyodrębnij wiadomość ze skorygowanego słowa kodowego (kod systematyczny)
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
        std::cerr << "Błąd: Długość odebranego słowa musi być " << encoder.n
                  << " bitów, otrzymano " << received.size() << std::endl;
        errorCount = -1;
        return {};
    }

    std::vector<uint8_t> currentVector = received;
    int shifts = 0;

    // Główna pętla dekodowania
    while (shifts <= encoder.n) {
        // Krok 1: Oblicz syndrom
        std::vector<uint8_t> syndrome = calculateSyndrome(currentVector);

        // Krok 2: Oblicz wagę Hamminga syndromu
        int weight = hammingWeight(syndrome);

        std::cout << "Przesunięcie " << shifts
                  << ": waga syndromu = " << weight;

        // Sprawdź czy wszystkie bity syndromu są zerowe (brak błędów)
        bool syndromeZero = (weight == 0);

        if (syndromeZero && shifts == 0) {
            std::cout << " - brak błędów" << std::endl;
            return currentVector;  // Brak błędów
        }

        // Przypadek 1: w(s) ≤ t - błędy w części kontrolnej
        if (weight <= encoder.t) {
            std::cout << " ≤ t=" << encoder.t
                      << " - korekcja błędów w części kontrolnej" << std::endl;

            // Korekcja: c_D = c_Y + s
            std::vector<uint8_t> corrected = currentVector;

            // Dodaj syndrom do pierwszych (n-k) bitów (część kontrolna)
            int parityBits = encoder.n - encoder.k;
            for (int i = 0; i < parityBits && i < (int)syndrome.size(); i++) {
                corrected[i] = corrected[i] ^ syndrome[i];  // XOR w GF(2)
            }

            // Jeśli były przesunięcia, cofnij je (przesuń w lewo)
            for (int i = 0; i < shifts; i++) {
                corrected = cyclicShiftLeft(corrected);
            }

            errorCount = weight;

            std::cout << "Skorygowano " << errorCount << " błąd(ów)"
                      << std::endl;
            return corrected;
        }

        // Przypadek 2: w(s) > t - błędy w części informacyjnej
        // Przesuń cyklicznie w prawo i spróbuj ponownie
        std::cout << " > t=" << encoder.t << " - przesuwam cyklicznie w prawo"
                  << std::endl;

        currentVector = cyclicShiftRight(currentVector);
        shifts++;
    }

    // Po n przesunięciach nie udało się skorygować - błędy niekorygowalne
    std::cerr << "BŁĄD: Nie udało się skorygować po " << shifts
              << " przesunięciach - błędy niekorygowalne!" << std::endl;
    errorCount = -1;
    return received;  // Zwróć niezmieniony wektor
}
