# Reed-Solomon GF(31) - Comprehensive Testing Guide

## Overview

This project now includes extensive error testing capabilities to validate the Reed-Solomon error correction algorithm. The system can introduce controlled errors in both **x-coordinates** and **y-coordinates** of transmitted frames.

## Test Modes

The sender cycles through 6 different transmission modes every 10 seconds:

### Mode 0: Clean Transmission ✓
- **Description**: All 6 points transmitted correctly
- **Expected Result**: Receiver decodes successfully without error correction
- **Purpose**: Baseline verification

### Mode 1: Single Y-Error ⚠️
- **Description**: Introduces 1 error in the y-coordinate of a random point
- **Error Type**: Changes y-value by adding a random value (1-30) in GF(31)
- **Expected Result**: Receiver detects and corrects the single error
- **RS Capability**: Should correct successfully (1 error ≤ (n-k)/2 = 1)

### Mode 2: Double Y-Error ❌❌
- **Description**: Introduces 2 errors in y-coordinates at different random positions
- **Error Type**: Changes both y-values independently
- **Expected Result**: Receiver detects errors but **cannot correct** (exceeds correction capability)
- **RS Capability**: Can only detect, not correct (2 errors > (n-k)/2 = 1)

### Mode 3: Single X-Error ⚠️
- **Description**: Introduces 1 error in the x-coordinate
- **Error Type**: Changes x-value to a different value in range [0, 5]
- **Impact**: 
  - Creates duplicate x-values (collision)
  - Or creates missing x-value gaps
- **Expected Result**: Receiver detects duplicate x or missing x values
- **Note**: This corrupts the mathematical structure - cannot be corrected by RS alone

### Mode 4: Mixed X+Y Error ❌❌
- **Description**: Introduces 1 error in x AND 1 error in y (different points)
- **Error Type**: Changes one point's x-coordinate and another point's y-coordinate
- **Expected Result**: Multiple structural errors - receiver cannot correct
- **Impact**: Demonstrates worst-case scenario with coordinate corruption

### Mode 5: Double X-Error ❌❌❌
- **Description**: Introduces 2 errors in x-coordinates at different positions
- **Error Type**: Changes 2 x-values to different random values
- **Impact**: Creates severe structural corruption (multiple duplicates/gaps)
- **Expected Result**: Critical failure - receiver detects severe corruption
- **Note**: Mathematically impossible to correct

## Frame Structure

Each transmitted point is encoded in an 8-bit frame:

```
Bit:  7  6  5  4  3  2  1  0
     [x][x][x][y][y][y][y][y]
```

- **Bits 7-5**: x-coordinate (3 bits, range 0-7, valid 0-5)
- **Bits 4-0**: y-coordinate (5 bits, range 0-31, valid 0-30)

## Error Introduction Mechanisms

### Y-Coordinate Errors (`introduce_y_error`)
```cpp
int introduce_y_error(int y) {
  int error = random(1, 31);
  return gf_add(y, error);  // Add error in GF(31)
}
```
- Adds random non-zero value in GF(31)
- Preserves valid range [0, 30]
- Simulates bit errors or noise in y-value

### X-Coordinate Errors (`introduce_x_error`)
```cpp
int introduce_x_error(int x) {
  int new_x = random(0, 6);
  while (new_x == x) {
    new_x = random(0, 6);
  }
  return new_x;
}
```
- Changes x to a different valid value
- Creates duplicate x values (collision)
- Simulates addressing errors or frame corruption

## Receiver Capabilities

### Error Detection
1. **Duplicate X Detection**: Identifies when multiple points share same x-coordinate
2. **Out-of-Range Detection**: Validates x ∈ [0, 5] and y ∈ [0, 30]
3. **X Distribution Analysis**: Shows count of each x-value received
4. **Polynomial Verification**: Checks if all points satisfy the decoded polynomial

### Error Correction
- **Single Y-Error**: Can correct using Reed-Solomon algorithm
- **Multiple Errors**: Can detect but not correct (reports failure)
- **X-Coordinate Errors**: Cannot correct (structural corruption)

### Statistics Tracking
The receiver maintains running statistics:
- Total transmissions received
- Successfully decoded/corrected transmissions
- Failed corrections
- Success rate percentage

## Expected Behavior by Mode

