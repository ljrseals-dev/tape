#pragma once

#include "TapeTypes.hpp"
#include <random>
#include <cstdint>

namespace jh24 {

// ---------------------------------------------------------------------------
// Simple one-pole low-pass (first-order IIR)
// ---------------------------------------------------------------------------
class OnePoleLP {
public:
    void setCutoff(double sampleRate, double cutoffHz) noexcept {
        const double x = std::exp(-kTwoPi * cutoffHz / sampleRate);
        a_ = x;
        b_ = 1.0 - x;
    }

    void reset(double value = 0.0) noexcept { z_ = value; }

    double process(double x) noexcept {
        z_ = z_ * a_ + x * b_;
        return z_;
    }

    double state() const noexcept { return z_; }

private:
    double a_ = 0.0;
    double b_ = 1.0;
    double z_ = 0.0;
};

// ---------------------------------------------------------------------------
// Biquad filter (Direct Form I)
// ---------------------------------------------------------------------------
class Biquad {
public:
    void setCoefficients(double b0, double b1, double b2,
                         double a1, double a2) noexcept {
        b0_ = b0; b1_ = b1; b2_ = b2;
        a1_ = a1; a2_ = a2;
    }

    void reset() noexcept {
        x1_ = x2_ = y1_ = y2_ = 0.0;
    }

    double process(double x) noexcept {
        const double y = b0_ * x + b1_ * x1_ + b2_ * x2_
                       - a1_ * y1_ - a2_ * y2_;
        x2_ = x1_; x1_ = x;
        y2_ = y1_; y1_ = y;
        return y;
    }

    // Design helpers
    void setPeakingEQ(double sampleRate, double freq, double Q, double gainDb) noexcept;
    void setLowShelf (double sampleRate, double freq, double gainDb, double S = 1.0) noexcept;
    void setHighShelf(double sampleRate, double freq, double gainDb, double S = 1.0) noexcept;
    void setLowPass  (double sampleRate, double freq, double Q) noexcept;
    void setHighPass (double sampleRate, double freq, double Q) noexcept;

private:
    double b0_ = 1.0, b1_ = 0.0, b2_ = 0.0;
    double a1_ = 0.0, a2_ = 0.0;
    double x1_ = 0.0, x2_ = 0.0;
    double y1_ = 0.0, y2_ = 0.0;
};

// ---------------------------------------------------------------------------
// Deterministic PRNG (xoshiro128+) – no global state
// ---------------------------------------------------------------------------
class Random {
public:
    explicit Random(uint32_t seed = 0x4A483234) {
        seed = seed ? seed : 0x4A483234u;
        s_[0] = seed;
        s_[1] = seed * 0x9E3779B9u;
        s_[2] = seed * 0x85EBCA6Bu;
        s_[3] = seed * 0xC2B2AE35u;
        // warm up
        for (int i = 0; i < 8; ++i) nextU32();
    }

    uint32_t nextU32() noexcept {
        const uint32_t result = s_[0] + s_[3];
        const uint32_t t = s_[1] << 9;
        s_[2] ^= s_[0];
        s_[3] ^= s_[1];
        s_[1] ^= s_[2];
        s_[0] ^= s_[3];
        s_[2] ^= t;
        s_[3] = rotl(s_[3], 11);
        return result;
    }

    // Uniform [-1, 1)
    double nextUniform() noexcept {
        return (static_cast<int32_t>(nextU32()) * (1.0 / 2147483648.0));
    }

    // Approximate white noise, zero mean, unit variance-ish
    double nextGaussianApprox() noexcept {
        // sum of 4 uniforms is a reasonable fast approximation
        return 0.5 * (nextUniform() + nextUniform() + nextUniform() + nextUniform());
    }

private:
    static uint32_t rotl(uint32_t x, int k) noexcept {
        return (x << k) | (x >> (32 - k));
    }
    uint32_t s_[4];
};

// ---------------------------------------------------------------------------
// Simple envelope follower (rectified + one-pole)
// ---------------------------------------------------------------------------
class EnvelopeFollower {
public:
    void setAttackRelease(double sampleRate, double attackMs, double releaseMs) noexcept {
        attackCoeff_  = std::exp(-1.0 / (attackMs  * 0.001 * sampleRate));
        releaseCoeff_ = std::exp(-1.0 / (releaseMs * 0.001 * sampleRate));
    }

    void reset() noexcept { env_ = 0.0; }

    double process(double x) noexcept {
        const double absX = std::abs(x);
        if (absX > env_)
            env_ = attackCoeff_ * env_ + (1.0 - attackCoeff_) * absX;
        else
            env_ = releaseCoeff_ * env_ + (1.0 - releaseCoeff_) * absX;
        return env_;
    }

    double value() const noexcept { return env_; }

private:
    double env_ = 0.0;
    double attackCoeff_  = 0.0;
    double releaseCoeff_ = 0.0;
};

} // namespace jh24