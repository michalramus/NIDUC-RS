# Reed-Solomon GF(31) - Project Structure

```
RS-gf31/
│
├── platformio.ini                 # PlatformIO configuration
│   ├── [env:esp8266_gf31_sender]      # Sender environment (COM8)
│   └── [env:esp8266_gf31_receiver]    # Receiver environment (COM5)
│
├── lib/
│   └── gf31_math/
│       └── gf31_math.hpp          # GF(31) arithmetic operations
│           ├── gf_add()               # Addition mod 31
│           ├── gf_mul()               # Multiplication mod 31
│           ├── gf_pow()               # Fast exponentiation
│           └── gf_inv()               # Multiplicative inverse
│
├── src/
│   ├── gf31_sender.cpp            # ★ ENHANCED SENDER ★
│   │   ├── Modes: 0-5 (6 test modes)
│   │   ├── introduce_y_error()    # Y-coordinate error injection
│   │   ├── introduce_x_error()    # X-coordinate error injection
│   │   ├── poly_eval()            # Polynomial evaluation
│   │   ├── send_point()           # Frame transmission
│   │   └── send_transmission()    # Main transmission logic
│   │
│   └── gf31_receiver.cpp          # ★ ENHANCED RECEIVER ★
│       ├── is_valid_x()           # X-coordinate validation
│       ├── is_valid_y()           # Y-coordinate validation
│       ├── hasDuplicateX()        # Duplicate detection
│       ├── analyzeXDistribution() # X-value analysis
│       ├── lagrange_interpolate() # Polynomial reconstruction
│       ├── verify_points()        # Polynomial verification
│       ├── reed_solomon_decode()  # RS error correction
│       └── Statistics tracking    # Success rate monitoring
│
├── Documentation/
│   ├── TESTING_GUIDE.md           # ★ NEW ★ Comprehensive testing guide
│   ├── QUICK_REFERENCE.md         # ★ NEW ★ Quick reference card
│   ├── ENHANCEMENT_SUMMARY.md     # ★ NEW ★ Changes summary
│   └── PROJECT_STRUCTURE.md       # ★ THIS FILE ★
│
└── Hardware Setup:
    ├── ESP8266 #1 (Sender)        # NodeMCU v2, COM8
    │   └── SoftwareSerial TX: Pin 12
    │
    └── ESP8266 #2 (Receiver)      # NodeMCU v2, COM5
        └── SoftwareSerial RX: Pin 13
```

## Data Flow

```
┌──────────────────────────────────────────────────────────────────┐
│                         SENDER (COM8)                            │
├──────────────────────────────────────────────────────────────────┤
│  1. Define Polynomial: P(x) = a₀ + a₁x + a₂x² + a₃x³ (mod 31)  │
│     coeffs[] = {5, 7, 3, 2}                                     │
│                                                                  │
│  2. Generate Points:                                             │
│     x = 0: P(0) = 5                                             │
│     x = 1: P(1) = 17                                            │
│     x = 2: P(2) = 11                                            │
│     x = 3: P(3) = 11                                            │
│     x = 4: P(4) = 3                                             │
│     x = 5: P(5) = 16                                            │
│                                                                  │
│  3. Introduce Errors (based on mode):                           │
│     Mode 0: None                                                │
│     Mode 1: 1 Y-error                                           │
│     Mode 2: 2 Y-errors                                          │
│     Mode 3: 1 X-error (creates duplicate)                       │
│     Mode 4: 1 X + 1 Y error                                     │
│     Mode 5: 2 X-errors                                          │
│                                                                  │
│  4. Encode Frames (8-bit):                                      │
│     [xxx|yyyyy] where x ∈ [0,5], y ∈ [0,30]                    │
│                                                                  │
│  5. Transmit via SoftwareSerial (9600 baud)                     │
│     Pin 12 (TX) ──────────────────┐                            │
└───────────────────────────────────┼─────────────────────────────┘
                                    │
                                    │ Serial Link
                                    │
┌───────────────────────────────────┼─────────────────────────────┐
│                                   └──> Pin 13 (RX)              │
├──────────────────────────────────────────────────────────────────┤
│                        RECEIVER (COM5)                           │
├──────────────────────────────────────────────────────────────────┤
│  1. Receive 6 Frames (8-bit each)                               │
│     Decode: x = (frame >> 5) & 0x07                             │
│             y = frame & 0x1F                                     │
│                                                                  │
│  2. Validate Coordinates:                                        │
│     ✓ Check x ∈ [0, 5]                                          │
│     ✓ Check y ∈ [0, 30]                                         │
│     ✓ Detect duplicate x values                                 │
│     ✓ Analyze x distribution                                    │
│                                                                  │
│  3. Reed-Solomon Decoding:                                       │
│     a) Try interpolation with first 4 points                    │
│     b) Verify all points against polynomial                     │
│     c) If mismatch:                                             │
│        - Try excluding each point (error location search)       │
│        - Interpolate with remaining 5 points                    │
│        - Check if all 5 match → error corrected!               │
│     d) If still fails → 2+ errors, cannot correct               │
│                                                                  │
│  4. Report Results:                                              │
│     ✓ Decoded coefficients (if successful)                      │
│     ✓ Error locations and corrections                           │
│     ✓ Success/failure status                                    │
│     ✓ Statistics (success rate)                                 │
└──────────────────────────────────────────────────────────────────┘
```

