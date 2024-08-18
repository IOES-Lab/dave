/*
 * Copyright (C) 2022 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <optional>
#include <unordered_map>
#include <vector>

// TODO(hidmic): implement SVD in gazebo?
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/SVD>

#include <gz/common/Console.hh>
#include <gz/common/Event.hh>
#include <gz/common/Profiler.hh>
#include <gz/math/Helpers.hh>
#include <gz/math/Quaternion.hh>
#include <gz/math/Vector2.hh>
#include <gz/math/Vector3.hh>

#include <gz/msgs/float_v.pb.h>
#include <gz/msgs/pointcloud_packed.pb.h>
#include <gz/msgs/PointCloudPackedUtils.hh>
#include <gz/msgs/Utility.hh>

#include <gz/math/TimeVaryingVolumetricGrid.hh>

#include <gz/rendering/Camera.hh>
#include <gz/rendering/GpuRays.hh>
#include <gz/rendering/RayQuery.hh>

#include <gz/sensors/GaussianNoiseModel.hh>
#include <gz/sensors/Manager.hh>
#include <gz/sensors/Noise.hh>
#include <gz/sensors/RenderingEvents.hh>
#include <gz/sensors/RenderingSensor.hh>
#include <gz/sensors/SensorTypes.hh>
#include "MultibeamSonarSensor.hh"

#include <gz/transport/Node.hh>

namespace gz
{
namespace sensors
{
namespace
{

using RowMajorMatrix3d = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>;

/// \brief Axis-aligned patch on a plane, using image frame conventions.
template <typename T>
class AxisAlignedPatch2
{
public:
  AxisAlignedPatch2() = default;

public:
  AxisAlignedPatch2(
    const gz::math::Vector2<T> & _topLeft, const gz::math::Vector2<T> & _bottomRight)
  : topLeft(_topLeft), bottomRight(_bottomRight)
  {
  }

  /// \brief Scalar converting copy constructor
public:
  template <typename U>
  // cppcheck-suppress noExplicitConstructor
  AxisAlignedPatch2(const AxisAlignedPatch2<U> & _other)
  {
    this->topLeft.X(static_cast<T>(_other.XMax()));
    this->topLeft.Y(static_cast<T>(_other.YMax()));
    this->bottomRight.X(static_cast<T>(_other.XMin()));
    this->bottomRight.Y(static_cast<T>(_other.YMin()));
  }

public:
  T XMax() const { return this->topLeft.X(); }

public:
  T XMin() const { return this->bottomRight.X(); }

public:
  T XSize() const { return this->XMax() - this->XMin(); }

public:
  T YMax() const { return this->topLeft.Y(); }

public:
  T YMin() const { return this->bottomRight.Y(); }

public:
  T YSize() const { return this->YMax() - this->YMin(); }

  /// \brief Merge patch with `_other`.
  /// \return a patch that includes both.
public:
  AxisAlignedPatch2<T> & Merge(const AxisAlignedPatch2<T> & _other)
  {
    this->topLeft.Set(
      std::max(this->topLeft.X(), _other.topLeft.X()),
      std::max(this->topLeft.Y(), _other.topLeft.Y()));
    this->bottomRight.Set(
      std::min(this->bottomRight.X(), _other.bottomRight.X()),
      std::min(this->bottomRight.Y(), _other.bottomRight.Y()));
    return *this;
  }

  /// \brief Flip patch, sending each corner to the opposite quadrant.
public:
  AxisAlignedPatch2<T> Flip() const { return {-this->bottomRight, -this->topLeft}; }

  /// \brief Broadcast multiply corner coordinates by `_vector`
  /// coordinates.
  const AxisAlignedPatch2<T> operator*(gz::math::Vector2<T> _vector) const
  {
    return {this->topLeft * _vector, this->bottomRight * _vector};
  }

  /// \brief Broadcast divide corner coordinates by `_vector`
  /// coordinates.
  const AxisAlignedPatch2<T> operator/(gz::math::Vector2<T> _vector) const
  {
    return {this->topLeft / _vector, this->bottomRight / _vector};
  }

  /// \brief Broadcast sum corner coordinates with `_vector` coordinates.
  const AxisAlignedPatch2<T> operator+(gz::math::Vector2<T> _vector) const
  {
    return {this->topLeft + _vector, this->bottomRight + _vector};
  }

  /// \brief Broadcast subtract corner coordinate with `_vector`
  /// coordinates.
  const AxisAlignedPatch2<T> operator-(gz::math::Vector2<T> _vector) const
  {
    return {this->topLeft - _vector, this->bottomRight - _vector};
  }

  /// \brief Upper-left corner i.e. (x, y) maxima
private:
  gz::math::Vector2<T> topLeft;

  /// \brief Bottom-right corner i.e (x, y) minima
private:
  gz::math::Vector2<T> bottomRight;
};

// Handy type definitions
using AxisAlignedPatch2d = AxisAlignedPatch2<double>;
using AxisAlignedPatch2i = AxisAlignedPatch2<int>;

/// \brief Acoustic DVL beam, modelled as a circular cone with aperture
/// angle α and its apex at the origin. Its axis of symmetry is nominally
/// aligned with the x-axis of an x-forward, y-left, z-up frame
/// (following usual Gazebo frame conventions, typically facing
/// downwards).
///
///          +            The cone may be tilted w.r.t. the x-axis
///         /|\           and rotated about the same to accommodate
///        / | \          different beam arrangements, in that order.
///       /  |  \         That is, an extrinsic XY rotation applies.
///      /   v   \        /
///         x
///      |-------|
///          α
class AcousticBeam
{
  /// \brief Acoustic beam constructor.
  /// \param[in] _id ID of the beam. Ought to be unique.
  /// \param[in] _apertureAngle Aperture angle α of the beam.
  /// \param[in] _rotationAngle Rotation angle ψ of the beam
  /// i.e. a rotation about the x-axis of its frame.
  /// \param[in] _tiltAngle Tilt angle φ of the
  /// beam i.e. a rotation about the y-axis of its frame,
  /// away from the x-axis. Must lie in the (-90, 90) degrees
  /// interval.
public:
  AcousticBeam(
    const int _id, const gz::math::Angle _apertureAngle, const gz::math::Angle _rotationAngle,
    const gz::math::Angle _tiltAngle)
  : id(_id),
    apertureAngle(_apertureAngle),
    normalizedRadius(std::atan(_apertureAngle.Radian() / 2.))
  {
    // Use extrinsic XY convention (as it is easier to reason about)
    using Quaterniond = gz::math::Quaterniond;
    this->transform.Rot() = Quaterniond::EulerToQuaternion(_rotationAngle.Radian(), 0., 0.) *
                            Quaterniond::EulerToQuaternion(0., _tiltAngle.Radian(), 0.);
    this->axis = this->transform.Rot() * gz::math::Vector3d::UnitX;
    const gz::math::Angle azimuthAngle = std::atan2(this->axis.Y(), this->axis.X());
    const gz::math::Angle inclinationAngle = std::atan2(
      this->axis.Z(), std::sqrt(std::pow(this->axis.X(), 2.) + std::pow(this->axis.Y(), 2.)));
    const gz::math::Vector2d topLeft{
      (azimuthAngle + _apertureAngle / 2.).Radian(),
      (inclinationAngle + _apertureAngle / 2.).Radian()};
    const gz::math::Vector2d bottomRight{
      (azimuthAngle - _apertureAngle / 2.).Radian(),
      (inclinationAngle - _apertureAngle / 2.).Radian()};
    this->sphericalFootprint = AxisAlignedPatch2d{topLeft, bottomRight};
  }

public:
  int Id() const { return this->id; }

public:
  const gz::math::Pose3d & Transform() const { return this->transform; }

public:
  const gz::math::Vector3d & Axis() const { return this->axis; }

public:
  double NormalizedRadius() const { return this->normalizedRadius; }

public:
  const gz::math::Angle & ApertureAngle() const { return this->apertureAngle; }

public:
  const AxisAlignedPatch2d & SphericalFootprint() const { return this->sphericalFootprint; }

private:
  int id;

private:
  gz::math::Angle apertureAngle;

private:
  double normalizedRadius;

private:
  gz::math::Pose3d transform;

private:
  gz::math::Vector3d axis;

private:
  AxisAlignedPatch2d sphericalFootprint;
};

/// \brief DVL acoustic beam reflecting target.
///
/// Pose is defined w.r.t. to the beams frame.
struct ObjectTarget
{
  gz::math::Pose3d pose;
  uint64_t entity;
  std::string name;
};

/// \brief A time-varying vector field built on
/// per-axis time-varying volumetric data grids
///
/// \see gz::math::InMemoryTimeVaryingVolumetricGrid
template <typename T, typename V = T, typename P = T>
class InMemoryTimeVaryingVectorField
{
public:
  using SessionT = gz::math::InMemorySession<T, P>;

public:
  using GridT = gz::math::InMemoryTimeVaryingVolumetricGrid<T, V, P>;

  /// \brief Default constructor.
public:
  InMemoryTimeVaryingVectorField() = default;

  /// \brief Constructor
  /// \param[in] _xData X-axis volumetric data grid.
  /// \param[in] _yData Y-axis volumetric data grid.
  /// \param[in] _zData Z-axis volumetric data grid.
public:
  explicit InMemoryTimeVaryingVectorField(
    const GridT * _xData, const GridT * _yData, const GridT * _zData)
  : xData(_xData), yData(_yData), zData(_zData)
  {
    if (this->xData)
    {
      this->xSession = this->xData->CreateSession();
    }
    if (this->yData)
    {
      this->ySession = this->yData->CreateSession();
    }
    if (this->zData)
    {
      this->zSession = this->zData->CreateSession();
    }
  }

  /// \brief Advance vector field in time.
  /// \param[in] _now Time to step data grids to.
public:
  void StepTo(const std::chrono::steady_clock::duration & _now)
  {
    const T now = std::chrono::duration<T>(_now).count();
    if (this->xData && this->xSession)
    {
      this->xSession = this->xData->StepTo(this->xSession.value(), now);
    }
    if (this->yData && this->ySession)
    {
      this->ySession = this->yData->StepTo(this->ySession.value(), now);
    }
    if (this->zData && this->zSession)
    {
      this->zSession = this->zData->StepTo(this->zSession.value(), now);
    }
  }

  /// \brief Look up vector field value, interpolating data grids.
  /// \param[in] _pos Vector field argument.
  /// \return vector field value at `_pos`
public:
  gz::math::Vector3<V> LookUp(const gz::math::Vector3<P> & _pos)
  {
    auto outcome = gz::math::Vector3<V>::Zero;
    if (this->xData && this->xSession)
    {
      const auto interpolation = this->xData->LookUp(this->xSession.value(), _pos);
      outcome.X(interpolation.value_or(0.));
    }
    if (this->yData && this->ySession)
    {
      const auto interpolation = this->yData->LookUp(this->ySession.value(), _pos);
      outcome.Y(interpolation.value_or(0.));
    }
    if (this->zData && this->zSession)
    {
      const auto interpolation = this->zData->LookUp(this->ySession.value(), _pos);
      outcome.Z(interpolation.value_or(0.));
    }
    return outcome;
  }

  /// \brief Session for x-axis volumetric data grid, if any.
private:
  std::optional<SessionT> xSession{std::nullopt};

  /// \brief Session for y-axis volumetric data grid, if any.
private:
  std::optional<SessionT> ySession{std::nullopt};

  /// \brief Session for z-axis volumetric data grid, if any.
private:
  std::optional<SessionT> zSession{std::nullopt};

  /// \brief X-axis volumetric data grid, if any.
private:
  const GridT * xData{nullptr};

  /// \brief Y-axis volumetric data grid, if any.
private:
  const GridT * yData{nullptr};

  /// \brief Z-axis volumetric data grid, if any.
private:
  const GridT * zData{nullptr};
};

}  // namespace

using namespace gz::msgs;

/// \brief Implementation for MultibeamSonarSensor
class MultibeamSonarSensor::Implementation
{
  /// \brief SDF DOM object
public:
  sdf::ElementPtr sensorSdf;

  /// \brief Sensor entity ID (for world state lookup)
public:
  uint64_t entityId{0};

  /// \brief true if Load() has been called and was successful
public:
  bool initialized = false;

  /// \brief Initialize DVL sensor
public:
  bool Initialize(MultibeamSonarSensor * _sensor);

  /// \brief Raw buffer of ray data.
public:
  float * rayBuffer = nullptr;

  /// \brief Number of channels of the raw ray buffer
public:
  const unsigned int kChannelCount = 3u;

  GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
  /// \brief Just a mutex for thread safety
public:
  mutable std::mutex rayMutex;
  GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING

  /// \brief Initialize beam arrangement for DVL sensor
  ///
  /// This primarily creates rendering sensors.
public:
  bool InitializeBeamArrangement(MultibeamSonarSensor * _sensor);

  /// \brief Maximum range for beams.
public:
  double maximumRange;

  /// \brief State of the world.
public:
  const WorldState * worldState;

  /// \brief Depth sensor (i.e. a GPU raytracing sensor).
public:
  gz::rendering::GpuRaysPtr depthSensor;

  /// \brief Image sensor (i.e. a camera sensor) to aid ray queries.
public:
  gz::rendering::CameraPtr imageSensor;

  /// \brief Depth sensor intrinsic constants
public:
  struct
  {
    gz::math::Vector2d offset;  ///<! Azimuth and elevation offsets
    gz::math::Vector2d step;    ///<! Azimuth and elevation steps
  } depthSensorIntrinsics;

  /// \brief Callback for rendering sensor frames
public:
  void OnNewFrame(
    const float * _scan, unsigned int _width, unsigned int _height, unsigned int _channels,
    const std::string & /*_format*/);

  /// \brief Connection from depth camera with new depth data.
