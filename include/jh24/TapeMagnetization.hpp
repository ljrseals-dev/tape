#pragma once

#include "TapeTypes.hpp"
#include "TapeCalibration.hpp"
#include "Utilities.hpp"

namespace jh24 {

/**
 * Magnetic magnetization stage.
 *
 * Three selectable cores:
 *  1. Rational (production default) – fitted odd rational function
 *  2. TapeHack – legacy polynomial series supplied by the user
 *  3. Jiles-Atherton – simplified differential hysteresis (research mode)
 *
 * The stage expects a signal already scaled relative to the chosen
 * reference fluxivity and record level.
 */
class TapeMagnetization {
public:
    void prepare(double sampleRate, const TapeCalibration& cal);
    void reset();
    void setCalibration(const TapeCalibration& cal);

    // Process one sample (mono). Returns magnetized sample.
    double process(double x);

    // Stereo convenience
    void processStereo(double& left, double& right);

private:
    MagnetizationMode mode_ = MagnetizationMode::Rational;
    double drive_ = 1.0;           // linear drive from recordLevelDb
    double outputScale_ = 1.0;

    // Rational coefficients (normalized so F'(0) = 1)
    // F(x) = (x + a3 x^3 + a5 x^5) / (1 + b2 x^2 + b4 x^4)
    double a3_ = -0.15;
    double a5_ =  0.012;
    double b2_ =  0.18;
    double b4_ =  0.025;

    // Jiles-Atherton state (per channel)
    struct JAState {
        double M = 0.0;
        double Mirr = 0.0;
    };
    JAState jaL_, jaR_;
    // JA parameters (typical ferric oxide starting values)
    double ja_Ms_ = 3.5e5;
    double ja_a_  = 22000.0;
    double ja_k_  = 27000.0;
    double ja_c_  = 0.35;
    double ja_alpha_ = 5.0e-5;

    // Internal helpers
    double processRational(double x) const noexcept;
    double processTapeHack(double x) const noexcept;
    double processJA(double x, JAState& state) noexcept;

    void updateCoefficients(const TapeCalibration& cal);
};

} // namespace jh24