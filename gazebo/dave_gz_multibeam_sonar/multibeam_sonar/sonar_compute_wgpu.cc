/*
 * Copyright (C) 2025 Open Source Robotics Foundation
 * Licensed under the Apache License, Version 2.0
 */
#include "sonar_compute_wgpu.hh"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdint>
#include <iostream>
#include <vector>

#include <opencv2/core.hpp>

// Use a literal instead of M_PI — more portable across compilers/platforms
static constexpr float kPi = 3.14159265358979323846f;

#ifdef HAVE_WGPU_BACKEND
extern "C"
{
  void * sonar_wgpu_create(
    uint32_t n_beams, uint32_t n_rays, uint32_t n_freq, float sound_speed, float bandwidth,
    float max_range, float attenuation, float source_level, float sensor_gain, float h_fov,
    float v_fov, uint64_t seed);

  float * sonar_wgpu_compute(
    void * engine, const float * depth, const float * normals, const float * refl,
    const float * beam_corr, uint32_t n_beams, uint32_t n_rays, uint32_t n_freq, uint64_t frame,
    float beam_corr_sum);

  void sonar_wgpu_free(float * ptr, size_t len);
  void sonar_wgpu_destroy(void * engine);
}
#endif  // HAVE_WGPU_BACKEND

namespace gz
{
namespace sensors
{

// CPU fallback — always available, implements Eq.14 + Eq.8 without speckle
class CpuComputeBackend : public SonarComputeBackend
{
public:
  const char * Name() const override { return "cpu"; }
  bool Initialize(const SonarComputeInput &) override { return true; }

