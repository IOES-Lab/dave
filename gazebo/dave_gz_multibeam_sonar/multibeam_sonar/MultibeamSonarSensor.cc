/*

 * Authors:
 *   Helena Moyen helenamoyen@gmail.com
 *   Woen-Sug Choi woensug.choi@gmail.com
 *
 * Copyright (C) 2025 Open Source Robotics Foundation
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

// TODO(hidmic): implement SVD in gazebo?
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/SVD>

#include <gz/common/Console.hh>
#include <gz/common/Event.hh>
#include <gz/common/Profiler.hh>
#include <gz/math/Helpers.hh>

#include <gz/msgs/float_v.pb.h>
#include <gz/msgs/image.pb.h>
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
#include <gz/sensors/SensorTypes.hh>
#include "MultibeamSonarSensor.hh"
#include "sonar_calculation_cuda.cuh"

#include <sys/stat.h>
#include <cv_bridge/cv_bridge.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <marine_acoustic_msgs/msg/ping_info.hpp>
#include <marine_acoustic_msgs/msg/sonar_image_data.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

namespace gz
{
namespace sensors
{
namespace
{

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

//////////////////////////////////////////////////
MultibeamSonarSensor::MultibeamSonarSensor() : dataPtr(new Implementation()) {}

//////////////////////////////////////////////////
MultibeamSonarSensor::~MultibeamSonarSensor()
{
  this->dataPtr->rayConnection.reset();
  this->dataPtr->sceneChangeConnection.reset();
  // CSV log write stream close
  this->dataPtr->writeLog.close();
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
          << "a Multibeam Sonar but found a " << _sdf.TypeStr() << "." << std::endl;
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
          << "multibeam_sonar but it is of '" << type << "' type. Aborting load." << std::endl;
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
  // for csv write logs
  this->dataPtr->writeCounter = 0;
  this->dataPtr->writeNumber = 1;

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

  this->raySensor = _sensor->Scene()->CreateGpuRays(_sensor->Name() + "_ray_sensor");
  if (!this->raySensor)
  {
    gzerr << "Failed to create Ray (GPU Ray) sensor for "
          << "for [" << _sensor->Name() << "] sensor." << std::endl;
    return false;
  }

  // Read ray definition from SDF
  sdf::ElementPtr rayElement = this->sensorSdf->GetElement("ray");
  sdf::ElementPtr sensorElement = this->sensorSdf->GetElement("spec");

  if (!rayElement)
  {
    gzerr << "No beam properties(format of GPU Ray) specified for "
          << "[" << _sensor->Name() << "] sensor" << std::endl;
    return false;
  }

  if (!sensorElement)
  {
    gzerr << "No sonar properties(format of GPU Ray) specified for "
          << "[" << _sensor->Name() << "] sensor" << std::endl;
    return false;
  }

  const bool useDegrees = rayElement->Get("degrees", false).first;
  const gz::math::Angle angleUnit = useDegrees ? GZ_DTOR(1.) : 1.;

  // -------- Assign ray sensor properties
  sdf::ElementPtr rangeElement = rayElement->GetElement("range");
  const double minimumRange = rangeElement->Get<double>("min", 0.1).first;
  gzmsg << "Setting minimum range to " << minimumRange << " m for [" << _sensor->Name()
        << "] sensor." << std::endl;
  this->raySensor->SetNearClipPlane(minimumRange);
  this->maximumRange = rangeElement->Get<double>("max", 5.).first;
  gzmsg << "Setting maximum range to " << this->maximumRange << " m for [" << _sensor->Name()
        << "] sensor." << std::endl;
  this->raySensor->SetFarClipPlane(this->maximumRange);

  // Read sonar properties from model.sdf

  this->verticalFOV = sensorElement->Get<double>("verticalFOV", 10).first;
  gzmsg << "verticalFOV: " << this->verticalFOV << " degrees" << std::endl;
  this->sonarFreq = sensorElement->Get<double>("sonarFreq", 900e3).first;
  gzmsg << "sonarFreq: " << this->sonarFreq << " Hz" << std::endl;

  this->bandwidth = sensorElement->Get<double>("bandwidth", 29.5e6).first;
  gzmsg << "bandwidth: " << this->bandwidth << " Hz" << std::endl;

  this->soundSpeed = sensorElement->Get<double>("soundSpeed", 1500).first;
  gzmsg << "soundSpeed: " << this->soundSpeed << " m/s" << std::endl;

  this->maxDistance = sensorElement->Get<double>("maxDistance", 60).first;
  gzmsg << "maxDistance: " << this->maxDistance << " meters" << std::endl;

  this->sourceLevel = sensorElement->Get<double>("sourceLevel", 220).first;
  gzmsg << "sourceLevel: " << this->sourceLevel << " dB" << std::endl;

  this->raySkips = sensorElement->Get<int>("raySkips", 10).first;
  gzmsg << "raySkips: " << this->raySkips << std::endl;

  this->plotScaler = sensorElement->Get<float>("plotScaler", 10).first;
  gzmsg << "plotScaler: " << this->plotScaler << std::endl;

  this->sensorGain = sensorElement->Get<float>("sensorGain", 0.02).first;
  gzmsg << "sensorGain: " << this->sensorGain << std::endl;

  this->debugFlag = sensorElement->Get<bool>("debugFlag", false).first;
  gzmsg << "Debug: " << this->debugFlag << std::endl;

  this->blazingFlag = sensorElement->Get<bool>("blazingSonarImage", false).first;
  gzmsg << "BlazingSonarImage: " << this->blazingFlag << std::endl;

  this->pointCloudTopicName =
    sensorElement->Get<std::string>("pointCloudTopicName", "point_cloud").first;
  gzmsg << "pointCloudTopicName: " << this->pointCloudTopicName << std::endl;

  this->sonarImageRawTopicName =
    sensorElement->Get<std::string>("sonarImageRawTopicName", "sonar_image_raw").first;
  gzmsg << "sonarImageRawTopicName: " << this->sonarImageRawTopicName << std::endl;

  this->sonarImageTopicName =
    sensorElement->Get<std::string>("sonarImageTopicName", "sonar_image").first;
  gzmsg << "sonarImageTopicName: " << this->sonarImageTopicName << std::endl;

  this->frameName =
    sensorElement->Get<std::string>("frameName", "forward_sonar_optical_link").first;
  gzmsg << "frameName: " << this->frameName << std::endl;

  this->frameId = _sensor->FrameId();
  size_t pos = 0;
  while ((pos = this->frameId.find("::", pos)) != std::string::npos)
  {
    this->frameId.replace(pos, 2, "/");
    pos += 1;
  }
  gzmsg << "frameId: " << this->frameId << std::endl;

  // ROS Initialization

  if (!rclcpp::ok())
  {
    rclcpp::init(0, nullptr);
  }

  this->ros_node_ = std::make_shared<rclcpp::Node>("multibeam_sonar_node");
  this->ros_node_->set_parameters({rclcpp::Parameter("use_sim_time", true)});

  // Create the point cloud publisher
  this->pointPub = this->node.Advertise<gz::msgs::PointCloudPacked>(
    _sensor->Topic() + "/" + this->pointCloudTopicName);
  this->pointFloatPub =
    this->node.Advertise<gz::msgs::Float_V>(_sensor->Topic() + "/point_cloud_float_vector");

  this->sonarImageRawPub =
    this->ros_node_->create_publisher<marine_acoustic_msgs::msg::ProjectedSonarImage>(
      _sensor->Topic() + "/" + this->sonarImageRawTopicName, rclcpp::SystemDefaultsQoS());

  this->sonarImagePub = this->ros_node_->create_publisher<sensor_msgs::msg::Image>(
    _sensor->Topic() + "/" + this->sonarImageTopicName, rclcpp::SystemDefaultsQoS());

  this->normalImagePub = this->ros_node_->create_publisher<sensor_msgs::msg::Image>(
    _sensor->Topic() + "/normal_image", rclcpp::SystemDefaultsQoS());

  // Configure skips
  if (this->raySkips == 0)
  {
    this->raySkips = 1;
    gzmsg << "raySkips was 0, setting to 1." << std::endl;
  }

  this->writeLogFlag = sensorElement->Get<bool>("writeLog", false).first;

  if (this->writeLogFlag)
  {
    this->writeInterval = sensorElement->Get<int>("writeFrameInterval", 10).first;

    RCLCPP_INFO_STREAM(
      this->ros_node_->get_logger(), "Raw data at " << "/SonarRawData_{numbers}.csv"
                                                    << " every " << this->writeInterval
                                                    << " frames");
    RCLCPP_INFO_STREAM(
      this->ros_node_->get_logger(), "Beam angles at SonarRawData_beam_angles.csv");
    RCLCPP_INFO_STREAM(this->ros_node_->get_logger(), "");

    struct stat buffer;
    std::string logfilename("SonarRawData_000001.csv");
    std::string logfilename_angles("SonarRawData_beam_angles.csv");
    if (stat(logfilename.c_str(), &buffer) == 0)
    {
      system("rm SonarRawData*.csv");
    }
    if (stat(logfilename_angles.c_str(), &buffer) == 0)
    {
      system("rm SonarRawData_beam_angles.csv");
    }
  }

  // Mask ranges outside of min/max to +/- inf, as per REP 117
  this->raySensor->SetClamp(false);

  sdf::ElementPtr horizontalElement = rayElement->GetElement("scan")->GetElement("horizontal");
  const double horizAngleMin = horizontalElement->Get<double>("min_angle", -M_PI / 4.0).first;
  const double horizAngleMax = horizontalElement->Get<double>("max_angle", M_PI / 4.0).first;

  sdf::ElementPtr verticalElement = rayElement->GetElement("scan")->GetElement("vertical");
  const double verticalAngleMin = verticalElement->Get<double>("min_angle", -M_PI / 8.0).first;
  const double verticalAngleMax = verticalElement->Get<double>("max_angle", M_PI / 8.0).first;

  const int beamCount = horizontalElement->Get<int>("beams", 256).first;
  const int rayCount = verticalElement->Get<int>("rays", 3).first;

  // Compute FOVs
  this->hFOV = horizAngleMax - horizAngleMin;  // Horizontal Field of View
  gzmsg << "HFOV :" << this->hFOV << std::endl;

  this->vFOV = verticalAngleMax - verticalAngleMin;  // Vertical Field of View
  gzmsg << "VFOV :" << this->vFOV << std::endl;

  // Ensure FOV is always in radians
  if (useDegrees == true)
  {
    this->hFOV = this->hFOV * (M_PI / 180.0);
    this->vFOV = this->vFOV * (M_PI / 180.0);
  }

  // Gazebo debug output
  gzmsg << "Debug Info: FOV Calculations" << std::endl;
  gzmsg << "horizAngleMax: " << horizAngleMax << ", horizAngleMin: " << horizAngleMin
        << ", hFOV (radians): " << this->hFOV << std::endl;
  gzmsg << "verticalAngleMax: " << verticalAngleMax << ", verticalAngleMin: " << verticalAngleMin
        << ", vFOV (radians): " << this->vFOV << std::endl;

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

  this->raySensor->SetAngleMin(horizAngleMin);
  this->raySensor->SetAngleMax(horizAngleMax);
  gzmsg << "Beams Angle Min: " << beamsSphericalFootprint.XMin() << std::endl;
  gzmsg << "Beams Angle Max: " << beamsSphericalFootprint.XMax() << std::endl;
  gzmsg << "V Angle Max: " << beamsSphericalFootprint.YMax() << std::endl;
  gzmsg << "V Angle Min: " << beamsSphericalFootprint.YMin() << std::endl;

  auto horizontalRayCount = beamCount;
  if (horizontalRayCount % 2 == 0)
  {
    ++horizontalRayCount;  // ensure odd
  }
  this->raySensor->SetRayCount(horizontalRayCount);
  this->nBeams = horizontalRayCount;

  this->raySensor->SetVerticalAngleMin(beamsSphericalFootprint.YMin());
  this->raySensor->SetVerticalAngleMax(beamsSphericalFootprint.YMax());
  auto verticalRayCount = rayCount;
  if (verticalRayCount % 2 == 0)
  {
    ++verticalRayCount;  // ensure odd
  }

  // Currently, this->width equals # of beams, and this->height equals # of rays
  // Each beam consists of (elevation,azimuth)=(this->height,1) rays
  // Beam patterns

  this->raySensor->SetVerticalRayCount(verticalRayCount);
  this->nRays = verticalRayCount;

  this->ray_nElevationRays = this->nRays;
  this->ray_nAzimuthRays = 1;
  this->elevation_angles = new float[this->nRays];

  gzmsg << "Horizontal Resolution " << this->raySensor->HorizontalResolution() << " px"
        << std::endl;
  gzmsg << "Vertical Resolution " << this->raySensor->VerticalResolution() << " px" << std::endl;

  gzmsg << "Vertical Range " << this->raySensor->VerticalRangeCount() << " px" << std::endl;
  gzmsg << "Horizontal Range " << this->raySensor->RangeCount() << " px" << std::endl;

  gzmsg << "Vertical Ray " << this->raySensor->RayCount() << " px" << std::endl;
  gzmsg << "Horizontal Ray " << this->raySensor->VerticalRayCount() << " px" << std::endl;

  auto & intrinsics = this->raySensorIntrinsics;
  intrinsics.offset.X(horizAngleMin);
  intrinsics.offset.Y(beamsSphericalFootprint.YMin());
  intrinsics.step.X(this->hFOV / (horizontalRayCount - 1));
  intrinsics.step.Y(beamsSphericalFootprint.YSize() / (verticalRayCount - 1));

  // Pre-compute scan indices covered by beam spherical
  // footprints for speed during scan iteration
  this->beamScanPatches.clear();
  for (const auto & beam : this->beams)
  {
    this->beamScanPatches.push_back(
      AxisAlignedPatch2i{(beam.SphericalFootprint() - intrinsics.offset) / intrinsics.step});
  }

  this->raySensor->SetVisibilityMask(GZ_VISIBILITY_ALL);

  _sensor->AddSensor(this->raySensor);

  // Set the values on the point message.
  this->pointMsg.set_width(this->raySensor->RangeCount());
  this->pointMsg.set_height(this->raySensor->VerticalRangeCount());
  this->pointMsg.set_row_step(this->pointMsg.point_step() * this->pointMsg.width());

  this->rayConnection = this->raySensor->ConnectNewGpuRaysFrame(std::bind(
    &MultibeamSonarSensor::Implementation::OnNewFrame, this, std::placeholders::_1,
    std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

  // Transmission path properties (typical model used here)
  // More sophisticated model by Francois-Garrison model is available
  this->absorption = 0.0354;  // [dB/m]
  this->attenuation = this->absorption * log(10) / 20.0;

  // Range vector
  const float max_T = this->maxDistance * 2.0 / this->soundSpeed;
  float delta_f = 1.0 / max_T;
  const float delta_t = 1.0 / this->bandwidth;
  this->nFreq = ceil(this->bandwidth / delta_f);
  delta_f = this->bandwidth / this->nFreq;
  const int nTime = nFreq;
  this->rangeVector = new float[nTime];
  for (int i = 0; i < nTime; i++)
  {
    this->rangeVector[i] = delta_t * i * this->soundSpeed / 2.0;
  }

  // Print sonar calculation settings
  RCLCPP_INFO_STREAM(this->ros_node_->get_logger(), "");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "==================================================");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "============   SONAR PLUGIN LOADED   =============");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "==================================================");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "============       RAY VERSION       =============");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "==================================================");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "Maximum view range  [m] = " << this->maxDistance);
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(),
    "Distance resolution [m] = " << this->soundSpeed * (1.0 / (this->nFreq * delta_f)));
  RCLCPP_INFO_STREAM(this->ros_node_->get_logger(), "# of Beams = " << this->nBeams);
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "# of Rays / Beam (Elevation, Azimuth) = ("
                                     << this->ray_nElevationRays << ", " << this->ray_nAzimuthRays
                                     << ")");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "Calculation skips (Elevation) = " << this->raySkips);
  RCLCPP_INFO_STREAM(this->ros_node_->get_logger(), "# of Time data / Beam = " << this->nFreq);
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "==================================================");
  RCLCPP_INFO_STREAM(this->ros_node_->get_logger(), "");

  // -- Pre calculations for sonar -- //

  // Hamming window
  gzmsg << "Computing Hamming window for " << this->nFreq << " frequencies." << std::endl;
  this->window = new float[this->nFreq];
  float windowSum = 0;

  for (size_t f = 0; f < this->nFreq; f++)
  {
    this->window[f] = 0.54 - 0.46 * cos(2.0 * M_PI * (f + 1) / this->nFreq);
    windowSum += pow(this->window[f], 2.0);
  }

  gzmsg << "Window sum before normalization: " << windowSum << std::endl;

  for (size_t f = 0; f < this->nFreq; f++)
  {
    this->window[f] = this->window[f] / sqrt(windowSum);
    // gzmsg << "Normalized Window[" << f << "] = " << this->window[f] << std::endl;
  }

  gzmsg << "Hamming window computation complete." << std::endl;

  // Sonar corrector preallocation
  this->beamCorrector = new float *[this->nBeams];
  for (int i = 0; i < this->nBeams; i++)
  {
    this->beamCorrector[i] = new float[this->nBeams];
  }
  this->beamCorrectorSum = 0.0;

  this->constMu = true;
  this->mu = 1e-3;

  return true;
}

//////////////////////////////////////////////////
std::vector<gz::rendering::SensorPtr> MultibeamSonarSensor::RenderingSensors() const
{
  return {this->dataPtr->raySensor};
}

// Simplified function to only process point cloud
void MultibeamSonarSensor::Implementation::OnNewFrame(
  const float * _scan, unsigned int _width, unsigned int _height, unsigned int _channels,
  const std::string & /*_format*/)
{
  // Lock the ray buffer for thread safety
  std::lock_guard<std::mutex> lock(this->rayMutex);

  // Total number of points in the scan
  unsigned int samples = _width * _height * _channels;
  unsigned int rayBufferSize = samples * sizeof(float);

  this->lastMeasurementTime = this->ros_node_->now().seconds();

  // Allocate memory for the ray buffer if not already allocated
  if (!this->rayBuffer)
  {
    this->rayBuffer = new float[samples];
  }

  // Copy the incoming scan data into the ray buffer
  memcpy(this->rayBuffer, _scan, rayBufferSize);

  // Fill point cloud with the ray buffer
  this->FillPointCloudMsg(this->rayBuffer);

  if (this->pointCloudImage.size().width != 0)
  {
    this->ComputeSonarImage();
  }
}

