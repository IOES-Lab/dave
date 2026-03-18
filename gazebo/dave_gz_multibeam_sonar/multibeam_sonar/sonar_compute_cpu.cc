#include "sonar_compute_backend.hh"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

// NOTE: This file serves two responsibilities -- it implements the CPU compute
// backend (CpuComputeBackend) AND hosts the CreateComputeBackend() factory
// that selects between CPU and WGPU at runtime. Because the factory lives here,
// this file must include sonar_compute_wgpu.hh, which creates a hard compile-time
// dependency on the WGPU backend even when only CPU support is needed.
//
// TODO: Move CreateComputeBackend() into its own file (e.g. sonar_compute_factory.cc)
// so that the CPU and WGPU backends are fully decoupled and the factory is the
// only translation unit that needs to know about both.
#include "sonar_compute_wgpu.hh"
#include "sonar_compute_cuda.hh"

namespace gz
{
namespace sensors
{
namespace
{

class CpuComputeBackend : public ComputeBackend
{
public:
  const char * Name() const override { return "cpu"; }

  bool Initialize(const SonarComputeInput &) override { return true; }

  bool Compute(const SonarComputeInput & input, SonarComputeOutput & output) override
  {
    if (!input.depthImage || !input.normalImage || input.nBeams <= 0 || input.nFreq <= 0)
    {
      return false;
    }

    auto start = std::chrono::high_resolution_clock::now();

    output.nBeams = input.nBeams;
    output.nFreq = input.nFreq;
    output.beamSpectrum.assign(
      static_cast<size_t>(input.nBeams) * static_cast<size_t>(input.nFreq),
      std::complex<float>(0.0f, 0.0f));

    const cv::Mat & depth = *input.depthImage;
    const cv::Mat & normal = *input.normalImage;

    if (depth.empty() || depth.type() != CV_32FC1 || normal.empty() || normal.type() != CV_32FC3)
    {
      return false;
    }

    const int rows = depth.rows;
    const int cols = depth.cols;
    const float maxDistance = static_cast<float>(std::max(1e-6, input.maxDistance));
    const float attenuation = static_cast<float>(input.attenuation);

    const float source_term =
      std::sqrt(std::pow(10.0f, static_cast<float>(input.sourceLevel) / 10.0f)) * 1e-6f;
    const float h_pixel = static_cast<float>(input.hFOV) / std::max(1, input.nBeams - 1);
    const float v_pixel = static_cast<float>(input.vFOV) / std::max(1, input.nRays - 1);
    const float area_scaler = h_pixel * v_pixel * static_cast<float>(input.raySkips + 1);

    for (int r = 0; r < rows; ++r)
    {
      const float * depthRow = depth.ptr<float>(r);
      const cv::Vec3f * normalRow = normal.ptr<cv::Vec3f>(r);
      for (int c = 0; c < cols; ++c)
      {
        const float d = depthRow[c];
        if (!std::isfinite(d) || d <= 0.0f || d > maxDistance)
        {
          continue;
        }

        const int beam = std::min(input.nBeams - 1, (c * input.nBeams) / std::max(1, cols));
        const int bin =
          std::min(input.nFreq - 1, static_cast<int>((d / maxDistance) * input.nFreq));

        const float nz = std::max(0.0f, normalRow[c][2]);
        float reflectivity = 1.0f;
        if (input.reflectivityImage && !input.reflectivityImage->empty())
        {
          reflectivity = input.reflectivityImage->at<float>(r, c);
        }

        // CUDA-aligned physics: spread + lambert + stochastic phase term
        const float lambert = std::sqrt(nz);  // sqrt(cos(incidence)); nz == cos(acos(nz))
        const float spread = 1.0f / std::max(1e-6f, d * d);
        const float target_area = std::sqrt(d * area_scaler);
        const float amplitude_mag = input.sensorGain * source_term * spread *
                                    std::exp(-2.0f * attenuation * d) * lambert *
                                    std::sqrt(reflectivity) * target_area;

        const float phase_noise = static_cast<float>((beam * input.nRays + r) % input.nFreq) /
                                  static_cast<float>(input.nFreq) * 2.0f * static_cast<float>(M_PI);
        output.At(beam, bin) += std::complex<float>(
          amplitude_mag * std::cos(phase_noise) / std::sqrt(2.0f),
          amplitude_mag * std::sin(phase_noise) / std::sqrt(2.0f));
      }
    }

    // Apply the configured range window.
    if (input.window)
    {
      for (int b = 0; b < input.nBeams; ++b)
      {
        for (int f = 0; f < input.nFreq; ++f)
        {
          output.At(b, f) *= input.window[f];
        }
      }
    }

    // Apply beam correction matrix when available.
    if (input.beamCorrector && input.beamCorrectorSum > 0.0f)
    {
      std::vector<std::complex<float>> corrected = output.beamSpectrum;
      const float norm = input.beamCorrectorSum;
      for (int b = 0; b < input.nBeams; ++b)
      {
        for (int f = 0; f < input.nFreq; ++f)
        {
          std::complex<float> accum{0.0f, 0.0f};
          for (int k = 0; k < input.nBeams; ++k)
          {
            const float w = input.beamCorrector[b][k] / norm;
            accum += output.At(k, f) * w;
          }
          corrected[static_cast<size_t>(b) * input.nFreq + f] = accum;
        }
      }
      output.beamSpectrum.swap(corrected);
    }

    const auto stop = std::chrono::high_resolution_clock::now();
    output.computeMicros =
      std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
    return true;
  }
};

}  // namespace

std::unique_ptr<ComputeBackend> CreateComputeBackend(const std::string & requestedBackend)
{
  std::cerr << "[sonar_compute_factory] CreateComputeBackend called with: " << requestedBackend << std::endl;
  
  std::string backend = requestedBackend;
  std::transform(
    backend.begin(), backend.end(), backend.begin(),
    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  if (backend == "wgpu")
  {
    std::cerr << "[sonar_compute_factory] Attempting WgpuComputeBackend initialization..." << std::endl;
    auto wgpuBackend = std::make_unique<WgpuComputeBackend>();
    SonarComputeInput probe;
    if (wgpuBackend->Initialize(probe))
    {
      std::cerr << "[sonar_compute_factory] SUCCESS: WgpuComputeBackend initialized!" << std::endl;
      return wgpuBackend;
    }
    std::cerr << "[sonar_compute_factory] WGPU backend requested but FAILED to initialize."
              << std::endl;
    return nullptr;
  }

  if (backend == "cuda")
  {
    std::cerr << "[sonar_compute_factory] Attempting CudaComputeBackend initialization..." << std::endl;
    auto cudaBackend = std::make_unique<CudaComputeBackend>();
    SonarComputeInput probe;
    if (cudaBackend->Initialize(probe))
    {
      std::cerr << "[sonar_compute_factory] SUCCESS: CudaComputeBackend initialized!" << std::endl;
      return cudaBackend;
    }
    std::cerr << "[sonar_compute_factory] CUDA backend requested but failed to initialize." << std::endl;
    return nullptr;
  }

  if (backend == "cpu")
  {
    std::cerr << "[sonar_compute_factory] Creating CPU backend" << std::endl;
    return std::make_unique<CpuComputeBackend>();
  }

  if (backend == "auto")
  {
    std::cerr << "[sonar_compute_factory] Auto-selecting best available backend..." << std::endl;
    SonarComputeInput probe;
    auto cudaBackend = std::make_unique<CudaComputeBackend>();
    if (cudaBackend->Initialize(probe))
    {
      std::cerr << "[sonar_compute_factory] SUCCESS: Auto-selected CUDA backend" << std::endl;
      return cudaBackend;
    }

    auto wgpuBackend = std::make_unique<WgpuComputeBackend>();
    if (wgpuBackend->Initialize(probe))
    {
      std::cerr << "[sonar_compute_factory] SUCCESS: Auto-selected WGPU backend (CUDA unavailable)"
                << std::endl;
      return wgpuBackend;
    }
    std::cerr << "[sonar_compute_factory] SUCCESS: Auto-selected CPU backend (both CUDA and WGPU unavailable)"
              << std::endl;
  }

  std::cerr << "[sonar_compute_factory] Creating default CPU backend" << std::endl;
  return std::make_unique<CpuComputeBackend>();
}

}  // namespace sensors
}  // namespace gz
