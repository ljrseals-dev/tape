#include "jh24/TapeLossModel.hpp"
#include <cmath>

namespace jh24 {

void TapeLossModel::prepare(double sampleRate, const TapeCalibration& cal) {
    sampleRate_ = sampleRate;
    setCalibration(cal);
    reset();
}

void TapeLossModel::reset() {
    fixedLossL_.reset(); fixedLossR_.reset();
    envL_.reset(); envR_.reset();
    dynamicLpL_.reset(); dynamicLpR_.reset();
}

void TapeLossModel::setCalibration(const TapeCalibration& cal) {
    tapeSpeedMs_ = cal.tapeSpeedMs();
    hfLossAmount_ = cal.hfLossAmount;

    // Base cutoff depends on speed and head condition
    baseCutoffHz_ = (cal.speed == TapeSpeed::IPS15) ? 16000.0 : 20000.0;
    if (cal.headCondition == HeadCondition::Worn)
        baseCutoffHz_ *= 0.85;
    else if (cal.headCondition == HeadCondition::Fresh)
        baseCutoffHz_ *= 1.05;

    minCutoffHz_ = baseCutoffHz_ * 0.45;
    kHf_ = 0.28 * hfLossAmount_;

    envL_.setAttackRelease(sampleRate_, 0.5, 25.0);
    envR_.setAttackRelease(sampleRate_, 0.5, 25.0);

    designFixedLoss(cal);
    dynamicLpL_.setCutoff(sampleRate_, baseCutoffHz_);
    dynamicLpR_.setCutoff(sampleRate_, baseCutoffHz_);
}

void TapeLossModel::designFixedLoss(const TapeCalibration& cal) {
    // Mild fixed HF roll-off approximating gap + spacing + thickness
    // for a typical 0.25 mil gap and small residual spacing.
    double cutoff = (cal.speed == TapeSpeed::IPS15) ? 19000.0 : 24000.0;
    if (cal.headCondition == HeadCondition::Worn)
        cutoff *= 0.9;

    fixedLossL_.setLowPass(sampleRate_, cutoff, 0.707);
    fixedLossR_.setLowPass(sampleRate_, cutoff, 0.707);
}

void TapeLossModel::updateDynamicCutoff(double env, OnePoleLP& lp) noexcept {
    // env is roughly 0..1+ for typical signal levels
    const double e = clamp(env, 0.0, 2.0);
    const double fc = baseCutoffHz_ * std::exp(-kHf_ * e);
    const double limited = clamp(fc, minCutoffHz_, baseCutoffHz_);
    lp.setCutoff(sampleRate_, limited);
}

void TapeLossModel::process(double& left, double& right) {
    // Envelope of the recorded signal (pre-loss)
    const double eL = envL_.process(left);
    const double eR = envR_.process(right);

    updateDynamicCutoff(eL, dynamicLpL_);
    updateDynamicCutoff(eR, dynamicLpR_);

    // Fixed losses first
    left  = fixedLossL_.process(left);
    right = fixedLossR_.process(right);

    // Then level-dependent self-erasure
    left  = dynamicLpL_.process(left);
    right = dynamicLpR_.process(right);
}

} // namespace jh24