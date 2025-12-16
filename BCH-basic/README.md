# BCH Encoder & Decoder Implementation

## Overview
This project implements a complete BCH (Bose-Chaudhuri-Hocquenghem) codec for error correction using GF(2^4), including both systematic encoding and syndrome-based decoding.

## BCH Encoding Process

The encoder follows these steps to create systematic codewords:

### 1. **Galois Field GF(2^4) Construction**
   - Creates a finite field with 16 elements (2^4)
   - Uses a primitive polynomial for field arithmetic
   - Default for GF(2^4): `x^4 + x + 1` (binary: 10011)
   - Builds logarithm tables for efficient multiplication

### 2. **Controllable Parameters**
   - **m**: Extension degree of Galois Field (e.g., m=4 for GF(2^4))
   - **t**: Error correction capability (number of errors to correct)
   - **n**: Code length = 2^m - 1 (e.g., 15 for m=4)
   - **k**: Message length = n - deg(g(x))
   - **d_min**: Minimum distance ≥ 2t + 1

### 3. **Cyclotomic Coset Generation**
   - For BCH code with designed distance d = 2t + 1
   - Generate cosets containing powers {1, 2, ..., 2t}
   - Each coset C_i = {i, 2i mod n, 4i mod n, 8i mod n, ...}
   - Example for m=4, t=2: C_1 = {1, 2, 4, 8}, C_3 = {3, 6, 12, 9}, etc.

### 4. **Generator Polynomial Construction**
   - For each cyclotomic coset, compute minimal polynomial
   - Minimal polynomial m_i(x) = ∏(x - α^j) for j ∈ C_i
   - Generator polynomial g(x) = LCM of all minimal polynomials
   - g(x) has roots at α^1, α^2, ..., α^(2t)

### 5. **Message Scaling**
   - Multiply message polynomial m(x) by x^(n-k)
   - This shifts the message to the higher-order positions
   - Leaves room for parity bits in lower positions

### 6. **Systematic Encoding**
   - Divide scaled message x^(n-k) · m(x) by g(x)
   - Obtain remainder r(x)
   - Codeword: c(x) = r(x) + x^(n-k) · m(x)
   - Result: [parity bits | message bits]

## BCH Decoding Process (Hamming Weight Method)

The decoder implements simplified syndrome-based decoding with cyclic shifts:

### 1. **Syndrome Calculation**
   - Calculate syndrome s as remainder: s = r(x) mod g(x)
   - Syndrome is a binary vector of length (n-k)

### 2. **Hamming Weight Check**
   - Calculate Hamming weight w(s) = number of 1's in syndrome
   - Compare with error correction capability t

### 3. **Case 1: w(s) ≤ t (Errors in Parity Bits)**
   - Errors are located in control/parity portion
   - Correct by adding syndrome: c_D = c_Y ⊕ s
   - If cyclic shifts were done, shift back to restore original form

### 4. **Case 2: w(s) > t (Errors in Information Bits)**
   - Errors span into information portion
   - Cyclically shift received vector right by 1 position
   - Recalculate syndrome and check weight again
   - Repeat until w(s) ≤ t or all n positions checked
   - Once w(s) ≤ t: correct errors, then shift back left by same amount

### 5. **Uncorrectable Errors**
   - If after n cyclic shifts no correction possible
   - Return error indication (-1)

This method avoids matrix operations and uses simple binary operations.

## Mathematical Validation (Decoding)

The decoding process is **mathematically sound**:

1. **Syndrome Properties**: s = r(x) mod g(x)
   - If r(x) is valid codeword, s = 0
   - If errors present, s encodes error pattern

2. **Hamming Weight Method**: w(s) ≤ t indicates correctable pattern
   - For cyclic codes, errors can be rotated to parity portion
   - Cyclic shifts preserve code structure

3. **Correction Guarantee**: For e ≤ t errors, method succeeds
   - After at most n shifts, errors align with parity bits
   - XOR with syndrome corrects the errors

## Code Structure

```
lib/bch/
├── bch.hpp          - BCH encoder & decoder class definitions
└── bch.cpp          - Complete implementation

src/
├── sender.cpp       - Demo program for encoding
└── receiver.cpp     - Demo program for decoding with test cases
```

## Usage Example

