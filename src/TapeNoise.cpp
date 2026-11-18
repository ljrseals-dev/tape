#include "jh24/TapeNoise.hpp"
#include <cmath>

namespace jh24 {

void TapeNoise::prepare(double sampleRate, const TapeCalibration& cal) {
    sampleRate_ = sampleRate;
    setCalibration(cal);
    reset();
}

void TapeNoise::reset() {
    tapeLpL_.reset(); tapeLpR_.reset();
    modLpL_.reset();  modLpR_.reset();
    transportLpL_.reset(); transportLpR_.reset();
    modStateL_ = modStateR_ = 0.0;
}

void TapeNoise::setCalibration(const TapeCalibration& cal) {
    // Convert dB to linear gain relative to a nominal full-scale signal
    tapeNoiseGain_  = dbToLinear(cal.tapeNoiseDb);
    modNoiseGain_   = dbToLinear(cal.modulationNoiseDb);
    transportGain_  = dbToLinear(cal.tapeNoiseDb - 12.0); // quieter mechanical component

    // Adjust for head condition
    if (cal.headCondition == HeadCondition::Worn) {
        tapeNoiseGain_ *= 1.25;
        modNoiseGain_  *= 1.4;
    }

    rngL_ = Random(cal.randomSeed ^ 0x4E4F4953u); // "NOIS"
    rngR_ = Random(cal.randomSeed ^ 0x52494748u); // "RIGH"

    designFilters();
}

void TapeNoise::designFilters() {
    // Tape noise – slightly rolled off at extremes
    tapeLpL_.setCutoff(sampleRate_, 12000.0);
    tapeLpR_.setCutoff(sampleRate_, 12000.0);

    // Modulation / asperity – emphasis on lower midrange
    modLpL_.setCutoff(sampleRate_, 2500.0);
    modLpR_.setCutoff(sampleRate_, 2500.0);

    // Transport – low-passed mechanical
    transportLpL_.setCutoff(sampleRate_, 800.0);
    transportLpR_.setCutoff(sampleRate_, 800.0);
}

void TapeNoise::process(double& left, double& right, double envL, double envR) {
    // 1. Constant tape / residual bias noise
    const double tL = tapeLpL_.process(rngL_.nextGaussianApprox()) * tapeNoiseGain_;
    const double tR = tapeLpR_.process(rngR_.nextGaussianApprox()) * tapeNoiseGain_;

    // 2. Level-dependent modulation / asperity noise
    const double mL = modLpL_.process(rngL_.nextGaussianApprox())
                    * modNoiseGain_ * (0.15 + std::abs(envL));
    const double mR = modLpR_.process(rngR_.nextGaussianApprox())
                    * modNoiseGain_ * (0.15 + std::abs(envR));

    // 3. Transport / roller noise with weak slow modulation
    modStateL_ = modStateL_ * 0.9995 + rngL_.nextUniform() * 0.001;
    modStateR_ = modStateR_ * 0.9995 + rngR_.nextUniform() * 0.001;

    const double trL = transportLpL_.process(rngL_.nextGaussianApprox())
                     * transportGain_ * (1.0 + modStateL_ * 2.0);
    const double trR = transportLpR_.process(rngR_.nextGaussianApprox())
                     * transportGain_ * (1.0 + modStateR_ * 2.0);

    left  += tL + mL + trL;
    right += tR + mR + trR;
}

} // namespace jh24