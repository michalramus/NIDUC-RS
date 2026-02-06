# Test-polynomial - Lagrange Polynomial Interpolation Tester

## Overview

This is a standalone C++ desktop application for testing and validating Lagrange polynomial interpolation algorithms. It's a utility tool used to verify the mathematical correctness of polynomial interpolation before implementing it on embedded systems (ESP8266) for Reed-Solomon decoding.

## Purpose

Lagrange polynomial interpolation is a core component of Reed-Solomon decoding. This tool allows you to:

- ✅ Test polynomial interpolation with various data points
- ✅ Verify mathematical correctness of interpolation algorithms
- ✅ Debug polynomial reconstruction issues
- ✅ Experiment with different point sets before embedded implementation
- ✅ Validate that interpolation correctly reconstructs the original polynomial

## Features

- Interactive command-line interface
- Support for arbitrary number of data points
- Polynomial multiplication and addition operations
- Lagrange basis polynomial computation
- Coefficient extraction and display
- Floating-point arithmetic for easy testing

## Project Structure

```
Test-polynomial/
├── CMakeLists.txt              # CMake build configuration
├── main.cpp                    # Main program with interpolation logic
├── README.md                   # This file
└── cmake-build-debug/          # Build output directory (generated)
    └── Test_polynomial         # Compiled executable
```

## Requirements

### Software
- **CMake** 4.0 or higher
- **C++ Compiler** with C++20 support:
  - GCC 10+ (Linux)
  - Clang 13+ (macOS)
  - MSVC 19.29+ (Windows)
  - Apple Clang 13+ (macOS)
- **Make** or **Ninja** build system

### Operating System
- Linux (Ubuntu, Debian, Fedora, etc.)
- macOS (11.0+)
- Windows (10/11 with appropriate toolchain)

## Installation

### Install CMake

#### macOS:
```bash
brew install cmake
```

#### Linux (Ubuntu/Debian):
```bash
sudo apt-get update
sudo apt-get install cmake build-essential
```

#### Linux (Fedora):
```bash
sudo dnf install cmake gcc-c++
```

