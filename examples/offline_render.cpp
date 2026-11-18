/**
 * Offline render helper – reads a raw float32 stereo file, processes it
 * through the JH-24 model with a chosen calibration, writes the result.
 *
 * Usage:
 *   jh24_offline input.raw output.raw [mode]
 *   mode = classic | hot | highheadroom   (default classic)
 */

#include "jh24/jh24.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>

using namespace jh24;

static bool readRaw(const std::string& path, std::vector<float>& data) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) return false;
    const auto size = in.tellg();
    in.seekg(0);
    data.resize(static_cast<size_t>(size) / sizeof(float));
    in.read(reinterpret_cast<char*>(data.data()), size);
    return true;
}

static bool writeRaw(const std::string& path, const std::vector<float>& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<const char*>(data.data()),
              static_cast<std::streamsize>(data.size() * sizeof(float)));
    return true;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " input.raw output.raw [classic|hot|highheadroom]\n";
        return 1;
    }

    const std::string inPath  = argv[1];
    const std::string outPath = argv[2];
    std::string mode = (argc >= 4) ? argv[3] : "classic";

    std::vector<float> buffer;
    if (!readRaw(inPath, buffer) || buffer.size() < 2) {
        std::cerr << "Failed to read " << inPath << "\n";
        return 1;
    }

    const int numFrames = static_cast<int>(buffer.size() / 2);
    const double sampleRate = 48000.0; // assumed

    TapeCalibration cal;
    if (mode == "hot")
        cal.setHotDriven15();
    else if (mode == "highheadroom")
        cal.setHighHeadroom30();
    else
        cal.setClassicAmerican15();

    cal.oversampleFactor = 4;

    TapeProcessor tape;
    tape.prepare(sampleRate);
    tape.setCalibration(cal);

    std::cout << "JH-24 offline render – mode: " << mode
              << ", frames: " << numFrames << "\n";

    tape.process(buffer.data(), numFrames);

    if (!writeRaw(outPath, buffer)) {
        std::cerr << "Failed to write " << outPath << "\n";
        return 1;
    }

    std::cout << "Wrote " << outPath << "\n";
    return 0;
}