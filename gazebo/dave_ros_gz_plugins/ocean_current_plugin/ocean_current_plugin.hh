// Copyright (c) 2024 The dave Simulator Authors.
// All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// \file ocean_current_plugin.hh
/// \brief Publishes the ocean current velocity in ROS messages and cxreates a
/// service to alter the flow model in runtime

#ifndef OCEAN_CURRENT_PLUGIN_H_
#define OCEAN_CURRENT_PLUGIN_H_

// #include <dave_gz_ros_plugins/GetCurrentModel.h>
// #include <dave_gz_ros_plugins/GetOriginSphericalCoord.h>
// #include <dave_gz_ros_plugins/SetCurrentDirection.h>
// #include <dave_gz_ros_plugins/SetCurrentModel.h>
// #include <dave_gz_ros_plugins/SetCurrentVelocity.h>
// #include <dave_gz_ros_plugins/SetOriginSphericalCoord.h>
// #include <dave_gz_ros_plugins/SetStratifiedCurrentDirection.h>
// #include <dave_gz_ros_plugins/SetStratifiedCurrentVelocity.h>
// #include <dave_gz_ros_plugins/StratifiedCurrentDatabase.h>
// #include <dave_gz_ros_plugins/StratifiedCurrentVelocity.h>
// #include <dave_gz_world_plugins/ocean_current_world_plugin.h>
// #include <geometry_msgs/TwistStamped.h>
// #include <geometry_msgs/Vector3.h>
// #include <ros/package.h>
// #include <ros/ros.h>

#include <gz/sim/System.hh>
#include <gz/utilis/ImplPtr.hh>

#include <boost/shared_ptr.hpp>
#include <gz/common/Plugin.hh>
#include <gz/physics/World.hh>

#include <map>
#include <memory>
#include <string>

// using namespace gz;
// using namespace sim;
// using namespace systems;

namespace gz
{
namespace sim
{
inline namespace GZ_SIM_VERSION_NAMESPACE  // can remove this line if gazebo.
{
namespace systems
{
namespace dave_simulator_ros
{

class UnderwaterCurrentROSPlugin : public System,
                                   public ISystemConfigure,
                                   public ISystemPreUpdate,
                                   public ISystemPostUpdate
{
  /// \brief Class constructor
public:
  explicit UnderwaterCurrentROSPlugin();

  /// \brief Class destructor
public:
  ~UnderwaterCurrentROSPlugin();

  // Documentation inherited
public:
  void Configure(
    const Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
    EntityComponentManager & _ecm, EventManager & _eventMgr) override;

  // Documentation inherited
public:
  void PreUpdate(const UpdateInfo & _info, EntityComponentManager & _ecm) override;

  // Documentation inherited
public:
  void PostUpdate(const UpdateInfo & _info, const EntityComponentManager & _ecm) override;
}
}  // namespace dave_simulator_ros
}  // namespace systems
}  // namespace GZ_SIM_VERSION_NAMESPACE
}  // namespace sim
}  // namespace gz

// Register plugin
GZ_ADD_PLUGIN(
  UnderwaterCurrentROSPlugin, gz::sim::System, UnderwaterCurrentROSPlugin::ISystemConfigure,
  UnderwaterCurrentROSPlugin::ISystemPostUpdate)

// Add plugin alias so that we can refer to the plugin without the version
// namespace
GZ_ADD_PLUGIN_ALIAS(UnderwaterCurrentROSPlugin, "gz::sim::systems::UnderwaterCurrentROSPlugin")

#endif  // OCEAN_CURRENT_PLUGIN_H_
