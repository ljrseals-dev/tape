# Engineering Notes – JH-24 Tape Model

This document summarises the design decisions that turn the full engineering specification into a practical C++17 library.

## Magnetization core

Three modes are provided:

1. **Rational** (default) – odd rational function  
   \( F(x) = (x + a_3 x^3 + a_5 x^5) / (1 + b_2 x^2 + b_4 x^4) \)  
   Coefficients are lightly tuned per formulation and head condition.  
   Continuous, differentiable, progressive odd harmonics, efficient.

2. **TapeHack** – the exact polynomial series supplied in the original research note.  
   Retained as a legacy / comparison mode.

3. **Jiles-Atherton** – simplified discrete JA step.  
   Research / reference path only. A full bias-frequency simulation would require internal rates far above audio and is not attempted here.

## Equalisation

Record and reproduce paths are separate.  
Standard time constants are realised with high-shelf / low-shelf approximations that capture the essential amplitude response of NAB / IEC1 / AES.  
Phase-linearising networks present on the original JH-24 electronics are not modelled in detail; the resulting square-wave behaviour is therefore only approximate.

Head-bump is a parametric peaking filter whose centre frequency scales with tape speed.

## Losses

- Fixed mild low-pass approximates combined gap + residual spacing + thickness loss.  
- Level-dependent self-erasure is a one-pole whose cutoff falls with a smoothed envelope of the recorded signal.  
  This is a phenomenological model, not a microscopic self-erasure simulation.

## Transport

Speed variation is realised as a time-varying fractional delay.  
Components: slow random walk, low-frequency periodic term, higher-frequency scrape term.  
Depths are scaled from the published DIN-weighted JH-24 figures; the mapping is approximate.

## Noise

Three independent generators (tape floor, modulation/asperity, mechanical) with appropriate spectral shaping and level dependence.  
All PRNGs are instance-local and seedable for deterministic offline renders.

## Oversampling

Simple windowed-sinc FIR, factor 2 / 4 / 8.  
Applied only around the nonlinear magnetization stage.  
4× is the recommended production setting.

## Real-time constraints

- No allocations after `prepare()`  
- No locks  
- No global mutable state  
- All filter and noise state is per-channel / per-instance  

## Known limitations / future work

- Full JA bias-frequency simulation  
- Measured per-track head-contour curves  
- More accurate gap-loss (Westmijze tables)  
- Print-through (storage-dependent, currently omitted from the real-time path)  
- Explicit low-frequency weight stage (currently absorbed into head-bump + EQ)  

The library is intentionally conservative: it prefers physically motivated separations and published calibration anchors over subjective “hype” saturation.