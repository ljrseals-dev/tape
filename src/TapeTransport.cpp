#include "jh24/TapeTransport.hpp"
#include <cmath>
#include <algorithm>

namespace jh24 {

void TapeTransport::prepare(double sampleRate, const TapeCalibration& cal) {
    sampleRate_ = sampleRate;
    setCalibration(cal);
    reset();
}

void TapeTransport::reset() {
    delayL_.fill(0.0);
    delayR_.fill(0.0);
    writeIdx_ = 0;
    phaseSlow_ = 0.0;
    phaseScrape_ = 0.0;
    randomWalk_ = 0.0;
}

void TapeTransport::setCalibration(const TapeCalibration& cal) {
    // Convert DIN-weighted % to approximate peak fractional deviation
    // (very rough; real weighting is complex)
    flutterDepth_ = cal.transportFlutter * 0.01 * 1.6;
    scrapeDepth_  = cal.scrapeFlutter  * 0.01 * 2.0;
    scrapeFreqHz_ = (cal.speed == TapeSpeed::IPS15) ? 3800.0 : 5200.0;

    rng_ = Random(cal.randomSeed ^ 0x54524E53u); // "TRNS"
}

double TapeTransport::processChannel(double x, std::array<double, kMaxDelay>& buf,
                                     double mod) noexcept {
    // Write
    buf[writeIdx_] = x;

    // Read with fractional delay
    // Nominal delay ~ a few samples; modulation adds the flutter
    const double baseDelay = 4.0;
    const double delay = baseDelay + mod * sampleRate_ * 0.00015; // scale mod to samples
    const double readPos = static_cast<double>(writeIdx_) - delay;
    int i0 = static_cast<int>(std::floor(readPos));
    const double frac = readPos - i0;

    // Wrap
    while (i0 < 0) i0 += kMaxDelay;
    i0 %= kMaxDelay;
    int i1 = (i0 + 1) % kMaxDelay;

    const double y = buf[i0] * (1.0 - frac) + buf[i1] * frac;

    writeIdx_ = (writeIdx_ + 1) % kMaxDelay;
    return y;
}

void TapeTransport::process(double& left, double& right) {
    // Slow random walk (wow)
    randomWalk_ = randomWalk_ * 0.9997 + rng_.nextUniform() * 0.0004;

    // Periodic component (capstan-related, very mild)
    phaseSlow_ += kTwoPi * 12.0 / sampleRate_; // ~12 Hz
    if (phaseSlow_ > kTwoPi) phaseSlow_ -= kTwoPi;
    const double periodic = std::sin(phaseSlow_) * 0.3;

    // Scrape
    phaseScrape_ += kTwoPi * scrapeFreqHz_ / sampleRate_;
    if (phaseScrape_ > kTwoPi) phaseScrape_ -= kTwoPi;
    const double scrape = std::sin(phaseScrape_) * scrapeDepth_
                        + rng_.nextUniform() * scrapeDepth_ * 0.4;

    const double mod = (randomWalk_ + periodic) * flutterDepth_ + scrape;

    // Slight decorrelation between channels
    const double modL = mod;
    const double modR = mod * 0.92 + rng_.nextUniform() * 0.00002;

    left  = processChannel(left,  delayL_, modL);
    right = processChannel(right, delayR_, modR);
}

} // namespace jh24