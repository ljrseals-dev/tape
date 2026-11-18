#include "jh24/TapeMagnetization.hpp"
#include <cmath>
#include <algorithm>

namespace jh24 {

void TapeMagnetization::prepare(double /*sampleRate*/, const TapeCalibration& cal) {
    setCalibration(cal);
    reset();
}

void TapeMagnetization::reset() {
    jaL_ = JAState{};
    jaR_ = JAState{};
}

void TapeMagnetization::setCalibration(const TapeCalibration& cal) {
    mode_ = cal.magnetizationMode;
    // Drive from record level (dB relative to reference)
    drive_ = dbToLinear(cal.recordLevelDb);
    // Mild additional drive scaling for the rational core so that
    // 0 dB operating point sits in a musically useful region of the curve.
    drive_ *= 1.4;
    outputScale_ = 1.0 / 1.4;
    updateCoefficients(cal);
}

void TapeMagnetization::updateCoefficients(const TapeCalibration& cal) {
    // Slightly different knee character per formulation
    switch (cal.formulation) {
        case TapeFormula::Scotch250:
            a3_ = -0.145; a5_ = 0.011; b2_ = 0.17; b4_ = 0.022;
            break;
        case TapeFormula::Ampex456:
            a3_ = -0.155; a5_ = 0.013; b2_ = 0.185; b4_ = 0.026;
            break;
        case TapeFormula::AgfaPEM468:
            a3_ = -0.135; a5_ = 0.010; b2_ = 0.16; b4_ = 0.020;
            break;
        default:
            a3_ = -0.15; a5_ = 0.012; b2_ = 0.18; b4_ = 0.025;
            break;
    }

    // Head condition gently softens the knee when worn
    if (cal.headCondition == HeadCondition::Worn) {
        a3_ *= 1.08;
        b2_ *= 1.06;
    }
}

double TapeMagnetization::processRational(double x) const noexcept {
    // Soft pre-limit to protect the polynomial region
    x = softClip(x * drive_, 2.8);

    const double x2 = x * x;
    const double x3 = x * x2;
    const double x4 = x2 * x2;
    const double x5 = x3 * x2;

    const double num = x + a3_ * x3 + a5_ * x5;
    const double den = 1.0 + b2_ * x2 + b4_ * x4;
    return (num / den) * outputScale_;
}

double TapeMagnetization::processTapeHack(double x) const noexcept {
    // Legacy series supplied by the user (TapeHack core)
    x *= drive_;
    x = softClip(x, 2.3);

    double addtwo = x * x;
    double empower = x * addtwo;
    x -= empower / 6.0;
    empower *= addtwo; x += empower / 69.0;
    empower *= addtwo; x -= empower / 2530.08;
    empower *= addtwo; x += empower / 224985.6;
    empower *= addtwo; x -= empower / 9979200.0;

    return x * outputScale_;
}

namespace {
inline double coth(double x) noexcept {
    if (std::abs(x) < 1e-8) return 1.0 / x; // avoid div-by-zero, though Langevin handles 0
    return std::cosh(x) / std::sinh(x);
}
} // namespace

double TapeMagnetization::processJA(double x, JAState& state) noexcept {
    // Extremely simplified discrete Jiles-Atherton step.
    // This is a research/reference path only; it is not a full bias-frequency
    // simulation. Parameters are scaled into a convenient numerical range.
    constexpr double scale = 1.0e-4;   // map audio to field-like units
    const double H = x * drive_ / scale;

    const double He = H + ja_alpha_ * state.M;
    const double arg = He / ja_a_;
    const double Man = ja_Ms_ * (coth(arg) - 1.0 / arg);

    const double dH = H; // crude; proper implementation needs previous H
    const double delta = (dH >= 0.0) ? 1.0 : -1.0;
    const double deltaM = ((delta > 0 && (Man - state.Mirr) > 0) ||
                           (delta < 0 && (Man - state.Mirr) < 0)) ? 1.0 : 0.0;

    const double denom = delta * ja_k_ - ja_alpha_ * (Man - state.Mirr);
    double dMirr = 0.0;
    if (std::abs(denom) > 1.0e-12)
        dMirr = (Man - state.Mirr) / denom * deltaM;

    state.Mirr += dMirr * 0.15; // step size fudge for audio-rate stability
    state.M = ja_c_ * Man + (1.0 - ja_c_) * state.Mirr;

    return (state.M * scale) * outputScale_;
}

double TapeMagnetization::process(double x) {
    switch (mode_) {
        case MagnetizationMode::TapeHack:
            return processTapeHack(x);
        case MagnetizationMode::JilesAtherton:
            return processJA(x, jaL_);
        case MagnetizationMode::Rational:
        default:
            return processRational(x);
    }
}

void TapeMagnetization::processStereo(double& left, double& right) {
    if (mode_ == MagnetizationMode::JilesAtherton) {
        left  = processJA(left,  jaL_);
        right = processJA(right, jaR_);
    } else {
        left  = process(left);
        right = process(right);
    }
}

} // namespace jh24