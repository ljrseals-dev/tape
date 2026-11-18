# JH-24 Tape Model

**Definitive C++17 reference implementation of a well-maintained but real-world-used MCI JH-24 (or equivalent 2-inch 24-track) analog tape path.**

Operating speeds: 15 ips / 30 ips  
Oxide formulations: Scotch 250, Ampex 456, Agfa PEM 468 (and custom)  
Equalisation: NAB, IEC1/CCIR, AES  

This library implements the complete signal path described in the accompanying engineering specification:

- Record electronics + equalisation  
- AC-biased magnetic magnetization (rational, TapeHack legacy, or simplified Jiles-Atherton)  
- Level-dependent self-erasure / HF compression  
- Gap, spacing and thickness losses  
- Head-bump / contour effect  
- Transport wow, flutter and scrape  
- Multi-component noise (tape, modulation/asperity, mechanical)  
- Reproduce equalisation and head response  

The design deliberately separates physical processes rather than collapsing everything into a single waveshaper.

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

This produces:

- `libjh24.a` (or `.so` / `.dylib` if shared)  
- `jh24_demo` – generates a 3-second test signal and writes a raw float32 stereo file  
- `jh24_offline` – offline processor for existing raw float32 files  

Requirements: C++17 compiler (GCC 9+, Clang 10+, MSVC 2019+), CMake 3.16+.

## Quick start

```cpp
#include "jh24/jh24.hpp"

jh24::TapeCalibration cal;
cal.setClassicAmerican15();   // 15 ips NAB Ampex 456 @ 250 nWb/m
cal.recordLevelDb = 3.0;      // moderately hot
cal.oversampleFactor = 4;

jh24::TapeProcessor tape;
tape.prepare(48000.0);
tape.setCalibration(cal);

// Process interleaved float32 stereo
tape.process(interleavedBuffer, numFrames);

// Or separate channels
tape.process(left, right, numSamples);
```

### Classic operating modes

| Helper                    | Speed | EQ  | Tape        | Character                          |
|---------------------------|-------|-----|-------------|------------------------------------|
| `setClassicAmerican15()`  | 15    | NAB | Ampex 456   | Glue, weight, classic multitrack   |
| `setHighHeadroom30()`     | 30    | AES | PEM 468     | Cleaner, extended HF, less bump    |
| `setHotDriven15()`        | 15    | NAB | Ampex 456   | Stronger saturation & HF softening |

## Architecture

```
TapeProcessor
 ├── TapeEqualization     (record + reproduce NAB/IEC/AES + head-bump)
 ├── TapeMagnetization    (Rational / TapeHack / JA)
 ├── TapeLossModel        (fixed losses + level-dependent self-erasure)
 ├── TapeTransport        (wow / flutter / scrape via fractional delay)
 ├── TapeNoise            (tape + modulation + mechanical)
 └── Oversampler         (2×/4×/8× FIR for the nonlinear core)
```

All state is per-instance and per-channel. No global mutable state, no blocking calls in the audio path.

## Calibration parameters

See `TapeCalibration.hpp` for the full set. Key physical quantities:

- `referenceFluxivity` (185 / 250 / 320 / 510 nWb/m)  
- `recordLevelDb` (operating level relative to reference)  
- `overbiasDb` (formulation-specific)  
- `headBumpFrequencyHz`, `headBumpGainDb`, `headBumpQ`  
- `hfLossAmount`, `lowFrequencyWeight`  
- `transportFlutter`, `scrapeFlutter`  
- `tapeNoiseDb`, `modulationNoiseDb`  
- `headCondition` (Fresh / Maintained / Worn)  

## Verification targets (published JH-24)

- 15 ips NAB frequency response ≈ 30 Hz–26 kHz (+1.5 / −2 dB)  
- 30 ips AES ≈ 36 Hz–26 kHz (+1.5 / −3 dB)  
- 3rd harmonic at 510 nWb/m: < 0.50 % (15 ips), < 0.35 % (30 ips)  
- Wow & flutter (DIN weighted): < 0.04 % (15 ips), < 0.03 % (30 ips)  

## License

MIT (see LICENSE).  

This is a research / educational reference implementation. It is not affiliated with MCI, Sony, or any tape manufacturer.

## References

- Wallace, R.L. – *The Reproduction of Magnetically Recorded Signals* (Bell System Technical Journal, 1951)  
- Westmijze – gap-loss theory  
- Jiles & Atherton – ferromagnetic hysteresis model  
- MCI JH-24 Series Specifications & Service Bulletins  
- Contemporary bias optimisation and tape data sheets (Scotch 250, Ampex 456, Agfa PEM 468)  
- NAB / IEC / AES equalisation standards  

---

*Designed to serve as a historically grounded digital twin of the MCI JH-24 tape path.*