public:
  gz::common::ConnectionPtr depthConnection;

  /// \brief Connection to the Manager's scene change event.
public:
  gz::common::ConnectionPtr sceneChangeConnection;

  /// \brief DVL acoustic beams' description
public:
  std::vector<AcousticBeam> beams;

  /// \brief Rotation from sensor frame to reference frame.
  ///
  /// Useful to cope with different DVL frame conventions.
public:
  gz::math::Quaterniond referenceFrameRotation;

  /// \brief Transform from sensor frame to acoustic beams' frame.
  ///
  /// I.e. x-forward, y-left, z-up (dvl sensor frame) rotates to
  /// x-down, y-left, z-forward (acoustic beams' frame).
public:
  const gz::math::Pose3d beamsFrameTransform{
    gz::math::Vector3d::Zero, gz::math::Quaterniond{0., GZ_PI / 2., 0.}};

  /// \brief DVL acoustic beams' targets
public:
  std::vector<std::optional<ObjectTarget>> beamTargets;

  /// \brief DVL acoustic beams' patches in depth scan frame.
public:
  std::vector<AxisAlignedPatch2i> beamScanPatches;

  /// \brief The point cloud message.
public:
  msgs::PointCloudPacked pointMsg;

  /// \brief Fill the point cloud packed message
  /// \param[in] _rayBuffer Ray data buffer.