### Encoding
```cpp
#include "bch.hpp"

// Create BCH encoder: GF(2^4), correct 2 errors
BCHEncoder encoder(4, 2);
encoder.initialize();

// Encode a message
std::vector<uint8_t> message = {1, 0, 1, 0, 1, 0, 1};  // k bits
std::vector<uint8_t> codeword = encoder.encode(message);
// codeword contains: [parity bits | original message]
```

### Decoding
```cpp
// Create decoder (shares encoder parameters)
BCHDecoder decoder(encoder);

// Received codeword (possibly with errors)
std::vector<uint8_t> received = codeword;
received[5] ^= 1;  // Introduce error

// Decode and correct
std::vector<uint8_t> decoded;
int errorsFixed = decoder.decode(received, decoded);

if (errorsFixed >= 0) {
    // Successfully corrected errorsFixed errors
    // decoded contains the original message
}od g(x))
   - Since r(x) is the remainder of x^(n-k)·m(x) divided by g(x)
   - We have: x^(n-k)·m(x) = q(x)·g(x) + r(x)
   - Therefore: c(x) = r(x) + x^(n-k)·m(x) = r(x) + q(x)·g(x) + r(x) = q(x)·g(x)

2. **Error Detection**: g(x) has roots at consecutive powers of α
   - If up to t errors occur, syndrome will be non-zero
   - Allows detection and correction of up to t errors

3. **Systematic Property**: Original message is preserved
   - First (n-k) bits: parity/check bits
   - Last k bits: original message (unchanged)

## Code Structure

```
lib/bch/
├── bch.hpp          - BCH encoder class definition
└── bch.cpp          - Implementation of encoder

src/
├── sender.cpp       - Demo program for encoding
└── receiver.cpp     - Placeholder for future decoder
```

## Usage Example

```cpp
#include "bch.hpp"

// Create BCH encoder: GF(2^4), correct 2 errors
BCHEncoder encoder(4, 2);

// Initialize (builds GF and generator polynomial)
### Encoder
 **Flexible Parameters**: Control m, t, and primitive polynomial  
 **Cyclotomic Cosets**: Automatic generation for BCH construction  
 **Minimal Polynomials**: Computed from cyclotomic cosets  
 **Generator Polynomial**: LCM of minimal polynomials  
 **Systematic Encoding**: Message preserved in codeword  
 **GF(2^m) Arithmetic**: Efficient using logarithm tables  

### Decoder
 **Syndrome Calculation**: Evaluates at generator polynomial roots  
 **Peterson's Algorithm**: Simplified error locator computation  
 **Chien Search**: Efficient error location finding  
 **Error Correction**: Automatic bit-flip correction for binary codes  
 **Graceful Failure**: Detects uncorrectable error patterns  
 **Test Suite**: Multiple test cases demonstrating capabilitie
encoder.printCodeInfo();  // Shows BCH(15, k, 5)

// Encode a message
std::vector<uint8_t> message = {1, 0, 1, 0, 1, 0, 1};  // k bits
std::vector<uint8_t> codeword = encoder.encode(message);

// codeword contains: [parity bits | original message]
```

## Key Features

 **Flexible Parameters**: Control m, t, and primitive polynomial  
 **Cyclotomic Cosets**: Automatic generation for BCH construction  
 **Minimal Polynomials**: Computed from cyclotomic cosets  
 **Generator Polynomial**: LCM of minimal polynomials  
 **Systematic Encoding**: Message preserved in codeword  
 **GF(2^m) Arithmetic**: Efficient using logarithm tables  

## Primitive Polynomials

Default primitive polynomials for different extension degrees:

| m | Primitive Polynomial | Binary | Hex |
|---|---------------------|--------|-----|
| 2 | x² + x + 1 | 111 | 0x7 |
| 3 | x³ + x + 1 | 1011 | 0xB |
| 4 | x⁴ + x + 1 | 10011 | 0x13 |
| 5 | x⁵ + x² + 1 | 100101 | 0x25 |
| 6 | x⁶ + x + 1 | 1000011 | 0x43 |
| 7 | x⁷ + x + 1 | 10000011 | 0x83 |
| 8 | x⁸ + x⁴ + x³ + x² + 1 | 100011101 | 0x11D |

## Building and Running

### Build for Sender (ESP8266)
```bash
pio run -e esp8266_sender
pio run -e esp8266_sender -t upload
pio run -e esp8266_sender -t monitor
```

### Build for Receiver (ESP8266)
```bash
pio run -e esp8266_receiver
pio run -e esp8266_receiver -t upload
pio run -e esp8266_receiver -t monitor
```

## Example Output

### Encoder (Sender)
```
=== BCH Encoder Demo (Sender) ===

