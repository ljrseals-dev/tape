#include "jh24/Oversampler.hpp"
#include <algorithm>
#include <cmath>

namespace jh24 {

void Oversampler::prepare(double /*sampleRate*/, int factor) {
    factor_ = std::max(1, std::min(factor, 8));
    if (factor_ == 1) {
        coeffs_.clear();
        upState_.clear();
        downState_.clear();
        return;
    }
    designFilter(factor_);
    const int taps = static_cast<int>(coeffs_.size());
    upState_.assign(taps, 0.0);
    downState_.assign(taps, 0.0);
}

void Oversampler::reset() {
    std::fill(upState_.begin(), upState_.end(), 0.0);
    std::fill(downState_.begin(), downState_.end(), 0.0);
}

void Oversampler::designFilter(int factor) {
    // Simple windowed-sinc half-band style FIR for the chosen factor.
    // Length is chosen for a reasonable stop-band attenuation while remaining cheap.
    const int taps = 32 * factor + 1;
    coeffs_.resize(taps);
    const double fc = 0.45 / factor;   // normalised cutoff
    const int mid = taps / 2;

    for (int i = 0; i < taps; ++i) {
        const double x = static_cast<double>(i - mid);
        double sinc = (x == 0.0) ? 1.0 : std::sin(kTwoPi * fc * x) / (kTwoPi * fc * x);
        // Blackman window
        const double w = 0.42 - 0.5 * std::cos(kTwoPi * i / (taps - 1))
                              + 0.08 * std::cos(2.0 * kTwoPi * i / (taps - 1));
        coeffs_[i] = sinc * w;
    }

    // Normalise
    double sum = 0.0;
    for (double c : coeffs_) sum += c;
    for (double& c : coeffs_) c /= sum;
}

void Oversampler::upsample(double input, double* output) {
    if (factor_ == 1) {
        output[0] = input;
        return;
    }

    // Insert zeros and filter
    const int taps = static_cast<int>(coeffs_.size());
    for (int i = 0; i < factor_; ++i) {
        // Shift state
        for (int j = taps - 1; j > 0; --j)
            upState_[j] = upState_[j - 1];
        upState_[0] = (i == 0) ? input : 0.0;

        double y = 0.0;
        for (int j = 0; j < taps; ++j)
            y += upState_[j] * coeffs_[j];
        output[i] = y * factor_;   // gain compensation for zero-insertion
    }
}

double Oversampler::downsample(const double* input) {
    if (factor_ == 1)
        return input[0];

    const int taps = static_cast<int>(coeffs_.size());
    double acc = 0.0;

    for (int i = 0; i < factor_; ++i) {
        for (int j = taps - 1; j > 0; --j)
            downState_[j] = downState_[j - 1];
        downState_[0] = input[i];

        double y = 0.0;
        for (int j = 0; j < taps; ++j)
            y += downState_[j] * coeffs_[j];
        if (i == 0)   // take the polyphase component corresponding to the original rate
            acc = y;
    }
    return acc;
}

} // namespace jh24