# BCH-basic - How to Build and Run

This guide provides step-by-step instructions for building and running the BCH encoder/decoder project on ESP8266 hardware.

## Prerequisites

### Software
- [PlatformIO IDE](https://platformio.org/install/ide) (VS Code extension) or PlatformIO Core
- USB-to-Serial drivers for ESP8266 (usually CH340 or CP2102)

### Hardware
- ESP8266 board (NodeMCU v2 or compatible)
- USB cable (data-capable)
- Computer with USB port

## Build and Upload Instructions

### Using PlatformIO IDE (Recommended)

#### 1. Open Project
```bash
# Navigate to project directory
cd BCH-basic
```

Open the folder in VS Code with PlatformIO extension installed.

#### 2. Select Environment

The project has one environment defined:
- `esp8266_tester` - Main testing environment

#### 3. Build Project

**Using VS Code:**
- Click on the PlatformIO icon in the sidebar
- Expand "esp8266_tester"
- Click "Build"

**Using Terminal:**
```bash
pio run -e esp8266_tester
```

#### 4. Upload to ESP8266

**Using VS Code:**
- Connect your ESP8266 via USB
- In PlatformIO sidebar, expand "esp8266_tester"
- Click "Upload"

**Using Terminal:**
```bash
pio run -e esp8266_tester -t upload
```

**Note:** If upload fails, try:
- Holding the FLASH/BOOT button during upload
- Checking if the correct COM port is detected
- Verifying USB cable supports data transfer

#### 5. Monitor Serial Output

**Using VS Code:**
- In PlatformIO sidebar, expand "esp8266_tester"
- Click "Monitor"

**Using Terminal:**
```bash
pio device monitor -e esp8266_tester
```

**Monitor Settings:**
- Baud Rate: 115200
- Auto-detected COM port

To exit monitor: Press `Ctrl+C`

### Using PlatformIO Core CLI

If you prefer the command line without VS Code:

```bash
# Navigate to project
cd /path/to/BCH-basic

# Install dependencies (first time only)
pio lib install

# Build
platformio run -e esp8266_tester

# Upload (connect ESP8266)
platformio run -e esp8266_tester -t upload

# Monitor
platformio device monitor -e esp8266_tester -b 115200
```

## Configuration Options

### Change COM Port

If your ESP8266 is on a different COM port, edit [platformio.ini](platformio.ini):

```ini
[env:esp8266_tester]
upload_port = COM3    ; Change to your COM port (Windows)
monitor_port = COM3   ; Change to your COM port (Windows)
; For Linux/Mac: /dev/ttyUSB0 or /dev/cu.usbserial-XXXX
```

### Change Baud Rate

In [platformio.ini](platformio.ini):
```ini
[env:esp8266_tester]
monitor_speed = 115200    ; Change if needed
```

### Enable Different Source Files

If you want to use sender/receiver mode (currently commented out):

Edit [platformio.ini](platformio.ini) and uncomment the desired environment:

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

Then build with:
```bash
pio run -e esp8266_sender -t upload
# or
pio run -e esp8266_receiver -t upload
```

## Running the Tester

### Expected Output

When you run the tester, you should see output similar to:

```
========================================
BCH Code Information
========================================
Field: GF(2^4)
Primitive Polynomial: 0x13 (x^4 + x + 1)
Code Parameters: n=15, k=7, t=2
Generator Polynomial: [... coefficients ...]
----------------------------------------

Testing BCH Encoder/Decoder...
Original message: [0, 1, 1, 0, 1, 0, 1]
Encoded codeword: [... encoded data ...]
Introducing errors...
Decoding...
Decoded message: [0, 1, 1, 0, 1, 0, 1]
Success! Message correctly decoded.
```

### Test Modes

The tester typically runs through various scenarios:
1. Clean encoding/decoding (no errors)
2. Single-bit error correction
3. Two-bit error correction
4. Error detection beyond correction capability

## Troubleshooting

### Upload Issues

**Problem:** Upload fails with "Failed to connect"
```
Solution:
1. Hold FLASH/BOOT button during upload
2. Check COM port in Device Manager (Windows) or ls /dev/tty* (Linux/Mac)
3. Install/update USB drivers (CH340/CP2102)
4. Try different USB port or cable
```

**Problem:** "Access denied" error
```
Solution:
1. Close any programs using the COM port (Arduino IDE, other monitors)
2. Run command prompt/terminal as administrator
3. Unplug and replug the ESP8266
```

### Build Issues

**Problem:** Library dependencies not found
```bash
# Solution: Update PlatformIO platform
pio platform update
pio lib update
```

**Problem:** Compilation errors
```bash
# Solution: Clean and rebuild
pio run -t clean
pio run -e esp8266_tester
```

### Serial Monitor Issues

**Problem:** No output in serial monitor
```
Solution:
1. Check baud rate is set to 115200
2. Press RESET button on ESP8266
3. Verify upload was successful
4. Check if correct COM port is selected
```

**Problem:** Garbage characters in output
```
Solution:
1. Baud rate mismatch - set to 115200
2. ESP8266 boot mode messages (ignore initial garbled text)
3. Try resetting the board
```

### Runtime Issues

**Problem:** ESP8266 keeps resetting
```
Possible causes:
1. Power supply insufficient (use USB 2.0 port or powered hub)
2. Software exception - check serial output for stack trace
3. Watchdog timer reset - code might be blocking too long
```

**Problem:** Incorrect decoding results
```
Check:
1. BCH parameters (m, t) are correctly set
2. Generator polynomial is correct
3. Error count doesn't exceed correction capability (t)
4. Memory isn't corrupted (ESP8266 limitations)
```

## Quick Commands Reference

```bash
# Build only
pio run -e esp8266_tester

# Build and upload
pio run -e esp8266_tester -t upload

# Upload without building (if already built)
pio run -e esp8266_tester -t nobuild -t upload

# Clean build files
pio run -e esp8266_tester -t clean

# Monitor serial output
pio device monitor -e esp8266_tester

# Build, upload, and monitor (all in one)
pio run -e esp8266_tester -t upload && pio device monitor -e esp8266_tester

# List connected devices
pio device list

# Update platform
pio platform update espressif8266
```

## Advanced: Custom BCH Parameters

To test with different BCH parameters, modify [src/tester.cpp](src/tester.cpp):

```cpp
// Example: GF(2^5), t=3 errors
int m = 5;              // Field extension
int t = 3;              // Error correction capability
uint16_t primPoly = 0x25; // Primitive polynomial for GF(2^5)

BCHEncoder encoder(m, t, primPoly);
```

Remember to rebuild after changes:
```bash
pio run -e esp8266_tester -t upload
```

## Next Steps

- Review [README.md](README.md) for theoretical background
- Examine [bch.hpp](lib/bch/bch.hpp) and [bch.cpp](lib/bch/bch.cpp) for implementation details
- Experiment with different error patterns
- Try implementing sender/receiver mode with two ESP8266 boards

## Support

For issues specific to:
- **PlatformIO:** Check [PlatformIO documentation](https://docs.platformio.org)
- **ESP8266:** Review [ESP8266 Arduino Core docs](https://arduino-esp8266.readthedocs.io)
- **BCH Theory:** Consult coding theory textbooks or the main README

---
*Last Updated: February 2026*
