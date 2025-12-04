# Quick Reference - Error Testing Modes

## Transmission Mode Cycle (Every 10 seconds)

```
┌─────────────────────────────────────────────────────────────┐
│  Mode 0: ✓✓✓ CLEAN                                          │
│  No errors - Baseline test                                  │
│  Expected: Successful decode without correction             │
├─────────────────────────────────────────────────────────────┤
│  Mode 1: ⚠️  1 Y-ERROR                                       │
│  Single error in y-coordinate                               │
│  Expected: Detected and CORRECTED ✓                         │
├─────────────────────────────────────────────────────────────┤
│  Mode 2: ❌❌ 2 Y-ERRORS                                      │
│  Two errors in y-coordinates                                │
│  Expected: Detected but NOT corrected ❌                     │
├─────────────────────────────────────────────────────────────┤
│  Mode 3: ⚠️  1 X-ERROR                                       │
│  Single error in x-coordinate (creates duplicate)           │
│  Expected: Structural error detected ❌                      │
├─────────────────────────────────────────────────────────────┤
│  Mode 4: ❌❌ 1 X-ERROR + 1 Y-ERROR                           │
│  Mixed coordinate errors                                    │
│  Expected: Multiple errors, cannot correct ❌                │
├─────────────────────────────────────────────────────────────┤
│  Mode 5: ❌❌❌ 2 X-ERRORS                                     │
│  Two errors in x-coordinates (severe corruption)            │
│  Expected: Critical failure ❌                               │
└─────────────────────────────────────────────────────────────┘
```

## Reed-Solomon (6,4) Capabilities

```
n = 6 transmitted points
k = 4 data points (polynomial degree 3)
t = 1 correctable errors = ⌊(n-k)/2⌋ = ⌊2/2⌋ = 1
d = 3 minimum distance = n-k+1

✓ Can CORRECT: 1 error in y-values
✓ Can DETECT:  2 errors in y-values
❌ Cannot correct: 2+ errors or any x-coordinate errors
```

## Error Types Explained

### Y-Coordinate Errors (Value Errors)
- Corrupt the polynomial evaluation result
- Keep evaluation points (x) intact
- RS can correct if count ≤ t

### X-Coordinate Errors (Structural Errors)
- Corrupt the evaluation point itself
- Create duplicate x-values or gaps
- Break mathematical structure
- RS cannot correct (needs unique x-values)

## Sender Output Symbols

```
✓        Clean point transmitted
🔴       Error in Y-coordinate
🔴       Error in X-coordinate  
🔴🔴     Error in both X and Y
```

## Receiver Output Symbols

```
✓        Successful decode/correction
⚠️       Warning (validation issue, attempting correction)
❌       Error detected, cannot correct
📊       Decoded polynomial coefficients
🔍       Analysis in progress
📦       New transmission received
📈       Statistics summary
```

## Statistics Tracked

- **Total Transmissions**: Count of all 6-point frames received
- **Successful Corrections**: Clean decodes + corrected single errors
- **Failed Corrections**: 2+ errors detected but not correctable
- **Success Rate**: (Successful / Total) × 100%

## Quick Test Checklist

- [ ] Mode 0: Verify clean decode (baseline)
- [ ] Mode 1: Verify single error correction works
- [ ] Mode 2: Verify dual error detection (not corrected)
- [ ] Mode 3: Verify x-duplicate detection
- [ ] Mode 4: Verify mixed error handling
- [ ] Mode 5: Verify critical corruption detection
- [ ] Check statistics accuracy after full cycle

## Expected Success Rates

After one complete 6-mode cycle:
- Successful: 2/6 (33%) - Mode 0 and Mode 1 only
- Failed: 4/6 (67%) - Modes 2, 3, 4, 5

This validates RS(6,4) can only correct 1 error!
