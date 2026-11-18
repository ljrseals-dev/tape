#pragma once

#include "TapeTypes.hpp"
#include "TapeCalibration.hpp"
#include "Utilities.hpp"

namespace jh24 {

/**
 * Frequency- and level-dependent recording losses:
 *  - Gap loss (Westmijze / sinc approximation)
 *  - Wallace spacing loss
 *  - Coating thickness loss
 *  - Level-dependent self-erasure / HF compression
 *
 * The level-dependent path is a continuously variable low-pass whose
 * cutoff is modulated by a smoothed envelope of the recorded signal.
 */
class TapeLossModel {
public:
    void prepare(double sampleRate, const TapeCalibration& cal);
    void reset();
    void setCalibration(const TapeCalibration& cal);

    void process(double& left, double& right);

private:
    double sampleRate_ = 44100.0;
    double tapeSpeedMs_ = 0.381;
    double hfLossAmount_ = 1.0;

    // Fixed losses (gap + spacing + thickness) – mild fixed HF roll-off
    Biquad fixedLossL_, fixedLossR_;

    // Level-dependent self-erasure
    EnvelopeFollower envL_, envR_;
    OnePoleLP dynamicLpL_, dynamicLpR_;
    double baseCutoffHz_ = 18000.0;
    double minCutoffHz_  = 8000.0;
    double kHf_ = 0.35;          // sensitivity of cutoff to level

    void designFixedLoss(const TapeCalibration& cal);
    void updateDynamicCutoff(double env, OnePoleLP& lp) noexcept;
};

} // namespace jh24