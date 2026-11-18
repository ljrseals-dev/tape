#pragma once

/**
 * JH-24 Tape Model – definitive C++17 reference implementation
 *
 * High-fidelity mathematical and perceptual model of a well-maintained
 * but real-world-used MCI JH-24 (or equivalent 2-inch 24-track) tape path
 * operating at 15 ips or 30 ips with period oxide formulations.
 *
 * Based on documented research, service manuals and contemporary literature
 * 1975–1985 (Wallace, Westmijze, Jiles-Atherton, MCI JH-series specifications,
 * bias optimisation data, NAB/IEC/AES equalisation, head-bump measurements, etc.).
 */

#include "TapeTypes.hpp"
#include "TapeCalibration.hpp"
#include "TapeMagnetization.hpp"
#include "TapeEqualization.hpp"
#include "TapeLossModel.hpp"
#include "TapeTransport.hpp"
#include "TapeNoise.hpp"
#include "TapeProcessor.hpp"
#include "Oversampler.hpp"
#include "Utilities.hpp"

namespace jh24 {

inline const char* version() noexcept {
    return "1.0.0";
}

inline const char* description() noexcept {
    return "MCI JH-24 analog tape-path reference model (C++17)";
}

} // namespace jh24