| Mode | Error Type | Can Detect? | Can Correct? | Expected Output |
|------|------------|-------------|--------------|-----------------|
| 0 | None | N/A | N/A | ✓ Clean decode |
| 1 | 1 Y-error | ✓ Yes | ✓ Yes | ✓ Corrected decode |
| 2 | 2 Y-errors | ✓ Yes | ❌ No | ❌ Detection only |
| 3 | 1 X-error | ✓ Yes (duplicate) | ❌ No | ❌ Structural error |
| 4 | 1X + 1Y | ✓ Yes | ❌ No | ❌ Multiple errors |
| 5 | 2 X-errors | ✓ Yes (duplicates) | ❌ No | ❌ Critical corruption |

## Testing Procedure

### Hardware Setup
1. Connect two ESP8266 NodeMCU boards
2. **Sender**: Upload to COM8 (or update `platformio.ini`)
3. **Receiver**: Upload to COM5 (or update `platformio.ini`)
4. Connect SoftwareSerial: TX(pin 12) → RX(pin 13)

### Running Tests
1. Open two serial monitors (115200 baud)
2. **Sender monitor**: Shows transmission mode and errors introduced
3. **Receiver monitor**: Shows decoding process and results
4. Observe automatic 10-second cycle through all modes

### What to Observe

**Sender Output:**
- Current transmission mode header
- List of errors being introduced (position, original, corrupted values)
- Each transmitted point with error markers (🔴 for errors, ✓ for clean)

**Receiver Output:**
- Received points with validation warnings
- X-distribution analysis showing duplicates/gaps
- Error detection and correction attempts
- Decoded polynomial coefficients (if successful)
- Running statistics

## Reed-Solomon Theory Validation

This testing suite validates key RS properties:

### Proven Capabilities
- **Minimum Distance (d=3)**: Can detect up to 2 errors
- **Error Correction (t=1)**: Can correct up to (n-k)/2 = (6-4)/2 = 1 error
- **Erasure Correction (e=2)**: Can correct up to n-k = 2 erasures (if locations known)

### Limitations Demonstrated
- **Cannot correct 2 random errors** in values
- **Cannot handle x-coordinate corruption** (requires unique evaluation points)
- **Structural integrity required**: RS assumes evaluation point set is known

## Educational Value

This comprehensive test suite demonstrates:

1. **Error Detection vs Correction**: Modes 2-5 show detection without correction
2. **Coordinate Integrity**: X-errors show importance of evaluation point accuracy
3. **Redundancy Trade-offs**: 2 redundant points allow 1 error correction
4. **Statistical Analysis**: Success rate tracking validates theoretical predictions
5. **Real-world Constraints**: Physical layer errors affect different parts of frames

## Advanced Experiments

### Manual Mode Selection
Modify `transmission_mode` initial value in sender to start at specific mode:
```cpp
int transmission_mode = 3;  // Start with X-error testing
```

### Adjust Cycle Timing
Change delay in sender's `loop()`:
```cpp
delay(5000);  // Faster 5-second cycles
```

### Custom Error Patterns
Create specific error scenarios by modifying error introduction logic:
```cpp
// Always introduce error at position 2
int error_position = 2;  // Instead of random(0, 6)
```

## Troubleshooting

### No Reception
- Check SoftwareSerial connections (pins 12, 13)
- Verify baud rate (9600) matches on both devices
- Check power and ground connections

### Incorrect Error Detection
- Ensure random seed is working: `randomSeed(analogRead(0))`
- Verify frame encoding/decoding matches
- Check GF(31) arithmetic functions

### Statistics Not Updating
- Confirm receiver is successfully decoding frames
- Check that `count == 6` condition is reached
- Verify statistics increment in `reed_solomon_decode()`

## Files Modified

1. **`src/gf31_sender.cpp`**: Enhanced with 6 test modes, x-error introduction
2. **`src/gf31_receiver.cpp`**: Added x-validation, statistics, detailed analysis
3. **`lib/gf31_math/gf31_math.hpp`**: No changes (core math remains stable)

## Conclusion

This comprehensive testing framework allows systematic validation of Reed-Solomon error correction over GF(31), demonstrating both its powerful correction capabilities and its fundamental limitations when structural integrity is compromised.
