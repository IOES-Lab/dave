#pragma once

#include <complex>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <opencv2/core.hpp>

namespace gz
{
namespace sensors
{

struct SonarComputeInput
{
  const cv::Mat * depthImage{nullptr};
  const cv::Mat * normalImage{nullptr};
  const cv::Mat * reflectivityImage{nullptr};

  int nBeams{0};
  int nRays{0};
  int nFreq{0};
  int raySkips{1};

  double hFOV{0.0};
  double vFOV{0.0};
  double maxDistance{0.0};
  double soundSpeed{1500.0};
  double attenuation{0.0};

  float sensorGain{0.02f};
  bool blazingFlag{false};

  const float * window{nullptr};
  const float * rangeVector{nullptr};
  float ** beamCorrector{nullptr};
  float beamCorrectorSum{0.0f};

  double sourceLevel{220.0};
  double bandwidth{29.5e6};
  double sonarFreq{900e3};
  std::vector<float> elevation_angles;

  uint64_t frameIndex{0};
  uint64_t seed{0};
};

struct SonarComputeOutput
{
  int nBeams{0};
  int nFreq{0};
  std::vector<std::complex<float>> beamSpectrum;
  double computeMicros{0.0};

  std::complex<float> & At(int beam, int freq)
  {
    return this->beamSpectrum[static_cast<size_t>(beam) * this->nFreq + freq];
  }

  const std::complex<float> & At(int beam, int freq) const
  {
    return this->beamSpectrum[static_cast<size_t>(beam) * this->nFreq + freq];
  }
};

class ComputeBackend
{
public:
  virtual ~ComputeBackend() = default;

  virtual const char * Name() const = 0;

  virtual bool Initialize(const SonarComputeInput & prototype) = 0;

  virtual bool Compute(const SonarComputeInput & input, SonarComputeOutput & output) = 0;
};

std::unique_ptr<ComputeBackend> CreateComputeBackend(const std::string & requestedBackend);

}  // namespace sensors
}  // namespace gz
