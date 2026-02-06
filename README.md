# NIDUC - Error Correction Codes Project

This repository contains implementations of various error correction coding schemes for digital communication systems, developed as part of the NIDUC (Niezawodność i Diagnostyka Układów Cyfrowych / Reliability and Diagnostics of Digital Systems) course at Wrocław University of Science and Technology.

## 📚 Project Overview

The project explores different error correction techniques through practical implementations on ESP8266 microcontrollers and PC simulations. It includes:

- **BCH Codes** - Bose-Chaudhuri-Hocquenghem codes for binary error correction
- **Reed-Solomon Codes** - Non-binary BCH codes for burst error correction  
- **Galois Field Arithmetic** - Mathematical foundations for error correction
- **Polynomial Interpolation** - Lagrange interpolation testing tools

## 🗂️ Project Structure

```
NIDUC/
├── BCH-basic/          # BCH encoder/decoder for GF(2^4)
├── RS-basic/           # Reed-Solomon encoder/decoder
├── RS-gf31/            # Enhanced RS implementation with GF(31)
└── Test-polynomial/    # Lagrange polynomial interpolation tester
```

## 📦 Subprojects

### [BCH-basic](BCH-basic/)
Complete BCH (Bose-Chaudhuri-Hocquenghem) codec implementation for ESP8266.
- Error correction using GF(2^4)
- Systematic encoding and syndrome-based decoding
- Supports configurable error correction capability
- ESP8266 sender/receiver wireless communication

**Platform:** ESP8266 (Arduino Framework via PlatformIO)

### [RS-basic](RS-basic/)
Basic Reed-Solomon encoder and decoder implementation.
- Non-binary BCH codes
- Burst error correction
- ESP8266 wireless testing

**Platform:** ESP8266 (Arduino Framework via PlatformIO)

### [RS-gf31](RS-gf31/)
Advanced Reed-Solomon implementation with comprehensive testing capabilities.
- Operations in GF(31) - prime field arithmetic
- 6 different transmission test modes
- X and Y coordinate error injection
- Enhanced error detection and statistics tracking
- Duplicate detection and validation
- Success rate monitoring

**Platform:** ESP8266 (Arduino Framework via PlatformIO)

### [Test-polynomial](Test-polynomial/)
Standalone Lagrange polynomial interpolation testing tool.
- Validates polynomial interpolation algorithms
- C++ implementation for quick testing
- Helps verify Reed-Solomon decoding logic

**Platform:** Desktop (CMake C++ project)

## 🚀 Getting Started

### Prerequisites

#### For ESP8266 Projects (BCH-basic, RS-basic, RS-gf31):
- [PlatformIO IDE](https://platformio.org/install/ide) (VS Code extension or standalone)
- 2x ESP8266 boards (NodeMCU v2 or similar)
- USB cables for programming
- Wiring for serial communication between boards

#### For Test-polynomial:
- CMake (version 4.0+)
- C++ compiler with C++20 support
- Make or Ninja build system

### Quick Start

Each subproject contains its own README with detailed build and run instructions. Navigate to the specific project folder for more information:

- [BCH-basic README](BCH-basic/README.md)
- [RS-basic README](RS-basic/README.md)
- [RS-gf31 Documentation](RS-gf31/)
- [Test-polynomial README](Test-polynomial/README.md)

## 🔧 Hardware Setup (ESP8266 Projects)

### Typical Configuration:
```
ESP8266 #1 (Sender)          ESP8266 #2 (Receiver)
├── COM Port: COM8           ├── COM Port: COM5
├── TX Pin: 12      ────────────> RX Pin: 13
└── GND             ────────────> GND
```

**Note:** COM port numbers may vary depending on your system. Check PlatformIO device monitor or system device manager.

## 📖 Theory and Concepts

### Error Correction Codes
This project implements cyclic codes that detect and correct errors in transmitted data:

- **BCH Codes:** Binary codes operating in GF(2^m), excellent for random error correction
- **Reed-Solomon Codes:** Non-binary codes ideal for burst error correction, widely used in QR codes, CDs, DVDs, and communication systems

### Galois Field Arithmetic
All implementations use finite field mathematics (Galois Fields):
- **GF(2^m):** Binary extension fields (BCH-basic)
- **GF(31):** Prime field with 31 elements (RS-gf31)

### Key Parameters
- **n:** Codeword length
- **k:** Message length
- **t:** Error correction capability (number of errors correctable)
- **d:** Minimum distance (d ≥ 2t + 1)

## 🧪 Testing

Each subproject includes test modes and examples:

- **BCH-basic:** Tester mode with various error patterns
- **RS-basic:** Sender/receiver test modes
- **RS-gf31:** 6 comprehensive test modes (clean, Y-errors, X-errors, combined)
- **Test-polynomial:** Interactive polynomial interpolation testing

## 📝 Documentation

Comprehensive documentation is available in each subproject:

- **RS-gf31** includes extensive documentation:
  - [TESTING_GUIDE.md](RS-gf31/TESTING_GUIDE.md) - Comprehensive testing procedures
  - [QUICK_REFERENCE.md](RS-gf31/QUICK_REFERENCE.md) - Quick reference card
  - [PROJECT_STRUCTURE.md](RS-gf31/PROJECT_STRUCTURE.md) - Detailed project structure
  - [ENHANCEMENT_SUMMARY.md](RS-gf31/ENHANCEMENT_SUMMARY.md) - Changes and improvements

- **BCH-basic** includes detailed README with encoding/decoding algorithms

## 🛠️ Development Tools

- **PlatformIO:** Cross-platform IDE for embedded development
- **Arduino Framework:** Easy-to-use framework for ESP8266
- **CMake:** Build system for C++ projects
- **Serial Monitor:** Debug and monitor ESP8266 communication

## 📊 Performance

Error correction performance depends on:
- Code parameters (n, k, t)
- Error distribution (random vs burst)
- Field characteristics (GF size)
- Implementation efficiency

See individual project READMEs for specific performance characteristics.

## 🤝 Contributing

This is an academic project developed for the NIDUC course. Feel free to explore, learn, and adapt the code for educational purposes.

## 📜 License

Academic project - Wrocław University of Science and Technology

## 🔗 Related Topics

- Error Detection and Correction
- Coding Theory
- Digital Communications
- Galois Field Theory
- Polynomial Algebra
- ESP8266 Development
- Embedded Systems

## 📧 Contact

Project developed for NIDUC course at PWr (Politechnika Wrocławska / Wrocław University of Science and Technology) by Michał Ramus and Jakub Besz

---
*Last Updated: February 2026*
