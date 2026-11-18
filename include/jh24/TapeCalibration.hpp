#pragma once

#include "TapeTypes.hpp"

namespace jh24 {

/**
 * Complete calibration structure for a JH-24-style tape path.
 * All physical quantities are expressed in engineering units.
 */
struct TapeCalibration {
    // --- Core operating point ---
    TapeSpeed       speed               = TapeSpeed::IPS15;
    TapeEQ          equalization        = TapeEQ::NAB;
    TapeFormula     formulation         = TapeFormula::Ampex456;
    HeadCondition   headCondition       = HeadCondition::Maintained;
    MagnetizationMode magnetizationMode = MagnetizationMode::Rational;

    // Reference fluxivity (nWb/m) and operating level relative to it (dB)
    double referenceFluxivity   = kReference250;
    double recordLevelDb        = 0.0;      // 0 = reference, +6 = classic hot, etc.
    double overbiasDb           = 3.5;      // relative to 10 kHz peak (formulation dependent)

    // Head geometry & contour
    double headSpacingUm        = 0.5;      // micrometres effective spacing
    double effectiveGapUm       = 6.35;     // ~0.25 mil typical JH record/repro
    double headBumpFrequencyHz  = 55.0;     // nominal at 15 ips; scales with speed
    double headBumpGainDb       = 2.0;
    double headBumpQ            = 1.0;

    // Loss & weight controls
    double hfLossAmount         = 1.0;      // 0 = none, 1 = typical, >1 stronger
    double lowFrequencyWeight   = 0.35;     // 0..1, classic "mastering bass"

    // Transport
    double transportFlutter     = 0.04;     // DIN weighted target (%)
    double scrapeFlutter        = 0.008;    // additional high-freq component

    // Noise (relative to reference fluxivity, dB)
    double tapeNoiseDb          = -62.0;    // approximate unweighted floor
    double modulationNoiseDb    = -55.0;    // relative at 0 dB operating level

    // Processing
    double inputGain            = 1.0;
    double outputGain           = 1.0;
    double wet                  = 1.0;      // 0..1

    // Oversampling
    int    oversampleFactor     = 4;        // 2, 4 or 8 recommended

    // Random seed for deterministic noise (0 = non-deterministic)
    uint32_t randomSeed         = 0x4A483234; // "JH24"

    // Convenience setters for classic modes
    void setClassicAmerican15() {
        speed = TapeSpeed::IPS15;
        equalization = TapeEQ::NAB;
        formulation = TapeFormula::Ampex456;
        referenceFluxivity = kReference250;
        recordLevelDb = 0.0;
        overbiasDb = 3.5;
        headBumpFrequencyHz = 55.0;
        headBumpGainDb = 2.2;
    }

    void setHighHeadroom30() {
        speed = TapeSpeed::IPS30;
        equalization = TapeEQ::AES;
        formulation = TapeFormula::AgfaPEM468;
        referenceFluxivity = kReference250;
        recordLevelDb = 0.0;
        overbiasDb = 1.5;
        headBumpFrequencyHz = 35.0;
        headBumpGainDb = 1.5;
    }

    void setHotDriven15() {
        setClassicAmerican15();
        recordLevelDb = 6.0;
        overbiasDb = 3.0;
        hfLossAmount = 1.25;
        lowFrequencyWeight = 0.45;
    }

    // Derive tape speed in m/s
    double tapeSpeedMs() const noexcept {
        return (speed == TapeSpeed::IPS15) ? 0.381 : 0.762;
    }

    // Derive tape speed in ips
    double tapeSpeedIps() const noexcept {
        return (speed == TapeSpeed::IPS15) ? 15.0 : 30.0;
    }
};

} // namespace jh24