#### Windows:
Download and install from [cmake.org](https://cmake.org/download/)

### Install C++ Compiler

#### macOS:
```bash
# Xcode Command Line Tools (includes Clang)
xcode-select --install
```

#### Linux:
```bash
# Ubuntu/Debian
sudo apt-get install g++

# Fedora
sudo dnf install gcc-c++
```

#### Windows:
- Install [Visual Studio 2019/2022](https://visualstudio.microsoft.com/) (with C++ workload)
- Or install [MinGW-w64](https://www.mingw-w64.org/)

## How to Build and Run

### Method 1: Using CMake (Recommended)

```bash
# Navigate to project directory
cd Test-polynomial

# Create build directory
mkdir -p build
cd build

# Generate build files
cmake ..

# Build the project
cmake --build .

# Run the executable
./Test_polynomial
```

### Method 2: Using the existing build directory

If `cmake-build-debug` already exists:

```bash
# Navigate to project directory
cd Test-polynomial

# Navigate to build directory
cd cmake-build-debug

# Build
cmake --build .

# Run
./Test_polynomial
```

### Method 3: Quick rebuild

```bash
# From Test-polynomial directory
cd cmake-build-debug
make
./Test_polynomial
```

### Method 4: Clean build

```bash
# Remove old build
rm -rf cmake-build-debug

# Create new build
mkdir cmake-build-debug
cd cmake-build-debug

# Configure and build
cmake ..
make

# Run
./Test_polynomial
```

## Usage

### Interactive Mode

When you run the program, it prompts for input:

```
$ ./Test_polynomial
Enter number of data points: 3
Enter x values:
1
2
3
Enter y values:
2
3
5
```

### Example 1: Linear Function

Test with y = 2x:

```
Enter number of data points: 3
Enter x values:
0
1
2
Enter y values:
0
2
4

Polynomial coefficients (from constant to highest degree):
0.000000 2.000000 0.000000
```

Result: P(x) = 0 + 2x + 0x² ✓

### Example 2: Quadratic Function

Test with y = x² + x + 1:

```
Enter number of data points: 3
Enter x values:
0
1
2
Enter y values:
1
3
7

Polynomial coefficients (from constant to highest degree):
1.000000 1.000000 1.000000
```

Result: P(x) = 1 + x + x² ✓

### Example 3: Reed-Solomon Scenario

Test with 4 points from a polynomial:

```
Enter number of data points: 4
Enter x values:
0
1
2
3
Enter y values:
5
8
13
20

Polynomial coefficients (from constant to highest degree):
5.000000 2.000000 1.000000 0.000000
```

Result: P(x) = 5 + 2x + x² (exactly reconstructed!)

## Algorithm Explanation

### Lagrange Interpolation Formula

For n data points (x₀, y₀), (x₁, y₁), ..., (xₙ₋₁, yₙ₋₁):

$$P(x) = \sum_{i=0}^{n-1} y_i \cdot L_i(x)$$

Where the Lagrange basis polynomial is:

$$L_i(x) = \prod_{j=0, j \neq i}^{n-1} \frac{x - x_j}{x_i - x_j}$$

### Implementation Details

The program implements three key operations:

1. **Polynomial Multiplication:**
   ```cpp
   vector<double> multiplyPoly(const vector<double>& poly, double a)
   ```
   Multiplies polynomial by (x - a)

2. **Polynomial Addition:**
   ```cpp
   vector<double> addPoly(const vector<double>& a, const vector<double>& b)
   ```
   Adds two polynomials term by term

3. **Scalar Multiplication:**
   ```cpp
   vector<double> scalePoly(const vector<double>& poly, double scalar)
   ```
   Multiplies polynomial by a constant

## Code Structure

```cpp
main() {
    1. Read n data points
    2. For each point i:
        a. Compute Lagrange basis Li(x)
        b. Scale by yi
        c. Add to result polynomial
    3. Display final coefficients
}
```

## Testing Strategies

### Verification Steps

1. **Test with known polynomials:**
   - Linear: y = mx + b
   - Quadratic: y = ax² + bx + c
   - Cubic: y = ax³ + bx² + cx + d

2. **Check coefficient accuracy:**
   - Compare computed coefficients with expected values
   - Accept small floating-point errors (< 0.001)

3. **Test edge cases:**
   - Minimum points (2 for linear)
   - Collinear points
   - Large coefficient values

### Common Test Cases

```cpp
// Test Case 1: Constant function
x: [0, 1, 2]
y: [5, 5, 5]
Expected: P(x) = 5

// Test Case 2: Linear
x: [0, 1, 2]
y: [0, 1, 2]
Expected: P(x) = x

// Test Case 3: Quadratic
x: [0, 1, 2]
y: [0, 1, 4]
Expected: P(x) = x²
```

## Troubleshooting

### Build Issues

**Problem:** CMake version error
```bash
# Check CMake version
cmake --version

# Update CMake (macOS)
brew upgrade cmake

# Update CMake (Linux)
sudo apt-get install --upgrade cmake
```

**Problem:** Compiler not found
```bash
# Check compiler (macOS/Linux)
g++ --version
clang++ --version

# Install compiler (Ubuntu)
sudo apt-get install g++
```

**Problem:** Build fails with C++20 errors
```
Solution: Update compiler to version supporting C++20
- GCC 10+
- Clang 13+
- MSVC 19.29+
```

### Runtime Issues

**Problem:** Incorrect results
```
Check:
1. Input values are correct
2. Number of points matches x and y arrays
3. No duplicate x values (undefined interpolation)
4. Floating-point precision limits
```

**Problem:** Segmentation fault
```
Check:
1. Number of points > 0
2. Arrays are properly sized
3. No division by zero (duplicate x values)
```

## Limitations

- **Floating-point arithmetic:** Subject to rounding errors
- **No error checking:** Assumes valid input (no duplicates, valid numbers)
- **Desktop only:** Not optimized for embedded systems
- **No GF arithmetic:** Uses real numbers, not finite fields

## Integration with Reed-Solomon Projects

This tool validates algorithms used in:

- [RS-basic](../RS-basic/) - Basic RS implementation
- [RS-gf31](../RS-gf31/) - Enhanced RS with GF(31)

**Note:** The Reed-Solomon projects use Galois Field arithmetic (modular), while this tester uses floating-point. The interpolation algorithm is the same, but arithmetic operations differ.

## Advanced Usage

### Scripted Testing

Create a test input file `test_input.txt`:
```
3
0 1 2
1 3 5
```

Run with input redirection:
```bash
./Test_polynomial < test_input.txt
```

### Batch Testing

Create a shell script `run_tests.sh`:
```bash
#!/bin/bash
echo "Test 1: Linear"
echo -e "3\n0 1 2\n0 1 2" | ./Test_polynomial

echo "Test 2: Quadratic"
echo -e "3\n0 1 2\n0 1 4" | ./Test_polynomial
```

Run:
```bash
chmod +x run_tests.sh
./run_tests.sh
```

## Future Improvements

- [ ] Add GF(p) arithmetic support
- [ ] Error handling for invalid input
- [ ] Coefficient comparison with tolerance
- [ ] File input/output
- [ ] GUI interface
- [ ] Plotting polynomial graphs
- [ ] Automated test suite

## References

- Lagrange Interpolation - Numerical Methods
- Polynomial Algebra
- Reed-Solomon Decoding Theory
- CMake Documentation: https://cmake.org/documentation/

## Related Tools

- **Wolfram Alpha** - Verify polynomial interpolation online
- **MATLAB/Octave** - `polyfit` and `polyval` functions
- **Python NumPy** - `numpy.polyfit` for verification
- **SageMath** - Computer algebra system with polynomial tools

## License

Academic project - Wrocław University of Science and Technology (PWr)

---
*Part of the NIDUC course project series*
*Last Updated: February 2026*