/////////////////////////////////////////////////
void MultibeamSonarSensor::SetScene(gz::rendering::ScenePtr _scene)
{
  // APIs make it possible for the scene pointer to change
  if (this->Scene() != _scene)
  {
    // TODO(anyone) Remove camera from scene
    this->dataPtr->raySensor = nullptr;
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

  if (!this->dataPtr->raySensor)
  {
    gzerr << "Ray (GpuRays) Sensor for Multibeam Sonar doesn't exist.\n";
    return false;
  }

  if (this->dataPtr->pointPub.HasConnections())
  {
    this->dataPtr->publishingPointCloud = true;
  }

  // Generate sensor data
  this->Render();

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
  rclcpp::spin_some(this->dataPtr->ros_node_);

  if (this->dataPtr->publishingPointCloud)
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

    // For the point cloud visualization in gazebo
    // https://github.com/gazebosim/gz-gui/pull/346
    {
      this->AddSequence(this->dataPtr->pointMsg.mutable_header());
      GZ_PROFILE("MultibeamSonarSensor::Update Publish point cloud");
      this->dataPtr->pointPub.Publish(this->dataPtr->pointMsg);
    }
  }
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

  float angleStep = (this->raySensor->AngleMax() - this->raySensor->AngleMin()).Radian() /
                    (this->raySensor->RangeCount() - 1);

  float verticleAngleStep =
    (this->raySensor->VerticalAngleMax() - this->raySensor->VerticalAngleMin()).Radian() /
    (this->raySensor->VerticalRangeCount() - 1);

  // Angles of ray currently processing, azimuth is horizontal, inclination
  // is vertical
  float inclination = this->raySensor->VerticalAngleMin().Radian();

  std::string * msgBuffer = this->pointMsg.mutable_data();
  msgBuffer->resize(this->pointMsg.row_step() * this->pointMsg.height());
  char * msgBufferIndex = msgBuffer->data();
  // Set Pointcloud as dense. Change if invalid points are found.
  bool isDense{true};
  // Iterate over scan and populate point cloud
  for (uint32_t j = 0; j < height; ++j)
  {
    float azimuth = this->raySensor->AngleMin().Radian();

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

  // After filling pointMsg, compute pointCloudImage as well
  this->lock_.lock();
  this->pointCloudImage.create(height, width, CV_32FC1);
  cv::MatIterator_<float> iter_image = this->pointCloudImage.begin<float>();

  bool angles_calculation_flag = false;
  if (this->azimuth_angles.size() == 0)
  {
    angles_calculation_flag = true;
  }

  // Vertical angle min and max
  float elevation_min = this->raySensor->VerticalAngleMin().Radian();
  float elevation_max = this->raySensor->VerticalAngleMax().Radian();
  float elevation_step = verticleAngleStep;

  // Horizontal angle min and max
  float azimuth_min = this->raySensor->AngleMin().Radian();
  float azimuth_max = this->raySensor->AngleMax().Radian();
  float azimuth_step = angleStep;

  for (uint32_t j = 0; j < height; ++j)
  {
    float inclination = elevation_min + j * elevation_step;

    for (uint32_t i = 0; i < width; ++i, ++iter_image)
    {
      float azimuth = azimuth_min + i * azimuth_step;

      // Index in _rayBuffer
      auto index = j * width * channels + i * channels;
      float depth = _rayBuffer[index];

      float range = std::isfinite(depth) ? depth : 100000.0f;
      *iter_image = range;

      // Store azimuth angles on the first row only
      if (angles_calculation_flag && j == 0)
      {
        this->azimuth_angles.push_back(azimuth);
      }

      // Store elevation angles on first column
      if (angles_calculation_flag && i == 0)
      {
        this->elevation_angles[j] = inclination;
      }

      if (!std::isfinite(*iter_image) || std::isnan(*iter_image))
      {
        *iter_image = 100000.0f;
      }
    }
  }
  this->lock_.unlock();
}

cv::Mat MultibeamSonarSensor::Implementation::ComputeNormalImage(cv::Mat & depth)

{
  // filters
  cv::Mat_<float> f1 = (cv::Mat_<float>(3, 3) << 1, 2, 1, 0, 0, 0, -1, -2, -1) / 8;

  cv::Mat_<float> f2 = (cv::Mat_<float>(3, 3) << 1, 0, -1, 2, 0, -2, 1, 0, -1) / 8;

  cv::Mat f1m, f2m;
  cv::flip(f1, f1m, 0);
  cv::flip(f2, f2m, 1);

  cv::Mat n1, n2;
  cv::filter2D(depth, n1, -1, f1m, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
  cv::filter2D(depth, n2, -1, f2m, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);

  cv::Mat no_readings;
  cv::erode(depth == 0, no_readings, cv::Mat(), cv::Point(-1, -1), 2, 1, 1);
  // cv::dilate(no_readings, no_readings, cv::Mat(),
  //            cv::Point(-1, -1), 2, 1, 1);
  n1.setTo(0, no_readings);
  n2.setTo(0, no_readings);

  std::vector<cv::Mat> images(3);
  cv::Mat white = cv::Mat::ones(depth.rows, depth.cols, CV_32FC1);

  // NOTE: with different focal lengths, the expression becomes
  // (-dzx*fy, -dzy*fx, fx*fy)
  images.at(0) = n1;  // for green channel
  images.at(1) = n2;  // for red channel

  // Calculate focal length (?) Not sure if this is the right way;
  // I could not find in the original DAVE the definition of the focal length
  double focal_length = (0.5 * this->pointMsg.width()) / tan(0.5 * this->hFOV);
  images.at(2) = 1.0 / focal_length * depth;  // for blue channel

  cv::Mat normal_image;
  cv::merge(images, normal_image);

  for (int i = 0; i < normal_image.rows; ++i)
  {
    for (int j = 0; j < normal_image.cols; ++j)
    {
      cv::Vec3f & n = normal_image.at<cv::Vec3f>(i, j);
      n = cv::normalize(n);
      float & d = depth.at<float>(i, j);
    }
  }

  return normal_image;
}

// Precalculation of corrector sonar calculation
void MultibeamSonarSensor::Implementation::ComputeCorrector()
{
  double hPixelSize = this->hFOV / (this->pointMsg.width() - 1);

  // Beam culling correction precalculation
  for (size_t beam = 0; beam < this->nBeams; beam++)
  {
    for (size_t beam_other = 0; beam_other < this->nBeams; beam_other++)
    {
      float azimuthBeamPattern = unnormalized_sinc(
        M_PI * 0.884 / hPixelSize *
        sin(this->azimuth_angles[beam] - this->azimuth_angles[beam_other]));
      this->beamCorrector[beam][beam_other] = abs(azimuthBeamPattern);
      this->beamCorrectorSum += pow(azimuthBeamPattern, 2);
    }
  }
  this->beamCorrectorSum = sqrt(this->beamCorrectorSum);
}

// Compute sonar image from point cloud
void MultibeamSonarSensor::Implementation::ComputeSonarImage()

{
  this->lock_.lock();
  cv::Mat normal_image = this->ComputeNormalImage(this->pointCloudImage);
  double vPixelSize = this->vFOV / (this->pointMsg.height() - 1);
  double hPixelSize = this->hFOV / (this->pointMsg.width() - 1);

  if (this->beamCorrectorSum == 0)
  {
    ComputeCorrector();
  }

  if (this->reflectivityImage.rows == 0)
  {
    this->reflectivityImage =
      cv::Mat(this->pointMsg.width(), this->pointMsg.height(), CV_32FC1, cv::Scalar(this->mu));
  }

  auto start = std::chrono::high_resolution_clock::now();

  // ------------------------------------------------//
  // --------      Sonar calculations       -------- //
  // ------------------------------------------------//

  CArray2D P_Beams = NpsGazeboSonar::sonar_calculation_wrapper(
    this->pointCloudImage,        // cv::Mat& depth_image (the point cloud image)
    normal_image,                 // cv::Mat& normal_image
    hPixelSize,                   // hPixelSize
    vPixelSize,                   // vPixelSize
    hFOV,                         // hFOV
    vFOV,                         // VFOV
    hPixelSize,                   // _beam_azimuthAngleWidth
    verticalFOV / 180 * M_PI,     // _beam_elevationAngleWidth
    hPixelSize,                   // _ray_azimuthAngleWidth
    this->elevation_angles,       // _ray_elevationAngles
    vPixelSize * (raySkips + 1),  // _ray_elevationAngleWidth
    this->soundSpeed,             // _soundSpeed
    this->maxDistance,            // _maxDistance
    this->sourceLevel,            // _sourceLevel
    this->nBeams,                 // _nBeams
    this->nRays,                  // _nRays
    this->raySkips,               // _raySkips
    this->sonarFreq,              // _sonarFreq
    this->bandwidth,              // _bandwidth
    this->nFreq,                  // _nFreq
    this->reflectivityImage,      // reflectivity_image
    this->attenuation,            // _attenuation
    this->window,                 // _window
    this->beamCorrector,          // _beamCorrector
    this->beamCorrectorSum,       // _beamCorrectorSum
    this->debugFlag,              // debugFlag
    this->blazingFlag             // _blazingFlag
  );

  // For calc time measure
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

  if (debugFlag)
  {
    RCLCPP_INFO_STREAM(
      this->ros_node_->get_logger(),
      "GPU Sonar Frame Calc Time " << duration.count() / 10000 << "/100 [s]\n");
  }

  // CSV log write stream
  // Each cols corresponds to each beams
  if (this->writeLogFlag)
  {
    this->writeCounter = this->writeCounter + 1;
    if (this->writeCounter == 1 || this->writeCounter % this->writeInterval == 0)
    {
      double time = this->lastMeasurementTime;
      std::stringstream filename;
      filename << "SonarRawData_" << std::setw(6) << std::setfill('0') << this->writeNumber
               << ".csv";
      writeLog.open(filename.str().c_str(), std::ios_base::app);
      filename.clear();
      writeLog << "# Raw Sonar Data Log (Row: beams, Col: time series data)\n";
      writeLog << "# First column is range vector\n";
      writeLog << "#  nBeams : " << this->nBeams << "\n";
      writeLog << "# Simulation time : " << time << "\n";
      for (size_t i = 0; i < P_Beams[0].size(); i++)
      {
        // writing range vector at first column
        writeLog << this->rangeVector[i];
        for (size_t b = 0; b < this->nBeams; b++)
        {
          if (P_Beams[b][i].imag() > 0)
          {
            writeLog << "," << P_Beams[b][i].real() << "+" << P_Beams[b][i].imag() << "i";
          }
          else
          {
            writeLog << "," << P_Beams[b][i].real() << P_Beams[b][i].imag() << "i";
          }
        }
        writeLog << "\n";
      }
      writeLog.close();

      // write beam (azimuth) angles
      if (this->writeNumber == 1)
      {
        std::stringstream filename_angle;
        filename_angle << "SonarRawData_beam_angles.csv";
        writeLog.open(filename_angle.str().c_str(), std::ios_base::app);
        filename_angle.clear();
        writeLog << "# Raw Sonar Data Log \n";
        writeLog << "# Beam (azimuth) angles of rays\n";
        writeLog << "#  nBeams : " << nBeams << "\n";
        writeLog << "# Simulation time : " << time << "\n";
        for (size_t i = 0; i < this->azimuth_angles.size(); i++)
        {
          writeLog << this->azimuth_angles[i] << "\n";
        }
        writeLog.close();
      }

      this->writeNumber = this->writeNumber + 1;
    }
  }

  rclcpp::Time now = this->ros_node_->now();

  this->sonarRawDataMsg.header.frame_id = this->frameId;

  this->sonarRawDataMsg.header.stamp.sec = static_cast<int32_t>(now.seconds());
  this->sonarRawDataMsg.header.stamp.nanosec =
    static_cast<uint32_t>(now.nanoseconds() % 1000000000);

  marine_acoustic_msgs::msg::PingInfo ping_info_msg_;

  ping_info_msg_.frequency = this->sonarFreq;
  ping_info_msg_.sound_speed = this->soundSpeed;
  for (size_t beam = 0; beam < this->nBeams; beam++)
  {
    ping_info_msg_.rx_beamwidths.push_back(
      static_cast<float>(hFOV / floor(this->nBeams * 2.0 - 2.0) * 2.0));
    ping_info_msg_.tx_beamwidths.push_back(static_cast<float>(vFOV));
  }

  this->sonarRawDataMsg.ping_info = ping_info_msg_;

  for (size_t beam = 0; beam < this->nBeams; beam++)
  {
    geometry_msgs::msg::Vector3 beam_direction;
    beam_direction.x = cos(this->azimuth_angles[beam]);
    beam_direction.y = sin(this->azimuth_angles[beam]);
    beam_direction.z = 0.0;
    this->sonarRawDataMsg.beam_directions.push_back(beam_direction);
  }

  std::vector<float> ranges;
  for (size_t i = 0; i < P_Beams[0].size(); i++)
  {
    ranges.push_back(rangeVector[i]);
  }

  this->sonarRawDataMsg.ranges = ranges;
  marine_acoustic_msgs::msg::SonarImageData sonar_image_data;
  sonar_image_data.is_bigendian = false;
  sonar_image_data.dtype = marine_acoustic_msgs::msg::SonarImageData::DTYPE_FLOAT32;
  sonar_image_data.beam_count = this->nBeams;
  // this->sonar_image_raw_msg_.data_size = 1;  // sizeof(float) * nFreq * nBeams;
  std::vector<float> intensities;
  int Intensity[this->nBeams][this->nFreq];

  for (size_t r = 0; r < P_Beams[0].size(); r++)
  {
    for (size_t beam = 0; beam < this->nBeams; beam++)
    {
      float power_dB = 20.0f * log10(std::abs(P_Beams[beam][r]));
      intensities.push_back(power_dB);
    }
  }
  sonar_image_data.data.resize(intensities.size() * sizeof(float));
  memcpy(sonar_image_data.data.data(), intensities.data(), intensities.size() * sizeof(float));
  this->sonarRawDataMsg.image = sonar_image_data;
  this->sonarImageRawPub->publish(this->sonarRawDataMsg);

  // Construct visual sonar image for rqt plot in sensor::image msg format
  cv_bridge::CvImage img_bridge;

  // Generate image of CV_8UC1
  cv::Mat Intensity_image = cv::Mat::zeros(cv::Size(this->nBeams, this->nFreq), CV_8UC1);

  const float rangeMax = this->maxDistance;
  const float rangeRes = ranges[1] - ranges[0];
  const int nEffectiveRanges = ceil(rangeMax / rangeRes);
  const unsigned int radius = Intensity_image.size().height;
  const cv::Point origin(Intensity_image.size().width / 2, Intensity_image.size().height);
  const float binThickness = 2 * ceil(radius / nEffectiveRanges);

  struct BearingEntry
  {
    float begin, center, end;
    BearingEntry(float b, float c, float e) : begin(b), center(c), end(e) { ; }
  };

  std::vector<BearingEntry> angles;
  angles.reserve(this->nBeams);

  for (int b = 0; b < this->nBeams; ++b)
  {
    const float center = this->azimuth_angles[b];
    float begin = 0.0, end = 0.0;
    if (b == 0)
    {
      end = (this->azimuth_angles[b + 1] + center) / 2.0;
      begin = 2 * center - end;
    }
    else if (b == this->nBeams - 1)
    {
      begin = angles[b - 1].end;
      end = 2 * center - begin;
    }
    else
    {
      begin = angles[b - 1].end;
      end = (this->azimuth_angles[b + 1] + center) / 2.0;
    }
    angles.push_back(BearingEntry(begin, center, end));
  }

  const float ThetaShift = 1.5 * M_PI;

  float max_power = 0.0f;

  for (size_t r = 0; r < P_Beams[0].size(); ++r)
  {
    for (size_t b = 0; b < this->nBeams; ++b)
    {
      float power = std::abs(P_Beams[b][r]);
      if (power > max_power)
      {
        max_power = power;
      }
    }
  }

  float max_dB = 20.0f * log10(max_power + 1e-10f);
  float min_dB = max_dB - 60.0f;

  for (int r = 0; r < ranges.size(); ++r)
  {
    if (ranges[r] > rangeMax)
    {
      continue;
    }
    for (int b = 0; b < this->nBeams; ++b)
    {
      const float range = ranges[r];
      const float power_dB = 20.0f * log10(std::abs(P_Beams[this->nBeams - 1 - b][r]) + 1e-10f);
      int intensity = static_cast<int>(255.0f * (power_dB - min_dB) / (max_dB - min_dB));
      const float begin = angles[b].begin + ThetaShift, end = angles[b].end + ThetaShift;
      const float rad = static_cast<float>(radius) * range / rangeMax;
      // Assume angles are in image frame x-right, y-down
      cv::ellipse(
        Intensity_image, origin, cv::Size(rad, rad), 0.0, begin * 180.0 / M_PI, end * 180.0 / M_PI,
        intensity, binThickness);
    }
  }

  cv::Mat Itensity_image_color;
  cv::applyColorMap(Intensity_image, Itensity_image_color, cv::COLORMAP_HOT);

  now = this->ros_node_->now();
  this->sonarImgMsg.header.frame_id = this->frameName;
  this->sonarImgMsg.header.stamp.sec = static_cast<int32_t>(now.seconds());
  this->sonarImgMsg.header.stamp.nanosec = static_cast<uint32_t>(now.nanoseconds() % 1000000000);

  img_bridge = cv_bridge::CvImage(
    this->sonarImgMsg.header, sensor_msgs::image_encodings::BGR8, Itensity_image_color);
  // from cv_bridge to sensor_msgs::Image
  img_bridge.toImageMsg(this->sonarImgMsg);
  this->sonarImagePub->publish(this->sonarImgMsg);

  // ---------------------------------------- End of sonar calculation

  now = this->ros_node_->now();
  this->normalImgMsg.header.frame_id = this->frameName;
  this->normalImgMsg.header.stamp.sec = static_cast<int32_t>(now.seconds());
  this->normalImgMsg.header.stamp.nanosec = static_cast<uint32_t>(now.nanoseconds() % 1000000000);

  cv::Mat normal_image8;
  normal_image.convertTo(normal_image8, CV_8UC3, 255.0);
  img_bridge = cv_bridge::CvImage(
    this->normalImgMsg.header, sensor_msgs::image_encodings::RGB8, normal_image8);
  img_bridge.toImageMsg(this->normalImgMsg);
  // from cv_bridge to sensor_msgs::Image
  this->normalImagePub->publish(this->normalImgMsg);

  this->lock_.unlock();
}

}  // namespace sensors
}  // namespace gz
