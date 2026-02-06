# RS-basic - Reed-Solomon Encoder/Decoder

## Overview

This project implements a basic Reed-Solomon (RS) encoder and decoder for ESP8266 microcontrollers. Reed-Solomon codes are non-binary BCH codes particularly effective for correcting burst errors in digital communication systems.

## Features

- Reed-Solomon encoding/decoding algorithms
- Galois Field arithmetic operations
- Lagrange polynomial interpolation
- ESP8266 wireless sender/receiver implementation
- Serial communication between devices

## Project Structure

```
RS-basic/
├── platformio.ini          # PlatformIO configuration
├── lib/
│   └── rs/
│       ├── rs.hpp          # RS header with function declarations
│       └── rs.cpp          # RS implementation
├── src/
│   ├── sender.cpp          # Sender device code
│   └── receiver.cpp        # Receiver device code
└── include/
    └── README              # Include directory info
```

## Hardware Requirements

- 2x ESP8266 boards (NodeMCU v2 or compatible)
- 2x USB cables for programming and power
- Jumper wires for serial communication
- Computer with USB ports

## Hardware Setup

### Wiring Configuration

```
ESP8266 #1 (Sender)          ESP8266 #2 (Receiver)
├── COM: COM8                ├── COM: COM5
├── TX: Pin 12      ────────────> RX: Pin 13
└── GND             ────────────> GND (common ground)
```

**Important:** Ensure both ESP8266 boards share a common ground connection.

## Software Requirements

### Installation

