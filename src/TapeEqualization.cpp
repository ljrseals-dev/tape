#include "jh24/TapeEqualization.hpp"
#include <cmath>

namespace jh24 {

void TapeEqualization::prepare(double sampleRate, const TapeCalibration& cal) {
    sampleRate_ = sampleRate;
    setCalibration(cal);
    reset();
}

void TapeEqualization::reset() {
    recHighShelfL_.reset(); recHighShelfR_.reset();
    recLowShelfL_.reset();  recLowShelfR_.reset();
    reproHighL_.reset();    reproHighR_.reset();
    reproLowL_.reset();     reproLowR_.reset();
    bumpL_.reset();         bumpR_.reset();
}

void TapeEqualization::setCalibration(const TapeCalibration& cal) {
    eq_ = cal.equalization;
    speed_ = cal.speed;
    designRecordEQ();
    designReproduceEQ();
    designHeadBump(cal);
}

void TapeEqualization::designRecordEQ() {
    // Record-side pre-emphasis is the complement of the reproduce curve.
    // For simplicity we implement mild shelves that approximate the
    // required pre-emphasis for each standard.
    double hfShelfDb = 0.0;
    double hfFreq = 5000.0;
    double lfShelfDb = 0.0;
    double lfFreq = 80.0;

    switch (eq_) {
        case TapeEQ::NAB:
            // NAB requires more HF boost on record to counteract the 50 µs playback boost
            hfShelfDb = 2.5;
            hfFreq = 4000.0;
            lfShelfDb = -1.5;   // mild LF cut on record (NAB has LF boost on play)
            lfFreq = 60.0;
            break;
        case TapeEQ::IEC1:
            hfShelfDb = 1.5;
            hfFreq = 4500.0;
            lfShelfDb = 0.0;
            break;
        case TapeEQ::AES:
            hfShelfDb = 1.0;
            hfFreq = 8000.0;
            lfShelfDb = 0.0;
            break;
    }

    recHighShelfL_.setHighShelf(sampleRate_, hfFreq, hfShelfDb);
    recHighShelfR_.setHighShelf(sampleRate_, hfFreq, hfShelfDb);
    recLowShelfL_.setLowShelf(sampleRate_, lfFreq, lfShelfDb);
    recLowShelfR_.setLowShelf(sampleRate_, lfFreq, lfShelfDb);
}

void TapeEqualization::designReproduceEQ() {
    // Reproduce EQ realises the standard time-constant curves.
    // We approximate the required playback boost with shelves / peaking.
    double hfBoostDb = 0.0;
    double hfFreq = 3183.0;
    double lfBoostDb = 0.0;
    double lfFreq = 50.0;

    switch (eq_) {
        case TapeEQ::NAB:
            // 50 µs → ~3183 Hz, 3180 µs → ~50 Hz
            hfBoostDb = 3.5;
            hfFreq = 3200.0;
            lfBoostDb = 2.5;
            lfFreq = 55.0;
            break;
        case TapeEQ::IEC1:
            // 35 µs → ~4547 Hz, no LF time constant
            hfBoostDb = 2.8;
            hfFreq = 4500.0;
            lfBoostDb = 0.0;
            break;
        case TapeEQ::AES:
            // 17.5 µs → ~9095 Hz
            hfBoostDb = 2.0;
            hfFreq = 9000.0;
            lfBoostDb = 0.0;
            break;
    }

    reproHighL_.setHighShelf(sampleRate_, hfFreq, hfBoostDb);
    reproHighR_.setHighShelf(sampleRate_, hfFreq, hfBoostDb);
    reproLowL_.setLowShelf(sampleRate_, lfFreq, lfBoostDb);
    reproLowR_.setLowShelf(sampleRate_, lfFreq, lfBoostDb);
}

void TapeEqualization::designHeadBump(const TapeCalibration& cal) {
    double freq = cal.headBumpFrequencyHz;
    // Scale bump frequency with speed (roughly inverse)
    if (cal.speed == TapeSpeed::IPS30)
        freq *= 0.65;

    const double gain = cal.headBumpGainDb;
    const double Q = cal.headBumpQ;

    bumpL_.setPeakingEQ(sampleRate_, freq, Q, gain);
    bumpR_.setPeakingEQ(sampleRate_, freq, Q, gain);
}

void TapeEqualization::processRecord(double& left, double& right) {
    left  = recLowShelfL_.process(recHighShelfL_.process(left));
    right = recLowShelfR_.process(recHighShelfR_.process(right));
}

void TapeEqualization::processReproduce(double& left, double& right) {
    left  = bumpL_.process(reproLowL_.process(reproHighL_.process(left)));
    right = bumpR_.process(reproLowR_.process(reproHighR_.process(right)));
}

} // namespace jh24