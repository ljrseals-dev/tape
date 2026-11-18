#include "jh24/Utilities.hpp"
#include <cmath>

namespace jh24 {

void Biquad::setPeakingEQ(double sampleRate, double freq, double Q, double gainDb) noexcept {
    const double A  = std::pow(10.0, gainDb / 40.0);
    const double w0 = kTwoPi * freq / sampleRate;
    const double alpha = std::sin(w0) / (2.0 * Q);
    const double cosw0 = std::cos(w0);

    const double b0 = 1.0 + alpha * A;
    const double b1 = -2.0 * cosw0;
    const double b2 = 1.0 - alpha * A;
    const double a0 = 1.0 + alpha / A;
    const double a1 = -2.0 * cosw0;
    const double a2 = 1.0 - alpha / A;

    setCoefficients(b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0);
}

void Biquad::setLowShelf(double sampleRate, double freq, double gainDb, double S) noexcept {
    const double A  = std::pow(10.0, gainDb / 40.0);
    const double w0 = kTwoPi * freq / sampleRate;
    const double cosw0 = std::cos(w0);
    const double sinw0 = std::sin(w0);
    const double alpha = sinw0 / 2.0 * std::sqrt((A + 1.0 / A) * (1.0 / S - 1.0) + 2.0);
    const double twoSqrtAAlpha = 2.0 * std::sqrt(A) * alpha;

    const double b0 =    A * ((A + 1.0) - (A - 1.0) * cosw0 + twoSqrtAAlpha);
    const double b1 =  2.0 * A * ((A - 1.0) - (A + 1.0) * cosw0);
    const double b2 =    A * ((A + 1.0) - (A - 1.0) * cosw0 - twoSqrtAAlpha);
    const double a0 =        (A + 1.0) + (A - 1.0) * cosw0 + twoSqrtAAlpha;
    const double a1 = -2.0 *     ((A - 1.0) + (A + 1.0) * cosw0);
    const double a2 =        (A + 1.0) + (A - 1.0) * cosw0 - twoSqrtAAlpha;

    setCoefficients(b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0);
}

void Biquad::setHighShelf(double sampleRate, double freq, double gainDb, double S) noexcept {
    const double A  = std::pow(10.0, gainDb / 40.0);
    const double w0 = kTwoPi * freq / sampleRate;
    const double cosw0 = std::cos(w0);
    const double sinw0 = std::sin(w0);
    const double alpha = sinw0 / 2.0 * std::sqrt((A + 1.0 / A) * (1.0 / S - 1.0) + 2.0);
    const double twoSqrtAAlpha = 2.0 * std::sqrt(A) * alpha;

    const double b0 =    A * ((A + 1.0) + (A - 1.0) * cosw0 + twoSqrtAAlpha);
    const double b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosw0);
    const double b2 =    A * ((A + 1.0) + (A - 1.0) * cosw0 - twoSqrtAAlpha);
    const double a0 =        (A + 1.0) - (A - 1.0) * cosw0 + twoSqrtAAlpha;
    const double a1 =  2.0 *     ((A - 1.0) - (A + 1.0) * cosw0);
    const double a2 =        (A + 1.0) - (A - 1.0) * cosw0 - twoSqrtAAlpha;

    setCoefficients(b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0);
}

void Biquad::setLowPass(double sampleRate, double freq, double Q) noexcept {
    const double w0 = kTwoPi * freq / sampleRate;
    const double alpha = std::sin(w0) / (2.0 * Q);
    const double cosw0 = std::cos(w0);

    const double b0 = (1.0 - cosw0) * 0.5;
    const double b1 = 1.0 - cosw0;
    const double b2 = (1.0 - cosw0) * 0.5;
    const double a0 = 1.0 + alpha;
    const double a1 = -2.0 * cosw0;
    const double a2 = 1.0 - alpha;

    setCoefficients(b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0);
}

void Biquad::setHighPass(double sampleRate, double freq, double Q) noexcept {
    const double w0 = kTwoPi * freq / sampleRate;
    const double alpha = std::sin(w0) / (2.0 * Q);
    const double cosw0 = std::cos(w0);

    const double b0 =  (1.0 + cosw0) * 0.5;
    const double b1 = -(1.0 + cosw0);
    const double b2 =  (1.0 + cosw0) * 0.5;
    const double a0 =   1.0 + alpha;
    const double a1 =  -2.0 * cosw0;
    const double a2 =   1.0 - alpha;

    setCoefficients(b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0);
}

} // namespace jh24