  bool Compute(const SonarComputeInput & input, SonarComputeOutput & output) override
  {
    if (!input.depthImage || !input.normalImage || !input.reflectivityImage)
    {
      return false;
    }

    const int nb = input.nBeams;
    const int nr = input.nRays;
    const int nf = input.nFreq;

    output.nBeams = nb;
    output.nFreq = nf;
    output.beamSpectrum.assign(static_cast<size_t>(nb) * nf, std::complex<float>(0.f, 0.f));

    const float c = static_cast<float>(input.soundSpeed);
    const float B = static_cast<float>(input.bandwidth);
    const float df = B / static_cast<float>(nf);

    auto t0 = std::chrono::high_resolution_clock::now();

    for (int b = 0; b < nb; ++b)
    {
      for (int ray = 0; ray < nr; ++ray)
      {
        if (ray >= input.depthImage->rows || b >= input.depthImage->cols)
        {
          continue;
        }

        const float r = input.depthImage->at<float>(ray, b);
        if (r <= 0.f || r > static_cast<float>(input.maxDistance))
        {
          continue;
        }

        const float mu = input.reflectivityImage->at<float>(ray, b);
        const cv::Vec3f n = input.normalImage->at<cv::Vec3f>(ray, b);
        const float cos_inc = std::abs(n[2]);

        const float dth = input.hFOV / std::max(nb - 1, 1);
        const float dphi = input.vFOV / std::max(nr - 1, 1);
        const float dA = r * r * dth * dphi;
        const float TL = std::exp(-static_cast<float>(input.attenuation) * r) / r;

        // Eq.14 amplitude (deterministic, no speckle in CPU path)
        const float A = std::sqrt(mu) * cos_inc * std::sqrt(dA) * TL;

        for (int f = 0; f < nf; ++f)
        {
          // DC-centred frequency grid (matches backscatter.wgsl convention)
          float freq;
          if (nf % 2 == 0)
          {
            freq = df * (-static_cast<float>(nf) + 2.f * (static_cast<float>(f) + 1.f)) / 2.f;
          }
          else
          {
            freq =
              df * (-(static_cast<float>(nf) - 1.f) + 2.f * (static_cast<float>(f) + 1.f)) / 2.f;
          }

          // Eq.8 two-way phase
          const float k = 2.f * kPi * freq / c;
          const float phi = 2.f * r * k;

          output.At(b, f) += std::complex<float>(A * std::cos(phi), A * std::sin(phi));
        }
      }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    output.computeMicros =
      static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count());

    return true;
  }
};

// Backend factory
std::unique_ptr<SonarComputeBackend> CreateComputeBackend(const std::string & name)
{
  if (name == "wgpu")
  {
    return std::make_unique<WgpuComputeBackend>();
  }
  // "cuda" is handled by existing sonar_calculation_cuda path
  return std::make_unique<CpuComputeBackend>();
}

// WgpuComputeBackend — Name
const char * WgpuComputeBackend::Name() const { return "wgpu"; }

// WgpuComputeBackend — destructor: clean up persistent engine
WgpuComputeBackend::~WgpuComputeBackend()
{
#ifdef HAVE_WGPU_BACKEND
  if (engine_)
  {
    sonar_wgpu_destroy(engine_);
    engine_ = nullptr;
  }
#endif
}

// WgpuComputeBackend — Initialize
// Probe GPU with a minimal 1-ray call. Non-fatal: failures route to CPU.
bool WgpuComputeBackend::Initialize(const SonarComputeInput &)
{
#ifndef HAVE_WGPU_BACKEND
  std::cerr << "[sonar_wgpu] backend not compiled in "
               "(cargo absent at build time) — CPU fallback active.\n";
  gpuAvailable_ = false;
  initialized_ = true;
  return true;
#else
  void * probe =
    sonar_wgpu_create(1u, 1u, 4u, 1500.f, 2950.f, 10.f, 0.f, 220.f, 1.f, 1.5708f, 0.3491f, 1u);

  if (probe)
  {
    sonar_wgpu_destroy(probe);
    gpuAvailable_ = true;
    std::cout << "[sonar_wgpu] GPU backend ready (wgpu/Vulkan).\n";
  }
  else
  {
    gpuAvailable_ = false;
    std::cerr << "[sonar_wgpu] GPU init failed — CPU fallback active.\n";
  }

  initialized_ = true;
  return true;
#endif
}

// WgpuComputeBackend — Compute
bool WgpuComputeBackend::Compute(const SonarComputeInput & input, SonarComputeOutput & output)
{
  if (!initialized_)
  {
    return false;
  }

  //  CPU fallback path
  if (!gpuAvailable_)
  {
    if (!cpuFallback_)
    {
      cpuFallback_ = std::make_unique<CpuComputeBackend>();
      cpuFallback_->Initialize(input);
    }
    return cpuFallback_->Compute(input, output);
  }

#ifndef HAVE_WGPU_BACKEND
  return false;
#else

  //  Validate inputs
  if (!input.depthImage || !input.normalImage || !input.reflectivityImage)
  {
    return false;
  }

  const int nb = input.nBeams;
  const int nr = input.nRays;
  const int nf = input.nFreq;
  if (nb <= 0 || nr <= 0 || nf <= 0)
  {
    return false;
  }

  auto t0 = std::chrono::high_resolution_clock::now();

  //  Flatten OpenCV layout [row=ray, col=beam] → [beam * nr + ray]
  std::vector<float> depthFlat(static_cast<size_t>(nb) * nr, 0.f);
  std::vector<float> reflFlat(static_cast<size_t>(nb) * nr, 0.f);
  std::vector<float> normalFlat(static_cast<size_t>(nb) * nr * 3, 0.f);

  for (int b = 0; b < nb; ++b)
  {
    for (int r = 0; r < nr; ++r)
    {
      if (r >= input.depthImage->rows || b >= input.depthImage->cols)
      {
        continue;
      }
      const size_t idx = static_cast<size_t>(b) * nr + r;
      depthFlat[idx] = input.depthImage->at<float>(r, b);
      reflFlat[idx] = input.reflectivityImage->at<float>(r, b);
      const cv::Vec3f n = input.normalImage->at<cv::Vec3f>(r, b);
      normalFlat[idx * 3 + 0] = n[0];
      normalFlat[idx * 3 + 1] = n[1];
      normalFlat[idx * 3 + 2] = n[2];
    }
  }

  //  Flatten beam corrector (identity if not provided)
  std::vector<float> beamCorrFlat(static_cast<size_t>(nb) * nb, 0.f);
  if (input.beamCorrector)
  {
    for (int r = 0; r < nb; ++r)
    {
      for (int c = 0; c < nb; ++c)
      {
        beamCorrFlat[r * nb + c] = input.beamCorrector[r][c];
      }
    }
  }
  else
  {
    // Identity fallback: no cross-beam mixing
    for (int i = 0; i < nb; ++i)
    {
      beamCorrFlat[i * nb + i] = 1.f;
    }
  }

  //  Persistent engine: recreate only when dimensions change
  // engine_ is a class member (not static local) — safe for multiple sensors
  if (!engine_ || engNb_ != nb || engNr_ != nr || engNf_ != nf)
  {
    if (engine_)
    {
      sonar_wgpu_destroy(engine_);
      engine_ = nullptr;
    }

    engine_ = sonar_wgpu_create(
      static_cast<uint32_t>(nb), static_cast<uint32_t>(nr), static_cast<uint32_t>(nf),
      static_cast<float>(input.soundSpeed), static_cast<float>(input.bandwidth),
      static_cast<float>(input.maxDistance), static_cast<float>(input.attenuation),
      static_cast<float>(input.sourceLevel), input.sensorGain, input.hFOV, input.vFOV, input.seed);

    engNb_ = nb;
    engNr_ = nr;
    engNf_ = nf;

    if (!engine_)
    {
      std::cerr << "[sonar_wgpu] sonar_wgpu_create failed "
                   "— switching to CPU fallback.\n";
      gpuAvailable_ = false;
      if (!cpuFallback_)
      {
        cpuFallback_ = std::make_unique<CpuComputeBackend>();
        cpuFallback_->Initialize(input);
      }
      return cpuFallback_->Compute(input, output);
    }
  }

  //  GPU dispatch
  float * result = sonar_wgpu_compute(
    engine_, depthFlat.data(), normalFlat.data(), reflFlat.data(), beamCorrFlat.data(),
    static_cast<uint32_t>(nb), static_cast<uint32_t>(nr), static_cast<uint32_t>(nf),
    input.frameIndex,
    input.beamCorrectorSum > 0.f ? input.beamCorrectorSum : static_cast<float>(nb));

  if (!result)
  {
    // Log failure explicitly — silent fallback makes debugging painful
    std::cerr << "[sonar_wgpu] sonar_wgpu_compute returned null "
                 "— CPU fallback for this frame.\n";
    if (!cpuFallback_)
    {
      cpuFallback_ = std::make_unique<CpuComputeBackend>();
      cpuFallback_->Initialize(input);
    }
    return cpuFallback_->Compute(input, output);
  }

  //  Unpack interleaved [re, im] result
  output.nBeams = nb;
  output.nFreq = nf;
  output.beamSpectrum.resize(static_cast<size_t>(nb) * nf);

  for (int b = 0; b < nb; ++b)
  {
    for (int f = 0; f < nf; ++f)
    {
      const size_t i = (static_cast<size_t>(b) * nf + f) * 2;
      // explicit constructor — avoids initializer-list ambiguity with templates
      output.At(b, f) = std::complex<float>(result[i], result[i + 1]);
    }
  }

  sonar_wgpu_free(result, static_cast<size_t>(nb) * nf * 2);

  auto t1 = std::chrono::high_resolution_clock::now();
  output.computeMicros =
    static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count());

  return true;
#endif  // HAVE_WGPU_BACKEND
}

}  // namespace sensors
}  // namespace gz