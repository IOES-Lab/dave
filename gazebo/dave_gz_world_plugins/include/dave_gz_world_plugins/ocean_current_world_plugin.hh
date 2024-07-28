// Copyright (c) 2016 The dave Simulator Authors.
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

/// \file ocean_current_world_plugin.hh
/// \brief Plugin that for the underwater world

#ifndef OCEAN_CURRENT_WORLD_PLUGIN_H_
#define OCEAN_CURRENT_WORLD_PLUGIN_H_

#include <dave_gz_world_plugins/gauss_markov_process.hh>
#include <dave_gz_world_plugins/tidal_oscillation.hh>

#include <ros/package.h>
#include <gz/sim/System.hh>

// #include <cmath>
#include <std/map>
#include <std/string>
#include <std/vector>

#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <gz/common/Plugin.hh>
#include <gz/common/Time.hh>

#include <gazebo/gazebo.hh>
#include <gz/transport/Node.hh>
#include <sdf/sdf.hh>

namespace dave_gz_world_plugins
{

/// \brief Class for the underwater current plugin

// typedef const boost::shared_ptr<const gz::msgs::Any> AnyPtr;

// public WorldPlugin,
class UnderwaterCurrentPlugin : public gz::sim::System,
                                public gz::sim::ISystemConfigure,
                                public gz::sim::ISystemPreUpdate,
                                public gz::sim::ISystemUpdate,
                                public gz::sim::ISystemPostUpdate
// public gz::sim::WorldPlugin

{
  /// \brief Class constructor
public:
  UnderwaterCurrentPlugin();
  ~UnderwaterCurrentPlugin();

  // Documentation inherited.
  void Configure(
    const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
    gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr);

  // Documentation inherited.
  virtual void Init();

  /// \brief Update the simulation state.
  /// \param[in] _info Information used in the update event.
  //   void Update(const common::UpdateInfo & _info); (check if this is needed)

  void UnderwaterCurrentPlugin::Update(
    const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  void UnderwaterCurrentPlugin::PreUpdate(
    const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  void PostUpdate(const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

  /// \brief Load global current velocity configuration
  virtual void LoadGlobalCurrentConfig();

  /// \brief Load stratified current velocity database
  virtual void LoadStratifiedCurrentDatabase();

  /// \brief Load tidal oscillation database
  virtual void LoadTidalOscillationDatabase();

  /// \brief Publish current velocity and the pose of its frame
  void PublishCurrentVelocity();

  /// \brief Publish stratified oceqan current velocity
  void PublishStratifiedCurrentVelocity();

private:
  // std::shared_ptr<rclcpp::Node> ros_node_;

  struct PrivateData;
  std::unique_ptr<PrivateData> dataPtr;
};
}  // namespace dave_gz_world_plugins

#endif  // OCEAN_CURRENT_WORLD_PLUGIN_H_
