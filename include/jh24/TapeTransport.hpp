#pragma once

#include "TapeTypes.hpp"
#include "TapeCalibration.hpp"
#include "Utilities.hpp"

namespace jh24 {

/**
 * Transport model: wow, flutter and scrape flutter.
 *
 * Implemented as a time-varying fractional delay (linear interpolation).
 * Speed variation is composed of:
 *  - slow random walk (wow)
 *  - periodic components at rotational rates
 *  - higher-frequency scrape component
 *
 * Targets are the published JH-24 DIN-weighted figures
 * (0.04 % @ 15 ips, 0.03 % @ 30 ips).
 */
class TapeTransport {
public:
    void prepare(double sampleRate, const TapeCalibration& cal);
    void reset();
    void setCalibration(const TapeCalibration& cal);

    void process(double& left, double& right);

private:
    double sampleRate_ = 44100.0;
    double maxDelaySamples_ = 64.0;

    // Delay lines (circular buffers)
    static constexpr int kMaxDelay = 256;
    std::array<double, kMaxDelay> delayL_{};
    std::array<double, kMaxDelay> delayR_{};
    int writeIdx_ = 0;

    // Modulation state
    double phaseSlow_ = 0.0;
    double phaseScrape_ = 0.0;
    double randomWalk_ = 0.0;
    Random rng_;

    double flutterDepth_ = 0.0004;   // fractional speed deviation
    double scrapeDepth_  = 0.00008;
    double scrapeFreqHz_ = 4200.0;

    double processChannel(double x, std::array<double, kMaxDelay>& buf,
                          double mod) noexcept;
};

} // namespace jh24