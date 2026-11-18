#include "jh24/TapeProcessor.hpp"
#include <algorithm>
#include <cstring>

namespace jh24 {

void TapeProcessor::prepare(double sampleRate, int maxBlockSize) {
    sampleRate_ = sampleRate;

    magnetization_.prepare(sampleRate, cal_);
    equalization_.prepare(sampleRate, cal_);
    loss_.prepare(sampleRate, cal_);
    transport_.prepare(sampleRate, cal_);
    noise_.prepare(sampleRate, cal_);

    const int osFactor = std::max(1, cal_.oversampleFactor);
    oversamplerL_.prepare(sampleRate, osFactor);
    oversamplerR_.prepare(sampleRate, osFactor);

    osBufferL_.resize(static_cast<size_t>(osFactor));
    osBufferR_.resize(static_cast<size_t>(osFactor));

    envL_.setAttackRelease(sampleRate, 0.8, 30.0);
    envR_.setAttackRelease(sampleRate, 0.8, 30.0);

    (void)maxBlockSize; // reserved for future block-based optimisations
    reset();
}

void TapeProcessor::reset() {
    magnetization_.reset();
    equalization_.reset();
    loss_.reset();
    transport_.reset();
    noise_.reset();
    oversamplerL_.reset();
    oversamplerR_.reset();
    envL_.reset();
    envR_.reset();
}

void TapeProcessor::setCalibration(const TapeCalibration& cal) {
    cal_ = cal;
    magnetization_.setCalibration(cal_);
    equalization_.setCalibration(cal_);
    loss_.setCalibration(cal_);
    transport_.setCalibration(cal_);
    noise_.setCalibration(cal_);

    const int osFactor = std::max(1, cal_.oversampleFactor);
    oversamplerL_.prepare(sampleRate_, osFactor);
    oversamplerR_.prepare(sampleRate_, osFactor);
    osBufferL_.resize(static_cast<size_t>(osFactor));
    osBufferR_.resize(static_cast<size_t>(osFactor));
}

void TapeProcessor::processCore(double& left, double& right) {
    // Preserve dry for wet/dry
    const double dryL = left;
    const double dryR = right;

    // 1. Input gain
    left  *= cal_.inputGain;
    right *= cal_.inputGain;

    // 2. Record-side EQ
    equalization_.processRecord(left, right);

    // 3. Oversampled magnetization
    const int os = oversamplerL_.factor();
    if (os > 1) {
        oversamplerL_.upsample(left,  osBufferL_.data());
        oversamplerR_.upsample(right, osBufferR_.data());

        for (int i = 0; i < os; ++i) {
            double l = osBufferL_[i];
            double r = osBufferR_[i];
            magnetization_.processStereo(l, r);
            osBufferL_[i] = l;
            osBufferR_[i] = r;
        }

        left  = oversamplerL_.downsample(osBufferL_.data());
        right = oversamplerR_.downsample(osBufferR_.data());
    } else {
        magnetization_.processStereo(left, right);
    }

    // Envelope for noise & any future dynamic stages
    const double eL = envL_.process(left);
    const double eR = envR_.process(right);

    // 4. Recording losses (level-dependent HF + fixed gap/spacing)
    loss_.process(left, right);

    // 5. Transport (wow / flutter / scrape)
    transport_.process(left, right);

    // 6 + 7. Reproduce head response (bump) + reproduce EQ
    equalization_.processReproduce(left, right);

    // 8. Noise
    noise_.process(left, right, eL, eR);

    // 9. Output gain + wet/dry
    left  = left  * cal_.outputGain * cal_.wet + dryL * (1.0 - cal_.wet);
    right = right * cal_.outputGain * cal_.wet + dryR * (1.0 - cal_.wet);
}

void TapeProcessor::processSample(double& left, double& right) {
    processCore(left, right);
}

void TapeProcessor::process(float* left, float* right, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        double l = left[i];
        double r = right[i];
        processCore(l, r);
        left[i]  = static_cast<float>(l);
        right[i] = static_cast<float>(r);
    }
}

void TapeProcessor::process(float* interleavedStereo, int numFrames) {
    for (int i = 0; i < numFrames; ++i) {
        double l = interleavedStereo[i * 2];
        double r = interleavedStereo[i * 2 + 1];
        processCore(l, r);
        interleavedStereo[i * 2]     = static_cast<float>(l);
        interleavedStereo[i * 2 + 1] = static_cast<float>(r);
    }
}

} // namespace jh24