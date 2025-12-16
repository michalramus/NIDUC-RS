#ifndef BCH_HPP
#define BCH_HPP

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

/**
 * BCH (Bose-Chaudhuri-Hocquenghem) Encoder for GF(2^m)
 * Implements systematic encoding using cyclotomic cosets
 */
class BCHEncoder {
   public:
    /**
     * Constructor for BCH encoder
     * @param m Extension degree of Galois Field GF(2^m)
     * @param t Error correction capability (number of errors to correct)
     * @param primitivePoly Primitive polynomial for GF(2^m) construction
     */
    BCHEncoder(int m, int t, uint16_t primitivePoly = 0);

    /**
     * Initialize the BCH encoder with specific parameters
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * Encode a message using systematic BCH encoding
     * @param message Input message bits (k bits)
     * @return Encoded codeword (n bits) = message + parity
     */
    std::vector<uint8_t> encode(const std::vector<uint8_t>& message);

    // Getters for code parameters
    int getN() const { return n; }  // Code length
    int getK() const { return k; }  // Message length
    int getT() const { return t; }  // Error correction capability
    int getM() const { return m; }  // GF extension degree

    // Get generator polynomial
    std::vector<uint8_t> getGeneratorPolynomial() const {
        return generatorPoly;
    }

    // Print code information
    void printCodeInfo() const;

   private:
    // GF(2^m) parameters
    int m;                   // Extension degree
    int t;                   // Error correction capability
    uint16_t primitivePoly;  // Primitive polynomial
    int n;                   // Code length (2^m - 1)
    int k;                   // Message length (n - degree of g(x))

    // GF arithmetic tables
    std::vector<uint16_t> alphaToInt;  // α^i to integer representation
    std::vector<int> intToAlpha;       // Integer to α^i (log table)

    // Polynomials
    std::vector<uint8_t> generatorPoly;  // Generator polynomial g(x)

    // Helper functions for GF(2^m) arithmetic
    void buildGaloisField();
    uint16_t gfMultiply(uint16_t a, uint16_t b);
    uint16_t gfAdd(uint16_t a, uint16_t b);
    // uint16_t gfPower(uint16_t alpha, int power);

    // Cyclotomic coset generation
    std::vector<std::set<int>> generateCyclotomicCosets();

    // Minimal polynomial computation
    std::vector<uint8_t> computeMinimalPolynomial(const std::set<int>& coset);

    // Polynomial operations
    std::vector<uint8_t> polyMultiply(const std::vector<uint8_t>& a,
                                      const std::vector<uint8_t>& b);
    std::vector<uint8_t> polyDivide(const std::vector<uint8_t>& dividend,
                                    const std::vector<uint8_t>& divisor,
                                    std::vector<uint8_t>& remainder);
    std::vector<uint8_t> polyLCM(const std::vector<uint8_t>& a,
                                 const std::vector<uint8_t>& b);
    std::vector<uint8_t> polyGCD(const std::vector<uint8_t>& a,
                                 const std::vector<uint8_t>& b);
    int polyDegree(const std::vector<uint8_t>& poly) const;

    // Generator polynomial construction
    void generateGeneratorPolynomial();

    // Default primitive polynomials for different m values
    static uint16_t getDefaultPrimitivePoly(int m);

    // Allow decoder to access private members
    friend class BCHDecoder;
};

/**
 * BCH Decoder - Hamming weight based decoding with cyclic shifts
 * Works with BCHEncoder to decode and correct errors
 */
class BCHDecoder {
   public:
    /**
     * Constructor - uses same parameters as encoder
     * @param encoder Reference to initialized BCH encoder
     */
    BCHDecoder(BCHEncoder& encoder);

    /**
     * Decode a received codeword and correct errors
     * @param received Received codeword (n bits, may contain errors)
     * @param correctedMessage Output: corrected message (k bits)
     * @return Number of errors detected and corrected (-1 if uncorrectable)
     */
    int decode(const std::vector<uint8_t>& received,
               std::vector<uint8_t>& correctedMessage);

    /**
     * Decode and return full corrected codeword
     * @param received Received codeword (n bits)
     * @param errorCount Output: number of errors corrected
     * @return Corrected codeword
     */
    std::vector<uint8_t> decodeCodeword(const std::vector<uint8_t>& received,
                                        int& errorCount);

   private:
    BCHEncoder& encoder;  // Reference to encoder for parameters

    // Calculate syndrome vector (binary)
    std::vector<uint8_t> calculateSyndrome(
        const std::vector<uint8_t>& received);

    // Calculate Hamming weight
    int hammingWeight(const std::vector<uint8_t>& vector);

    // Cyclic shift right
    std::vector<uint8_t> cyclicShiftRight(const std::vector<uint8_t>& vector);

    // Cyclic shift left
    std::vector<uint8_t> cyclicShiftLeft(const std::vector<uint8_t>& vector);
};

#endif  // BCH_HPP
