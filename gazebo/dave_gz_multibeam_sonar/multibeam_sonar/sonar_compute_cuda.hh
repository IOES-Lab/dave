#pragma once

#include "sonar_compute_backend.hh"

namespace gz
{
namespace sensors
{

class CudaComputeBackend : public ComputeBackend
{
public:
  ~CudaComputeBackend() override;

  const char * Name() const override;

  bool Initialize(const SonarComputeInput & prototype) override;

  bool Compute(const SonarComputeInput & input, SonarComputeOutput & output) override;

private:
  bool initialized{false};
};

}  // namespace sensors
}  // namespace gz
