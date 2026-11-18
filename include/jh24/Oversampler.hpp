#pragma once

#include "TapeTypes.hpp"
#include <vector>
#include <cmath>

namespace jh24 {

/**
 * Simple polyphase FIR oversampler (2x / 4x / 8x).
 * Designed for the nonlinear magnetization core.
 * Not the most sophisticated design, but clean, linear-phase and sufficient
 * for high-quality tape modelling when factor <= 8.
 */
class Oversampler {
public:
    Oversampler() = default;

    void prepare(double sampleRate, int factor);
    void reset();

    // Upsample one sample -> factor samples written to output buffer
    void upsample(double input, double* output);

    // Downsample factor samples -> one sample
    double downsample(const double* input);

    int factor() const noexcept { return factor_; }

private:
    int factor_ = 1;
    std::vector<double> upState_;
    std::vector<double> downState_;
    std::vector<double> coeffs_;   // half-band-ish FIR

    void designFilter(int factor);
};

} // namespace jh24