#pragma once

#include "TapeTypes.hpp"
#include "TapeCalibration.hpp"
#include "TapeMagnetization.hpp"
#include "TapeEqualization.hpp"
#include "TapeLossModel.hpp"
#include "TapeTransport.hpp"
#include "TapeNoise.hpp"
#include "Oversampler.hpp"
#include "Utilities.hpp"

namespace jh24 {

/**
 * Complete JH-24 tape-path processor.
 *
 * Signal flow (per the engineering specification):
 *
 *  1. Input calibration / drive
 *  2. Record-side EQ
 *  3. Oversampled magnetization (rational / TapeHack / JA)
 *  4. Level-dependent recording losses
 *  5. Transport modulation (wow / flutter / scrape)
 *  6. Reproduce-head response + head-bump
 *  7. Reproduce EQ
 *  8. Noise injection
 *  9. Output calibration + wet/dry
 */
class TapeProcessor {
public:
    TapeProcessor() = default;

    void prepare(double sampleRate, int maxBlockSize = 4096);
    void reset();
    void setCalibration(const TapeCalibration& cal);

    // Process a stereo buffer (interleaved or separate)
    void process(float* left, float* right, int numSamples);
    void process(float* interleavedStereo, int numFrames);

    // Single-sample (useful for offline rendering / testing)
    void processSample(double& left, double& right);

    const TapeCalibration& calibration() const noexcept { return cal_; }

private:
    double sampleRate_ = 44100.0;
    TapeCalibration cal_;

    TapeMagnetization magnetization_;
    TapeEqualization  equalization_;
    TapeLossModel     loss_;
    TapeTransport     transport_;
    TapeNoise         noise_;

    Oversampler oversamplerL_;
    Oversampler oversamplerR_;

    EnvelopeFollower envL_, envR_;

    // Temporary oversampled buffers
    std::vector<double> osBufferL_;
    std::vector<double> osBufferR_;

    void processCore(double& left, double& right);
};

} // namespace jh24