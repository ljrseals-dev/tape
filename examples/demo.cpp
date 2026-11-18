/**
 * Simple real-time style demo of the JH-24 tape model.
 * Generates a test signal, processes it, and writes a short WAV-like dump
 * (raw float stereo) plus console diagnostics.
 */

#include "jh24/jh24.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <iomanip>

using namespace jh24;

static void writeRawFloatStereo(const std::string& path,
                                const std::vector<float>& interleaved,
                                double sampleRate) {
    // Minimal header-less float dump for quick inspection; a real app would write WAV.
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(interleaved.data()),
              static_cast<std::streamsize>(interleaved.size() * sizeof(float)));
    std::cout << "Wrote " << path << " (" << interleaved.size() / 2
              << " frames @ " << sampleRate << " Hz, float32 stereo)\n";
}

int main() {
    std::cout << "JH-24 Tape Model " << version() << "\n"
              << description() << "\n\n";

    const double sampleRate = 48000.0;
    const int numFrames = static_cast<int>(sampleRate * 3.0); // 3 seconds

    TapeCalibration cal;
    cal.setClassicAmerican15();
    cal.recordLevelDb = 3.0;          // moderately hot
    cal.oversampleFactor = 4;
    cal.wet = 1.0;

    TapeProcessor tape;
    tape.prepare(sampleRate);
    tape.setCalibration(cal);

    std::vector<float> buffer(static_cast<size_t>(numFrames * 2));

    // Generate a rich test signal: 110 Hz saw + 440 Hz sine + noise burst
    for (int i = 0; i < numFrames; ++i) {
        const double t = i / sampleRate;
        double s = 0.0;
        // saw
        s += 0.35 * (2.0 * std::fmod(t * 110.0, 1.0) - 1.0);
        // sine
        s += 0.25 * std::sin(kTwoPi * 440.0 * t);
        // gentle envelope
        const double env = 0.5 + 0.5 * std::sin(kTwoPi * 0.4 * t);
        s *= env;

        buffer[i * 2]     = static_cast<float>(s);
        buffer[i * 2 + 1] = static_cast<float>(s * 0.98); // slight stereo difference
    }

    std::cout << "Processing " << numFrames << " frames...\n";
    tape.process(buffer.data(), numFrames);

    // Simple peak / RMS report
    double peak = 0.0, sumSq = 0.0;
    for (float v : buffer) {
        peak = std::max(peak, static_cast<double>(std::abs(v)));
        sumSq += v * v;
    }
    const double rms = std::sqrt(sumSq / buffer.size());

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Peak : " << peak << "\n"
              << "RMS  : " << rms  << "\n"
              << "Crest: " << (rms > 1e-9 ? peak / rms : 0.0) << "\n";

    writeRawFloatStereo("jh24_demo_out.raw", buffer, sampleRate);

    std::cout << "\nDone. Load the .raw file as 32-bit float stereo @ 48 kHz to audition.\n";
    return 0;
}