#ifndef GZ_SENSORS_ACOUSTICBEAM_HH_
#define GZ_SENSORS_ACOUSTICBEAM_HH_

#include <cmath>
#include <gz/math/Angle.hh>
#include <gz/math/Pose3.hh>
#include <gz/math/Quaternion.hh>
#include <gz/math/Vector2.hh>
#include <gz/math/Vector3.hh>
#include <string>
#include "AxisAlignedPatch2.hh"

namespace gz::sensors
{

class AcousticBeam
{
public:
  AcousticBeam(
    int _id, const gz::math::Angle & _apertureAngle, const gz::math::Angle & _rotationAngle,
    const gz::math::Angle & _tiltAngle)
  : id(_id),
    apertureAngle(_apertureAngle),
    normalizedRadius(std::atan(_apertureAngle.Radian() / 2.))
  {
    using Quaterniond = gz::math::Quaterniond;

    // Rotação extrínseca XY (rot_x * rot_y)
    this->transform.Rot() = Quaterniond::EulerToQuaternion(_rotationAngle.Radian(), 0., 0.) *
                            Quaterniond::EulerToQuaternion(0., _tiltAngle.Radian(), 0.);

    this->axis = this->transform.Rot() * gz::math::Vector3d::UnitX;

    const gz::math::Angle azimuthAngle = std::atan2(this->axis.Y(), this->axis.X());
    const gz::math::Angle inclinationAngle = std::atan2(
      this->axis.Z(), std::sqrt(this->axis.X() * this->axis.X() + this->axis.Y() * this->axis.Y()));

    const gz::math::Vector2d topLeft{
      (azimuthAngle + _apertureAngle / 2.).Radian(),
      (inclinationAngle + _apertureAngle / 2.).Radian()};

    const gz::math::Vector2d bottomRight{
      (azimuthAngle - _apertureAngle / 2.).Radian(),
      (inclinationAngle - _apertureAngle / 2.).Radian()};

    this->sphericalFootprint = AxisAlignedPatch2d{topLeft, bottomRight};
  }

  int Id() const { return this->id; }
  const gz::math::Pose3d & Transform() const { return this->transform; }
  const gz::math::Vector3d & Axis() const { return this->axis; }
  double NormalizedRadius() const { return this->normalizedRadius; }
  const gz::math::Angle & ApertureAngle() const { return this->apertureAngle; }
  const AxisAlignedPatch2d & SphericalFootprint() const { return this->sphericalFootprint; }

private:
  int id;
  gz::math::Angle apertureAngle;
  double normalizedRadius;
  gz::math::Pose3d transform;
  gz::math::Vector3d axis;
  AxisAlignedPatch2d sphericalFootprint;
};

/// \brief Acoustic beam reflecting target.
/// Pose is defined w.r.t. the beams frame.
struct ObjectTarget
{
  gz::math::Pose3d pose;
  uint64_t entity;
  std::string name;
};

}  // namespace gz::sensors

#endif  // GZ_SENSORS_ACOUSTIC_BEAM_HH_
