#pragma once

#include "TapeTypes.hpp"
#include "TapeCalibration.hpp"
#include "Utilities.hpp"

namespace jh24 {

/**
 * Multi-component noise model:
 *  - Tape particle / residual bias noise (constant floor)
 *  - Modulation / asperity noise (level-dependent)
 *  - Transport / roller mechanical noise (weakly modulated)
 *
 * All sources are independent per channel and use a deterministic PRNG
 * seeded from the calibration.
 */
class TapeNoise {
public:
    void prepare(double sampleRate, const TapeCalibration& cal);
    void reset();
    void setCalibration(const TapeCalibration& cal);

    // Add noise components into the signal (in-place)
    void process(double& left, double& right, double envL, double envR);

private:
    double sampleRate_ = 44100.0;

    // Gains (linear)
    double tapeNoiseGain_ = 0.0;
    double modNoiseGain_  = 0.0;
    double transportGain_ = 0.0;

    // Filters
    OnePoleLP tapeLpL_, tapeLpR_;
    OnePoleLP modLpL_,  modLpR_;
    OnePoleLP transportLpL_, transportLpR_;

    // Slow modulation of transport noise
    double modStateL_ = 0.0;
    double modStateR_ = 0.0;

    Random rngL_, rngR_;

    void designFilters();
};

} // namespace jh24