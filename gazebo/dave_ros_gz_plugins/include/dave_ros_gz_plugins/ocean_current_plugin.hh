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

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <dave_ros_gz_plugins/GetCurrentModel.hh>
#include <dave_ros_gz_plugins/GetOriginSphericalCoord.hh>
#include <dave_ros_gz_plugins/SetCurrentDirection.hh>
#include <dave_ros_gz_plugins/SetCurrentModel.hh>
#include <dave_ros_gz_plugins/SetCurrentVelocity.hh>
#include <dave_ros_gz_plugins/SetOriginSphericalCoord.hh>
#include <dave_ros_gz_plugins/SetStratifiedCurrentDirection.hh>
#include <dave_ros_gz_plugins/SetStratifiedCurrentVelocity.hh>
#include <dave_ros_gz_plugins/StratifiedCurrentDatabase.hh>
#include <dave_ros_gz_plugins/StratifiedCurrentVelocity.hh>
#include <geometry_msgs/TwistStamped.hh>
#include <geometry_msgs/Vector3.hh>

#include <gz/sim/System.hh>
#include <gz/utilise/ImplPtr.hh>

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
}  // namespace sim
}  // namespace gz

// Register plugin
GZ_ADD_PLUGIN(
  UnderwaterCurrentROSPlugin, gz::sim::System, UnderwaterCurrentROSPlugin::ISystemConfigure,
  UnderwaterCurrentROSPlugin::ISystemPostUpdate)

#endif  // OCEAN_CURRENT_PLUGIN_H_
