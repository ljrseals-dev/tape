#pragma once

#include <cstdint>
#include <cmath>
#include <array>
#include <string>
#include <vector>

namespace jh24 {

// ---------------------------------------------------------------------------
// Fundamental enums
// ---------------------------------------------------------------------------

enum class TapeSpeed {
    IPS15 = 0,
    IPS30 = 1
};

enum class TapeEQ {
    NAB  = 0,   // 15 ips classic American
    IEC1 = 1,   // 15 ips European / CCIR
    AES  = 2    // 30 ips
};

enum class TapeFormula {
    Scotch250   = 0,
    Ampex456    = 1,
    AgfaPEM468  = 2,
    Custom      = 3
};

enum class HeadCondition {
    Fresh       = 0,
    Maintained  = 1,   // default real-world used
    Worn        = 2
};

enum class MagnetizationMode {
    Rational    = 0,   // production default (fitted odd rational)
    TapeHack    = 1,   // legacy polynomial series
    JilesAtherton = 2  // higher-cost physical reference
};

// ---------------------------------------------------------------------------
// Utility constants
// ---------------------------------------------------------------------------

constexpr double kPi          = 3.14159265358979323846;
constexpr double kTwoPi       = 6.28318530717958647692;
constexpr double kReference185 = 185.0;   // nWb/m
constexpr double kReference250 = 250.0;
constexpr double kReference320 = 320.0;
constexpr double kReference510 = 510.0;   // JH-24 published spec reference

inline double dbToLinear(double db) noexcept {
    return std::pow(10.0, db * 0.05);
}

inline double linearToDb(double lin) noexcept {
    return 20.0 * std::log10(std::max(lin, 1.0e-20));
}

inline double clamp(double x, double lo, double hi) noexcept {
    return x < lo ? lo : (x > hi ? hi : x);
}

// Soft clip helper used in several places
inline double softClip(double x, double limit = 2.3) noexcept {
    if (x >  limit) return  limit;
    if (x < -limit) return -limit;
    return x;
}

} // namespace jh24