## Test Mode Cycle (10-second intervals)

```
    START
      │
      ▼
┌─────────────┐
│   Mode 0    │  No errors
│   ✓✓✓       │  Expected: Success
└─────┬───────┘
      │ 10s
      ▼
┌─────────────┐
│   Mode 1    │  1 Y-error
│   ⚠️        │  Expected: Corrected ✓
└─────┬───────┘
      │ 10s
      ▼
┌─────────────┐
│   Mode 2    │  2 Y-errors
│   ❌❌      │  Expected: Detected, not corrected
└─────┬───────┘
      │ 10s
      ▼
┌─────────────┐
│   Mode 3    │  1 X-error
│   ⚠️        │  Expected: Structural error
└─────┬───────┘
      │ 10s
      ▼
┌─────────────┐
│   Mode 4    │  1 X + 1 Y error
│   ❌❌      │  Expected: Cannot correct
└─────┬───────┘
      │ 10s
      ▼
┌─────────────┐
│   Mode 5    │  2 X-errors
│   ❌❌❌    │  Expected: Critical failure
└─────┬───────┘
      │ 10s
      │
      └──────> (Loop back to Mode 0)
```

## Error Correction Algorithm (Receiver)

```
Input: 6 points (x_i, y_i)
Output: 4 coefficients {a₀, a₁, a₂, a₃} or ERROR

┌─────────────────────────────────────────────────────────┐
│ 1. Lagrange Interpolation with first 4 points          │
│    P(x) = Σ y_i · L_i(x)                                │
│    where L_i(x) = Π(x - x_j)/(x_i - x_j) for j ≠ i    │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│ 2. Verify all 6 points satisfy P(x)                    │
│    For each point: check if y_i == P(x_i)              │
└────────┬────────────────────────────────┬───────────────┘
         │                                │
    All match                        Mismatch found
         │                                │
         ▼                                ▼
┌────────────────┐          ┌────────────────────────────┐
│ SUCCESS        │          │ 3. Error Correction Attempt│
│ No errors      │          │    Try excluding each point │
│ Return coeffs  │          └────────┬───────────────────┘
└────────────────┘                   │
                                     ▼
                        ┌────────────────────────────────┐
                        │ For i = 0 to 5:                │
                        │   - Exclude point i            │
                        │   - Interpolate with 4 of      │
                        │     remaining 5 points         │
                        │   - Check if all 5 match P(x) │
                        └─────┬──────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
               Found match          No match found
                    │                   │
                    ▼                   ▼
         ┌──────────────────┐   ┌─────────────────┐
         │ SUCCESS          │   │ FAILURE         │
         │ 1 error corrected│   │ 2+ errors       │
         │ Return coeffs    │   │ Cannot correct  │
         └──────────────────┘   └─────────────────┘
```

## Reed-Solomon Code Parameters

```
Code: RS(n, k) over GF(31)

n = 6     Total symbols (transmitted points)
k = 4     Data symbols (polynomial coefficients)
r = 2     Redundancy (n - k)
d = 3     Minimum distance (n - k + 1)

Capabilities:
├─ Error Correction:   t = ⌊(d-1)/2⌋ = 1 error
├─ Error Detection:    d - 1 = 2 errors
└─ Erasure Correction: d - 1 = 2 erasures (if locations known)

Field: GF(31) - Galois Field with 31 elements {0,1,2,...,30}
       Prime field (31 is prime)
```

## File Sizes (Approximate)

```
gf31_sender.cpp:   ~250 lines  (enhanced from ~170)
gf31_receiver.cpp: ~320 lines  (enhanced from ~266)
gf31_math.hpp:     ~40 lines   (unchanged)
TESTING_GUIDE.md:  ~400 lines  (new)
QUICK_REFERENCE.md: ~100 lines (new)
```

## Compilation Targets

```bash
# Build sender
pio run -e esp8266_gf31_sender

# Build receiver  
pio run -e esp8266_gf31_receiver

# Upload sender
pio run -e esp8266_gf31_sender -t upload

# Upload receiver
pio run -e esp8266_gf31_receiver -t upload

# Monitor sender
pio device monitor -e esp8266_gf31_sender

# Monitor receiver
pio device monitor -e esp8266_gf31_receiver
```

## Key Enhancements

1. **Error Injection**: Can now test X and Y coordinate errors
2. **Error Detection**: Comprehensive validation and analysis
3. **Statistics**: Tracks success rates across transmissions
4. **Documentation**: Complete testing and reference guides
5. **Visualization**: Clear output with emoji indicators

This structure provides a complete testing framework for Reed-Solomon error correction!
