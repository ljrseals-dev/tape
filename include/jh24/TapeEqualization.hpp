#pragma once

#include "TapeTypes.hpp"
#include "TapeCalibration.hpp"
#include "Utilities.hpp"

namespace jh24 {

/**
 * Record and reproduce equalization matching NAB / IEC1 / AES standards
 * as implemented on JH-series machines, plus phase-compensation character.
 *
 * Two independent chains (record EQ and reproduce EQ) are maintained.
 */
class TapeEqualization {
public:
    void prepare(double sampleRate, const TapeCalibration& cal);
    void reset();
    void setCalibration(const TapeCalibration& cal);

    // Record-side pre-emphasis
    void processRecord(double& left, double& right);

    // Reproduce-side equalization
    void processReproduce(double& left, double& right);

private:
    double sampleRate_ = 44100.0;
    TapeEQ eq_ = TapeEQ::NAB;
    TapeSpeed speed_ = TapeSpeed::IPS15;

    // Record EQ filters (per channel)
    Biquad recHighShelfL_, recHighShelfR_;
    Biquad recLowShelfL_,  recLowShelfR_;

    // Reproduce EQ filters
    Biquad reproHighL_, reproHighR_;
    Biquad reproLowL_,  reproLowR_;

    // Head-bump / contour (applied on reproduce side)
    Biquad bumpL_, bumpR_;

    void designRecordEQ();
    void designReproduceEQ();
    void designHeadBump(const TapeCalibration& cal);
};

} // namespace jh24