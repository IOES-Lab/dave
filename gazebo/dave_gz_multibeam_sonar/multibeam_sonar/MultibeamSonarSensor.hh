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

#ifndef GZ_SENSORS_MULTIBEAMSONAR_HH_
#define GZ_SENSORS_MULTIBEAMSONAR_HH_

#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>

#include <gz/math/Pose3.hh>
#include <gz/math/Vector3.hh>
#include <gz/sensors/EnvironmentalData.hh>
#include <gz/sensors/RenderingSensor.hh>

namespace gz
{
namespace sensors
{
/// \brief Kinematic state for an entity in the world.
///
/// All quantities are defined w.r.t. the world frame.
struct EntityKinematicState
{
  gz::math::Pose3d pose;
  gz::math::Vector3d linearVelocity;
  gz::math::Vector3d angularVelocity;
};

/// \brief Kinematic state for all entities in the world.
using WorldKinematicState = std::unordered_map<uint64_t, EntityKinematicState>;

/// \brief Kinematic state for all entities in the world.
struct WorldState
{
  WorldKinematicState kinematics;
  gz::math::SphericalCoordinates origin;
};

class MultibeamSonarSensor : public gz::sensors::RenderingSensor
{
public:
  MultibeamSonarSensor();

public:
  ~MultibeamSonarSensor();

  /// Inherits documentation from parent class
public:
  virtual bool Load(const sdf::Sensor & _sdf) override;

  /// Inherits documentation from parent class
public:
  virtual bool Load(sdf::ElementPtr _sdf) override;

  /// Inherits documentation from parent class
public:
  virtual bool Update(const std::chrono::steady_clock::duration & _now) override;

  /// Perform any sensor updates after the rendering pass
public:
  virtual void PostUpdate(const std::chrono::steady_clock::duration & _now);

  /// Inherits documentation from parent class
public:
  void SetScene(gz::rendering::ScenePtr _scene) override;

  /// \brief Set this sensor's entity ID (for world state lookup).
public:
  void SetEntity(uint64_t entity);

  /// \brief Set world `_state` to support DVL water and bottom-tracking.
public:
  void SetWorldState(const WorldState & _state);

  /// \brief Set environmental `_data` to support DVL water-tracking.
public:
  void SetEnvironmentalData(const EnvironmentalData & _data);

  /// \brief Inherits documentation from parent class
public:
  virtual bool HasConnections() const override;

  /// \brief Yield rendering sensors that underpin the implementation.
  ///
  /// \internal
public:
  std::vector<gz::rendering::SensorPtr> RenderingSensors() const;

private:
  class Implementation;

private:
  std::unique_ptr<Implementation> dataPtr;
};

}  // namespace sensors
}  // namespace gz

#endif  // GZ_SENSORS_DOPPLERVELOCITYLOG_HH_
