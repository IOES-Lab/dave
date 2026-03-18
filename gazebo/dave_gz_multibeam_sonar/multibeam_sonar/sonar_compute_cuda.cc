#include "sonar_compute_cuda.hh"

#include <chrono>
#include <cmath>

#ifdef DAVE_HAS_CUDA_BACKEND
#include <cuda_runtime_api.h>
#include "sonar_calculation_cuda.cuh"
#endif

namespace gz
{
namespace sensors
{

CudaComputeBackend::~CudaComputeBackend()
{
#ifdef DAVE_HAS_CUDA_BACKEND
  if (this->initialized)
  {
    NpsGazeboSonar::free_cuda_memory();
  }
#endif
}

const char * CudaComputeBackend::Name() const { return "cuda"; }

bool CudaComputeBackend::Initialize(const SonarComputeInput &)
{
#ifdef DAVE_HAS_CUDA_BACKEND
  int deviceCount = 0;
  const cudaError_t err = cudaGetDeviceCount(&deviceCount);
  if (err != cudaSuccess || deviceCount <= 0)
  {
    return false;
  }

  this->initialized = true;
  return true;
#else
  return false;
#endif
}

bool CudaComputeBackend::Compute(const SonarComputeInput & input, SonarComputeOutput & output)
{
#ifdef DAVE_HAS_CUDA_BACKEND
  if (!this->initialized)
  {
    return false;
  }

  if (
    !input.depthImage || !input.normalImage || !input.reflectivityImage || !input.window ||
    !input.beamCorrector || input.elevation_angles.empty())
  {
    return false;
  }

  auto start = std::chrono::high_resolution_clock::now();

  const double hPixelSize = input.hFOV / static_cast<double>(input.nBeams - 1);
  const double vPixelSize = input.vFOV / static_cast<double>(input.nRays - 1);

  const NpsGazeboSonar::CArray2D beams = NpsGazeboSonar::sonar_calculation_wrapper(
    *input.depthImage, *input.normalImage, hPixelSize, vPixelSize, input.hFOV, input.vFOV,
    hPixelSize, input.vFOV / 180.0 * M_PI, hPixelSize,
    const_cast<float *>(input.elevation_angles.data()), vPixelSize * (input.raySkips + 1),
    input.soundSpeed, input.maxDistance, input.sourceLevel, input.nBeams, input.nRays,
    input.raySkips, input.sonarFreq, input.bandwidth, input.nFreq, *input.reflectivityImage,
    input.attenuation, const_cast<float *>(input.window), input.beamCorrector,
    input.beamCorrectorSum, false, input.blazingFlag);

  output.nBeams = input.nBeams;
  output.nFreq = input.nFreq;
  output.beamSpectrum.assign(
    static_cast<size_t>(input.nBeams) * static_cast<size_t>(input.nFreq),
    std::complex<float>(0.0f, 0.0f));

  for (int beam = 0; beam < input.nBeams; ++beam)
  {
    for (int freq = 0; freq < input.nFreq; ++freq)
    {
      output.At(beam, freq) = beams[beam][freq];
    }
  }

  auto stop = std::chrono::high_resolution_clock::now();
  output.computeMicros =
    std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
  return true;
#else
  static_cast<void>(input);
  static_cast<void>(output);
  return false;
#endif
}

}  // namespace sensors
}  // namespace gz