public:
  void FillPointCloudMsg(const float * _rayBuffer);

  /// \brief Node to create a topic publisher with.
public:
  gz::transport::Node node;

  /// \brief Publisher for messages.
public:
  gz::transport::Node::Publisher pointPub;

  /// \brief Publisher for messages.
public:
  gz::transport::Node::Publisher pointFloatPub;

  /// \brief Flag to indicate if sensor should be publishing estimates.
public:
  bool publishingEstimates = false;
};

//////////////////////////////////////////////////
MultibeamSonarSensor::MultibeamSonarSensor() : dataPtr(new Implementation()) {}

//////////////////////////////////////////////////
MultibeamSonarSensor::~MultibeamSonarSensor()
{
  this->dataPtr->depthConnection.reset();
  this->dataPtr->sceneChangeConnection.reset();
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Load(const sdf::Sensor & _sdf)
{
  if (!gz::sensors::RenderingSensor::Load(_sdf))
  {
    return false;
  }

  // Check if this sensor is of the right type
  if (_sdf.Type() != sdf::SensorType::CUSTOM)
  {
    gzerr << "Expected [" << this->Name() << "] sensor to be "
          << "a DVL but found a " << _sdf.TypeStr() << "." << std::endl;
    return false;
  }

  sdf::ElementPtr elem = _sdf.Element();
  if (!elem->HasAttribute("gz:type"))
  {
    gzerr << "Missing 'gz:type' attribute "
          << "for sensor [" << this->Name() << "]. "
          << "Aborting load." << std::endl;
    return false;
  }
  const auto type = elem->Get<std::string>("gz:type");
  if (type != "multibeam_sonar")
  {
    gzerr << "Expected sensor [" << this->Name() << "] to be a "
          << "DVL but it is of '" << type << "' type. Aborting load." << std::endl;
    return false;
  }
  if (!elem->HasElement("gz:multibeam_sonar"))
  {
    gzerr << "Missing 'gz:multibeam_sonar' configuration for "
          << "sensor [" << this->Name() << "]. "
          << "Aborting load." << std::endl;
    return false;
  }
  this->dataPtr->sensorSdf = elem->GetElement("gz:multibeam_sonar");

  // Initialize the point message.
  // \todo(anyone) The true value in the following function call forces
  // the xyz and rgb fields to be aligned to memory boundaries. This is need
  // by ROS1: https://github.com/ros/common_msgs/pull/77. Ideally, memory
  // alignment should be configured. This same problem is in the
  // RgbdCameraSensor.
  msgs::InitPointCloudPacked(
    this->dataPtr->pointMsg, this->FrameId(), true,
    {{"xyz", msgs::PointCloudPacked::Field::FLOAT32},
     {"intensity", msgs::PointCloudPacked::Field::FLOAT32},
     {"ring", msgs::PointCloudPacked::Field::UINT16}});

  // Instantiate interfaces
  // Create the point cloud publisher
  this->dataPtr->pointPub =
    this->dataPtr->node.Advertise<gz::msgs::PointCloudPacked>(this->Topic() + "/point_cloud");
  this->dataPtr->pointFloatPub =
    this->dataPtr->node.Advertise<gz::msgs::Float_V>(this->Topic() + "/point_cloud_float_vector");
  if (!this->dataPtr->pointPub)
  {
    gzerr << "Unable to create publisher on topic "
          << "[" << this->Topic() << "] for sensor "
          << "[" << this->Name() << "]" << std::endl;
    return false;
  }

  // Setup sensors
  if (this->Scene())
  {
    if (!this->dataPtr->Initialize(this))
    {
      gzerr << "Failed to setup [" << this->Name() << "] sensor. " << std::endl;
      return false;
    }
  }

  gzmsg << "Loaded [" << this->Name() << "] Multibeam Sonar sensor." << std::endl;
  this->dataPtr->sceneChangeConnection = gz::sensors::RenderingEvents::ConnectSceneChangeCallback(
    std::bind(&MultibeamSonarSensor::SetScene, this, std::placeholders::_1));

  return true;
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Load(sdf::ElementPtr _sdf)
{
  sdf::Sensor sdfSensor;
  sdfSensor.Load(_sdf);
  return this->Load(sdfSensor);
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Implementation::Initialize(MultibeamSonarSensor * _sensor)
{
  gzmsg << "Initializing [" << _sensor->Name() << "] sensor." << std::endl;

  if (!this->InitializeBeamArrangement(_sensor))
  {
    gzerr << "Failed to initialize beam arrangement for "
          << "[" << _sensor->Name() << "] sensor." << std::endl;
    return false;
  }

  gz::math::Pose3d referenceFrameTransform =
    this->sensorSdf->Get<gz::math::Pose3d>("reference_frame", gz::math::Pose3d{}).first;

  this->referenceFrameRotation = referenceFrameTransform.Rot().Inverse();

  gzmsg << "Initialized [" << _sensor->Name() << "] sensor." << std::endl;
  this->initialized = true;
  return true;
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Implementation::InitializeBeamArrangement(MultibeamSonarSensor * _sensor)
{
  this->beams.clear();
  this->beamTargets.clear();

  this->depthSensor = _sensor->Scene()->CreateGpuRays(_sensor->Name() + "_depth_sensor");
  if (!this->depthSensor)
  {
    gzerr << "Failed to create Depth (GPU Ray) sensor for "
          << "for [" << _sensor->Name() << "] sensor." << std::endl;
    return false;
  }

  // Read ray definition from SDF
  sdf::ElementPtr rayElement = this->sensorSdf->GetElement("ray");
  if (!rayElement)
  {
    gzerr << "No beam properties(format of GPU Ray) specified for "
          << "[" << _sensor->Name() << "] sensor" << std::endl;
    return false;
  }
  const bool useDegrees = rayElement->Get("degrees", false).first;
  const gz::math::Angle angleUnit = useDegrees ? GZ_DTOR(1.) : 1.;

  // -------- Assign depth sensor properties
  sdf::ElementPtr rangeElement = rayElement->GetElement("range");
  const double minimumRange = rangeElement->Get<double>("min", 0.1).first;
  gzmsg << "Setting minimum range to " << minimumRange << " m for [" << _sensor->Name()
        << "] sensor." << std::endl;
  this->depthSensor->SetNearClipPlane(minimumRange);
  this->maximumRange = rangeElement->Get<double>("max", 5.).first;
  gzmsg << "Setting maximum range to " << this->maximumRange << " m for [" << _sensor->Name()
        << "] sensor." << std::endl;
  this->depthSensor->SetFarClipPlane(this->maximumRange);

  // Mask ranges outside of min/max to +/- inf, as per REP 117
  this->depthSensor->SetClamp(false);

  sdf::ElementPtr horizontalElement = rayElement->GetElement("scan")->GetElement("horizontal");
  const double horizAngleMin = horizontalElement->Get<double>("min_angle", -M_PI / 4.0).first;
  const double horizAngleMax = horizontalElement->Get<double>("max_angle", M_PI / 4.0).first;

  sdf::ElementPtr verticalElement = rayElement->GetElement("scan")->GetElement("vertical");
  const double verticalAngleMin = verticalElement->Get<double>("min_angle", -M_PI / 8.0).first;
  const double verticalAngleMax = verticalElement->Get<double>("max_angle", M_PI / 8.0).first;

  const int beamCount = horizontalElement->Get<int>("beams", 256).first;
  const int rayCount = verticalElement->Get<int>("rays", 3).first;

  // ---- Construct AcousticBeam
  // Initialize beamId
  int beamId = 0;

  // Iterate through the rays
  for (int v = 0; v < rayCount; ++v)
  {
    for (int h = 0; h < beamCount; ++h)
    {
      // Calculate beam angles
      gz::math::Angle beamApertureAngle =
        gz::math::Angle(verticalAngleMax - verticalAngleMin) * angleUnit;
      gz::math::Angle beamRotationAngle =
        gz::math::Angle(horizAngleMin + (h * (horizAngleMax - horizAngleMin) / beamCount)) *
        angleUnit;
      gz::math::Angle beamTiltAngle =
        gz::math::Angle(verticalAngleMin + (v * (verticalAngleMax - verticalAngleMin) / rayCount)) *
        angleUnit;

      // Normalize angles
      beamApertureAngle = beamApertureAngle.Normalized();
      beamRotationAngle = beamRotationAngle.Normalized();
      beamTiltAngle = beamTiltAngle.Normalized();

      // Build acoustic beam
      this->beams.push_back(
        AcousticBeam{beamId, beamApertureAngle, beamRotationAngle, beamTiltAngle});

      // Increment beamId
      ++beamId;
    }
  }

  // Add as many (still null) targets as beams
  this->beamTargets.resize(this->beams.size());

  // Aggregate all beams' footprint in spherical coordinates into one
  AxisAlignedPatch2d beamsSphericalFootprint;
  for (const auto & beam : this->beams)
  {
    beamsSphericalFootprint.Merge(beam.SphericalFootprint());
  }
  // Rendering sensors' FOV must be symmetric about its main axis
  beamsSphericalFootprint.Merge(beamsSphericalFootprint.Flip());

  this->depthSensor->SetAngleMin(beamsSphericalFootprint.XMin());
  this->depthSensor->SetAngleMax(beamsSphericalFootprint.XMax());
  auto horizontalRayCount = beamCount;
  if (horizontalRayCount % 2 == 0)
  {
    ++horizontalRayCount;  // ensure odd
  }
  this->depthSensor->SetRayCount(horizontalRayCount);

  this->depthSensor->SetVerticalAngleMin(beamsSphericalFootprint.YMin());
  this->depthSensor->SetVerticalAngleMax(beamsSphericalFootprint.YMax());
  auto verticalRayCount = rayCount;
  if (verticalRayCount % 2 == 0)
  {
    ++verticalRayCount;  // ensure odd
  }

  this->depthSensor->SetVerticalRayCount(verticalRayCount);

  auto & intrinsics = this->depthSensorIntrinsics;
  intrinsics.offset.X(beamsSphericalFootprint.XMin());
  intrinsics.offset.Y(beamsSphericalFootprint.YMin());
  intrinsics.step.X(beamsSphericalFootprint.XSize() / (horizontalRayCount - 1));
  intrinsics.step.Y(beamsSphericalFootprint.YSize() / (verticalRayCount - 1));

  // Pre-compute scan indices covered by beam spherical
  // footprints for speed during scan iteration
  this->beamScanPatches.clear();
  for (const auto & beam : this->beams)
  {
    this->beamScanPatches.push_back(
      AxisAlignedPatch2i{(beam.SphericalFootprint() - intrinsics.offset) / intrinsics.step});
  }

  // _sensor->Scene()->RootVisual()->AddChild(this->depthSensor);

  this->depthSensor->SetVisibilityMask(GZ_VISIBILITY_ALL);

  _sensor->AddSensor(this->depthSensor);

  // Set the values on the point message.
  this->pointMsg.set_width(this->depthSensor->RangeCount());
  this->pointMsg.set_height(this->depthSensor->VerticalRangeCount());
  this->pointMsg.set_row_step(this->pointMsg.point_step() * this->pointMsg.width());

  this->imageSensor = _sensor->Scene()->CreateCamera(_sensor->Name() + "_image_sensor");
  if (!this->imageSensor)
  {
    gzerr << "Failed to create image sensor for "
          << "for [" << _sensor->Name() << "] sensor." << std::endl;
    return false;
  }

  this->imageSensor->SetImageWidth(horizontalRayCount);
  this->imageSensor->SetImageHeight(verticalRayCount);

  this->imageSensor->SetNearClipPlane(minimumRange);
  this->imageSensor->SetFarClipPlane(this->maximumRange);
  this->imageSensor->SetAntiAliasing(2);

  this->imageSensor->SetAspectRatio(
    beamsSphericalFootprint.XSize() / beamsSphericalFootprint.YSize());
  this->imageSensor->SetHFOV(beamsSphericalFootprint.XSize());
  this->imageSensor->SetVisibilityMask(~GZ_VISIBILITY_GUI);

  _sensor->AddSensor(this->imageSensor);

  this->depthConnection = this->depthSensor->ConnectNewGpuRaysFrame(std::bind(
    &MultibeamSonarSensor::Implementation::OnNewFrame, this, std::placeholders::_1,
    std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

  return true;
}

//////////////////////////////////////////////////
std::vector<gz::rendering::SensorPtr> MultibeamSonarSensor::RenderingSensors() const
{
  return {this->dataPtr->depthSensor, this->dataPtr->imageSensor};
}

//////////////////////////////////////////////////
void MultibeamSonarSensor::Implementation::OnNewFrame(
  const float * _scan, unsigned int _width, unsigned int _height, unsigned int _channels,
  const std::string & /*_format*/)
{
  // From GPU Lidar sensor
  std::lock_guard<std::mutex> lock(this->rayMutex);

  unsigned int samples = _width * _height * _channels;
  unsigned int rayBufferSize = samples * sizeof(float);

  if (!this->rayBuffer)
  {
    this->rayBuffer = new float[samples];
  }

  memcpy(this->rayBuffer, _scan, rayBufferSize);

  // From DVL sensor
  const auto & intrinsics = this->depthSensorIntrinsics;

  for (size_t i = 0; i < this->beams.size(); ++i)
  {
    const AxisAlignedPatch2i & beamScanPatch = this->beamScanPatches[i];
    const AcousticBeam & beam = this->beams[i];

    // Clear existing target, if any
    std::optional<ObjectTarget> & beamTarget = this->beamTargets[i];
    beamTarget.reset();

    // Iterate over the beam solid angle in camera coordinates
    for (auto v = beamScanPatch.YMin(); v < beamScanPatch.YMax(); ++v)
    {
      assert(v >= 0 && v < static_cast<int>(_height));
      const gz::math::Angle inclination = v * intrinsics.step.Y() + intrinsics.offset.Y();

      for (auto u = beamScanPatch.XMin(); u < beamScanPatch.XMax(); ++u)
      {
        assert(u >= 0 && u < static_cast<int>(_width));

        const float range = _scan[(u + v * _width) * _channels];
        if (!std::isfinite(range))
        {
          continue;
        }

        const gz::math::Angle azimuth = u * intrinsics.step.X() + intrinsics.offset.X();

        // Convert to cartesian coordinates in the acoustic beams' frame
        const auto point = range * gz::math::Vector3d{
                                     std::cos(inclination.Radian()) * std::cos(azimuth.Radian()),
                                     std::cos(inclination.Radian()) * std::sin(azimuth.Radian()),
                                     std::sin(inclination.Radian())};

        // Track point if (a) it effectively lies within the
        // beam's aperture and (b) it is the closest seen so far
        const gz::math::Angle angle = std::acos(point.Normalized().Dot(beam.Axis()));
        if (angle < beam.ApertureAngle() / 2.)
        {
          if (beamTarget)
          {
            if (beamTarget->pose.Pos().Length() > point.Length())
            {
              beamTarget->pose.Pos() = point;
            }
          }
          else
          {
            beamTarget = {gz::math::Pose3d{point, gz::math::Quaterniond::Identity}, 0};
          }
        }  // End of aperture angle loop
      }  // End of beamScanPatch X loop
    }  // End of beamScanPatch Y loop
  }  // End of beam loop
}

/////////////////////////////////////////////////
void MultibeamSonarSensor::SetScene(gz::rendering::ScenePtr _scene)
{
  // APIs make it possible for the scene pointer to change
  if (this->Scene() != _scene)
  {
    // TODO(anyone) Remove camera from scene
    this->dataPtr->depthSensor = nullptr;
    this->dataPtr->imageSensor = nullptr;
    if (this->dataPtr->rayBuffer)
    {
      delete[] this->dataPtr->rayBuffer;
      this->dataPtr->rayBuffer = nullptr;
    }
    RenderingSensor::SetScene(_scene);
    if (!this->dataPtr->initialized)
    {
      if (!this->dataPtr->Initialize(this))
      {
        gzerr << "Failed to initialize "
              << "[" << this->Name() << "]"
              << " sensor." << std::endl;
      }
    }
  }
}

//////////////////////////////////////////////////
void MultibeamSonarSensor::SetWorldState(const WorldState & _state)
{
  this->dataPtr->worldState = &_state;
}
//////////////////////////////////////////////////
void MultibeamSonarSensor::SetEnvironmentalData(const EnvironmentalData & _data) {}

//////////////////////////////////////////////////
void MultibeamSonarSensor::SetEntity(uint64_t _entityId) { this->dataPtr->entityId = _entityId; }

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Update(const std::chrono::steady_clock::duration & _now)
{
  GZ_PROFILE("MultibeamSonarSensor::Update");
  if (!this->dataPtr->initialized || this->dataPtr->entityId == 0)
  {
    gzerr << "Not initialized, update ignored." << std::endl;
    return false;
  }

  if (!this->dataPtr->depthSensor)
  {
    gzerr << "Depth (GpuRays) Sensor for Multibeam Sonar doesn't exist.\n";
    return false;
  }

  const gz::math::Pose3d beamsFramePose = this->Pose() * this->dataPtr->beamsFrameTransform;
  this->dataPtr->depthSensor->SetLocalPose(beamsFramePose);
  this->dataPtr->imageSensor->SetLocalPose(beamsFramePose);

  // Generate sensor data
  this->Render();

  if (this->dataPtr->pointPub.HasConnections())
  {
    // Set the time stamp
    *this->dataPtr->pointMsg.mutable_header()->mutable_stamp() = msgs::Convert(_now);
    // Set frame_id
    for (auto i = 0; i < this->dataPtr->pointMsg.mutable_header()->data_size(); ++i)
    {
      if (
        this->dataPtr->pointMsg.mutable_header()->data(i).key() == "frame_id" &&
        this->dataPtr->pointMsg.mutable_header()->data(i).value_size() > 0)
      {
        this->dataPtr->pointMsg.mutable_header()->mutable_data(i)->set_value(0, this->FrameId());
      }
    }

    this->dataPtr->FillPointCloudMsg(this->dataPtr->rayBuffer);

    // For the point cloud visualization in gazebo
    // https://github.com/gazebosim/gz-gui/pull/346
    // What is this? doesn't look right
    gz::msgs::Float_V floatVMsg;
    gz::msgs::PointCloudPackedIterator<float> xIter(this->dataPtr->pointMsg, "x");
    gz::msgs::PointCloudPackedIterator<float> yIter(this->dataPtr->pointMsg, "y");
    gz::msgs::PointCloudPackedIterator<float> zIter(this->dataPtr->pointMsg, "z");

    for (float x = 0.0, y = 0.0, z = 0.0; xIter != xIter.End(); ++xIter, ++yIter, ++zIter)
    {
      *xIter = x;
      *yIter = y;
      *zIter = z;
      floatVMsg.add_data(1);

      x += 1.0;
      if (x > 9)
      {
        x = 0.0;
        y += 1.0;
      }
      if (y > 9)
      {
        y = 0.0;
        z += 1.0;
      }
    }
    this->dataPtr->pointFloatPub.Publish(floatVMsg);

    {
      this->AddSequence(this->dataPtr->pointMsg.mutable_header());
      GZ_PROFILE("MultibeamSonarSensor::Update Publish point cloud");
      this->dataPtr->pointPub.Publish(this->dataPtr->pointMsg);
    }
  }

  return true;
}

//////////////////////////////////////////////////
void MultibeamSonarSensor::PostUpdate(const std::chrono::steady_clock::duration & _now)
{
  GZ_PROFILE("MultibeamSonarSensor::PostUpdate");

  if (!this->dataPtr->worldState)
  {
    gzwarn << "No world state available, "
           << "cannot estimate velocities." << std::endl;
    return;
  }

  for (size_t i = 0; i < this->dataPtr->beams.size(); ++i)
  {
    auto & beamTarget = this->dataPtr->beamTargets[i];
    if (beamTarget)
    {
      // TODO(hidmic): use shader to fetch target entity id
      const gz::math::Vector2i pixel = this->dataPtr->imageSensor->Project(beamTarget->pose.Pos());
      auto visual = this->dataPtr->imageSensor->VisualAt(pixel);
      if (visual)
      {
        if (visual->HasUserData("gazebo-entity"))
        {
          auto user_data = visual->UserData("gazebo-entity");
          beamTarget->entity = std::get<uint64_t>(user_data);
        }
        else
        {
          gzdbg << "No entity associated to [" << visual->Name() << "]"
                << " visual. Assuming it is static w.r.t. the world." << std::endl;
        }
      }
    }
  }

  // auto * headerMessage = waterMassModeMessage.mutable_header();
  // auto frame = headerMessage->add_data();
  // frame->set_key("frame_id");
  // frame->add_value(this->FrameId());
  // this->AddSequence(headerMessage, "doppler_velocity_log");
  // this->dataPtr->pointPub.Publish(waterMassModeMessage);
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::HasConnections() const
{
  return this->dataPtr->pointPub && this->dataPtr->pointPub.HasConnections();
}

//////////////////////////////////////////////////
void MultibeamSonarSensor::Implementation::FillPointCloudMsg(const float * _rayBuffer)
{
  uint32_t width = this->pointMsg.width();
  uint32_t height = this->pointMsg.height();
  unsigned int channels = 3;

  float angleStep = (this->depthSensor->AngleMax() - this->depthSensor->AngleMin()).Radian() /
                    (this->depthSensor->RangeCount() - 1);

  float verticleAngleStep =
    (this->depthSensor->VerticalAngleMax() - this->depthSensor->VerticalAngleMin()).Radian() /
    (this->depthSensor->VerticalRangeCount() - 1);

  // Angles of ray currently processing, azimuth is horizontal, inclination
  // is vertical
  float inclination = this->depthSensor->VerticalAngleMin().Radian();

  std::string * msgBuffer = this->pointMsg.mutable_data();
  msgBuffer->resize(this->pointMsg.row_step() * this->pointMsg.height());
  char * msgBufferIndex = msgBuffer->data();
  // Set Pointcloud as dense. Change if invalid points are found.
  bool isDense{true};
  // Iterate over scan and populate point cloud
  for (uint32_t j = 0; j < height; ++j)
  {
    float azimuth = this->depthSensor->AngleMin().Radian();

    for (uint32_t i = 0; i < width; ++i)
    {
      // Index of current point, and the depth value at that point
      auto index = j * width * channels + i * channels;
      float depth = _rayBuffer[index];
      // Validate Depth/Radius and update pointcloud density flag
      if (isDense)
      {
        isDense = !(gz::math::isnan(depth) || std::isinf(depth));
      }

      float intensity = _rayBuffer[index + 1];
      uint16_t ring = j;

      int fieldIndex = 0;

      // Convert spherical coordinates to Cartesian for pointcloud
      // See https://en.wikipedia.org/wiki/Spherical_coordinate_system
      *reinterpret_cast<float *>(msgBufferIndex + this->pointMsg.field(fieldIndex++).offset()) =
        depth * std::cos(inclination) * std::cos(azimuth);

      *reinterpret_cast<float *>(msgBufferIndex + this->pointMsg.field(fieldIndex++).offset()) =
        depth * std::cos(inclination) * std::sin(azimuth);

      *reinterpret_cast<float *>(msgBufferIndex + this->pointMsg.field(fieldIndex++).offset()) =
        depth * std::sin(inclination);

      // Intensity
      *reinterpret_cast<float *>(msgBufferIndex + this->pointMsg.field(fieldIndex++).offset()) =
        intensity;

      // Ring
      *reinterpret_cast<uint16_t *>(msgBufferIndex + this->pointMsg.field(fieldIndex++).offset()) =
        ring;

      // Move the index to the next point.
      msgBufferIndex += this->pointMsg.point_step();

      azimuth += angleStep;
    }
    inclination += verticleAngleStep;
  }
  this->pointMsg.set_is_dense(isDense);
}

}  // namespace sensors
}  // namespace gz
