#pragma once

#include <memory>

#include "sonar_compute_backend.hh"

namespace gz
{
namespace sensors
{

class WgpuComputeBackend : public ComputeBackend
{
public:
  const char * Name() const override;

  bool Initialize(const SonarComputeInput & prototype) override;

  bool Compute(const SonarComputeInput & input, SonarComputeOutput & output) override;

private:
  bool initialized{false};
  bool gpuAvailable{false};
  std::unique_ptr<ComputeBackend> cpuFallback;
};

}  // namespace sensors
}  // namespace gz