Initializing BCH(15, k, 5) over GF(2^4)
Primitive polynomial: 0x13
 Galois Field GF(2^4) constructed
Cyclotomic cosets for roots α^1 to α^4:
  Coset 0: {1, 2, 4, 8}
  Coset 1: {3, 6, 12, 9}
  Minimal polynomial for coset: x^4 + x + 1
  Minimal polynomial for coset: x^4 + x^3 + x^2 + x + 1
 Generator polynomial g(x) constructed
  Degree: 8
  Message length k: 7

=== BCH Code Information ===
Code parameters: BCH(15, 7, 5)
  n (code length): 15
  k (message length): 7
  t (error correction): 2 errors
  d_min (minimum distance): 5
  Parity bits: 8
  Code rate: 0.466667

Original message (7 bits): 1001001
Encoded codeword (15 bits): 101100111001001
  Parity bits (8): 10110011
  Message bits (7): 1001001
```

### Decoder (Receiver)
```
=== BCH Decoder Demo (Receiver) ===

--- Test 1: No Errors ---
No errors detected
Errors corrected: 0
Decoded message: 1001001
Decoding result:  CORRECT

--- Test 2: Single Error at Position 5 ---
Received (corrupted): 101101111001001
Detected 1 error(s)
Error positions: 5
Errors corrected: 1
Decoded message: 1001001
Decoding result:  CORRECT

--- Test 3: Double Error at Positions 3 and 10 ---
Received (corrupted): 101000110001001
Detected 2 error(s)
Error positions: 3 10
Errors corrected: 2
Decoded message: 1001001
Decoding result:  CORRECT

--- Test 4: Triple Error (Beyond Correction Capability) ---
Received (corrupted): 100100101001101
Unable to find error locator polynomial - too many errors
Decoding failed: too many errors

=== All Tests Complete ===
```

## Theory References

- **BCH Codes**: Named after Bose, Chaudhuri, and Hocquenghem
- **Galois Fields**: Finite fields GF(2^m) for binary BCH codes
- **Cyclotomic Cosets**: Used to construct minimal polynomials
- **Generator Polynomial**: LCM of minimal polynomials
- **Systematic Encoding**: Preserves original message in codeword
- **Syndrome Decoding**: Uses syndromes to identify error patterns
- **Peterson's Algorithm**: Simplified method for finding error locator polynomial
- **Chien Search**: Systematic search for roots of error locator polynomial

## Decoder Algorithm Details

### Peterson's Algorithm (Simplified)
For up to t errors, Peterson's algorithm solves:
```
[S₁   S₂   ... Sᵥ  ] [Λᵥ  ]   [Sᵥ₊₁]
[S₂   S₃   ... Sᵥ₊₁] [Λᵥ₋₁] = [Sᵥ₊₂]
[...  ...  ... ... ] [... ]   [... ]
[Sᵥ   Sᵥ₊₁ ... S₂ᵥ₋₁] [Λ₁  ]   [S₂ᵥ ]
```

Where:
- S_i are syndromes
- Λ_i are error locator coefficients
- v is the number of errors (try from t down to 1)

### Advantages of This Implementation
-  **Simple and understandable** - Peterson's algorithm is easier to grasp than Berlekamp-Massey
-  **Works well for small t** - Efficient for t ≤ 3
-  **Direct matrix solution** - Uses Cramer's rule in GF(2^m)
-  **Educational value** - Shows clear relationship between syndromes and errors

### Limitations
- Matrix inversion complexity increases with t (O(t³))
- For larger t values, Berlekamp-Massey algorithm is more efficient
- Currently implements binary BCH only (error values always 1)

## Future Enhancements

- [x] ~~BCH Decoder implementation~~ **COMPLETED**
- [x] ~~Error locator polynomial computation~~ **COMPLETED**  
- [x] ~~Chien search for error locations~~ **COMPLETED**
- [ ] Berlekamp-Massey algorithm for larger t
- [ ] Error evaluator polynomial (Forney's algorithm)
- [ ] Support for non-binary BCH codes
- [ ] Shortened BCH codes
- [ ] Performance optimizations
- [ ] Communication demo between sender and receiver

## License

MIT License - Feel free to use and modify