1. **Install PlatformIO:**
   - [PlatformIO IDE](https://platformio.org/install/ide) (VS Code extension recommended)
   - Or use PlatformIO Core CLI

2. **Install Dependencies:**
   - Arduino framework (automatically installed by PlatformIO)
   - ESP8266 platform support (automatically installed)

## How to Build and Run

### Using PlatformIO IDE (VS Code)

#### 1. Build the Project

For Sender:
```bash
pio run -e esp8266_sender
```

For Receiver:
```bash
pio run -e esp8266_receiver
```

#### 2. Upload to ESP8266

**Upload Sender:**
```bash
pio run -e esp8266_sender -t upload
```

**Upload Receiver:**
```bash
pio run -e esp8266_receiver -t upload
```

**Note:** Make sure the correct COM port is connected. Default configuration:
- Sender: COM8
- Receiver: COM5

To change COM ports, edit [platformio.ini](platformio.ini):
```ini
[env:esp8266_sender]
upload_port = COM8    # Change to your sender COM port

[env:esp8266_receiver]
upload_port = COM5    # Change to your receiver COM port
```

#### 3. Monitor Serial Output

**Monitor Sender:**
```bash
pio device monitor -e esp8266_sender
```

**Monitor Receiver:**
```bash
pio device monitor -e esp8266_receiver
```

To monitor both simultaneously, use two terminal windows or the PlatformIO IDE's built-in terminal manager.

### Using PlatformIO Core CLI

If using the command line directly:

```bash
# Navigate to project directory
cd RS-basic

# Build sender
platformio run -e esp8266_sender

# Upload sender (connect sender ESP8266)
platformio run -e esp8266_sender -t upload

# Build receiver
platformio run -e esp8266_receiver

# Upload receiver (connect receiver ESP8266)
platformio run -e esp8266_receiver -t upload

# Monitor output
platformio device monitor -e esp8266_sender
# or
platformio device monitor -e esp8266_receiver
```

## Configuration

### PlatformIO Environments

The project defines two environments in [platformio.ini](platformio.ini):

```ini
[env:esp8266_sender]
platform = espressif8266
board = nodemcuv2
framework = arduino
upload_port = COM8
monitor_speed = 115200
src_filter = +<sender.cpp>

[env:esp8266_receiver]
platform = espressif8266
board = nodemcuv2
framework = arduino
upload_port = COM5
monitor_speed = 115200
src_filter = +<receiver.cpp>
```

### Serial Communication Settings

- **Baud Rate:** 115200
- **Data Bits:** 8
- **Parity:** None
- **Stop Bits:** 1

## Reed-Solomon Algorithm

### Encoding Process

1. **Polynomial Representation:** Message represented as polynomial coefficients
2. **Evaluation:** Polynomial evaluated at multiple points in Galois Field
3. **Transmission:** Evaluated points sent as codeword

### Decoding Process

1. **Reception:** Receive transmitted points (possibly with errors)
2. **Lagrange Interpolation:** Reconstruct polynomial from received points
3. **Verification:** Check polynomial consistency
4. **Error Detection/Correction:** Use redundancy to correct errors

### Key Functions

```cpp
// Polynomial multiplication
void multiplyPoly(double poly[], int size, double a, double result[]);

// Polynomial addition
void addPoly(double a[], int sizeA, double b[], int sizeB, double res[]);

// Scalar multiplication
void scalePoly(double poly[], int size, double scalar, double res[]);

// Lagrange interpolation
void lagrangeInterpolation(double x[], double y[], int n, double coeffs[]);

// Coefficient comparison
bool coefficientsEqual(double coeffs1[], double coeffs2[], int size, double tolerance);
```

## Testing

### Basic Test Procedure

1. **Setup Hardware:** Wire both ESP8266 boards as described
2. **Upload Code:** Flash sender and receiver firmware
3. **Power On:** Ensure both boards are powered
4. **Monitor Output:** Open serial monitors for both devices
5. **Observe Transmission:** Watch encoded data transmission and decoding results

### Expected Output

**Sender Output:**
```
Initializing RS Sender...
Encoding message...
Sending codeword...
[Transmission data]
```

**Receiver Output:**
```
Initializing RS Receiver...
Waiting for data...
Received points: [data]
Decoding...
Decoded message: [result]
```

## Troubleshooting

### Common Issues

**1. Upload Failed:**
- Check COM port assignment
- Ensure USB cable supports data transfer
- Try pressing FLASH/BOOT button during upload
- Verify driver installation for ESP8266

**2. No Serial Output:**
- Check baud rate setting (115200)
- Verify serial monitor is connected to correct port
- Try resetting ESP8266

**3. Communication Errors:**
- Verify wiring connections (TX→RX, GND→GND)
- Check for loose connections
- Ensure common ground between devices
- Verify SoftwareSerial pin configuration

**4. Build Errors:**
- Update PlatformIO platform: `pio platform update`
- Clean build: `pio run -t clean`
- Check library dependencies

## Performance Considerations

- **Transmission Rate:** Depends on serial baud rate and encoding overhead
- **Error Correction Capability:** Based on code parameters (n, k)
- **Processing Time:** Limited by ESP8266 CPU (80/160 MHz)
- **Memory Usage:** Constrained by ESP8266 RAM (~80KB available)

## Limitations

- Basic implementation (not optimized for production)
- Limited error correction capability
- Floating-point arithmetic (not ideal for embedded systems)
- No hardware error checking

## Future Improvements

- Implement finite field arithmetic (instead of floating-point)
- Add error injection for testing
- Optimize memory usage
- Add multiple transmission modes
- Implement CRC checks
- Add configuration options via serial commands

## References

- Reed-Solomon Codes - Error Correction Coding Theory
- Galois Field Arithmetic
- Lagrange Interpolation
- ESP8266 Arduino Core Documentation

## Related Projects

- [BCH-basic](../BCH-basic/) - BCH encoder/decoder implementation
- [RS-gf31](../RS-gf31/) - Enhanced RS with GF(31) and comprehensive testing
- [Test-polynomial](../Test-polynomial/) - Polynomial interpolation tester

## License

Academic project - Wrocław University of Science and Technology (PWr)

---
*Part of the NIDUC course project series*
