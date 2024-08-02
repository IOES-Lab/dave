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

// #include <dave_ros_gz_plugins/GetCurrentModel.hh>
// #include <dave_ros_gz_plugins/GetOriginSphericalCoord.hh>
// #include <dave_ros_gz_plugins/SetCurrentDirection.hh>
// #include <dave_ros_gz_plugins/SetCurrentModel.hh>
// #include <dave_ros_gz_plugins/SetCurrentVelocity.hh>
// #include <dave_ros_gz_plugins/SetOriginSphericalCoord.hh>
// #include <dave_ros_gz_plugins/SetStratifiedCurrentDirection.hh>
// #include <dave_ros_gz_plugins/SetStratifiedCurrentVelocity.hh>
// #include <dave_ros_gz_plugins/StratifiedCurrentDatabase.hh>
// #include <dave_ros_gz_plugins/StratifiedCurrentVelocity.hh>

#include <gz/sim/System.hh>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/service.hpp>

#include <map>
#include <memory>
#include <string>

#include "dave_interfaces/srv/Get_Current_Model.hpp"
#include "dave_interfaces/srv/Get_Origin_Spherical_Coord.hpp"
#include "dave_interfaces/srv/Set_Current_Direction.hpp"
#include "dave_interfaces/srv/Set_Current_Model.hpp"
#include "dave_interfaces/srv/Set_Current_Velocity.hpp"
#include "dave_interfaces/srv/Set_Origin_Spherical_Coord.hpp"
#include "dave_interfaces/srv/Set_Stratified_Current_Direction.hpp"
#include "dave_interfaces/srv/Set_Stratified_Current_Velocity.hpp"
// #include "dave_interfaces/srv/Stratified_Current_Database.hpp"
// #include "dave_interfaces/srv/Stratified_Current_Velocity.hpp"

namespace dave_ros_gz_plugins
{
class UnderwaterCurrentROSPlugin : public gz::sim::System,
                                   public gz::sim::ISystemConfigure,
                                   public gz::sim::ISystemPreUpdate,
                                   public gz::sim::ISystemPostUpdate
{
public:
  UnderwaterCurrentROSPlugin();
  ~UnderwaterCurrentROSPlugin();

  // Documentation inherited
  void Configure(
    const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
    gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr);

  void Update(const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

  // Documentation inherited
  void PreUpdate(
    const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm) override;

  // Documentation inherited
  void PostUpdate(
    const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm) override;

  void UpdateHorzAngle(
    const std::shared_ptr<dave_interfaces::srv::SetCurrentDirection::Request> _req,
    std::shared_ptr<dave_interfaces::srv::SetCurrentDirection::Response> _res);

  bool UpdateStratHorzAngle(
    const std::shared_ptr<dave_interfaces::srv::SetStratifiedCurrentDirection::Request> _req,
    std::shared_ptr<dave_interfaces::srv::SetStratifiedCurrentDirection::Response> _res);

  bool UpdateVertAngle(
    const std::shared_ptr<dave_interfaces::srv::SetCurrentDirection::Request> _req,
    std::shared_ptr<dave_interfaces::srv::SetCurrentDirection::Response> _res);

  bool UpdateStratVertAngle(
    const std::shared_ptr<dave_interfaces::srv::SetStratifiedCurrentDirection::Request> _req,
    std::shared_ptr<dave_interfaces::srv::SetStratifiedCurrentDirection::Response> _res);

  bool UpdateCurrentVelocity(
    const std::shared_ptr<dave_interfaces::srv::SetCurrentVelocity::Request> _req,
    std::shared_ptr<dave_interfaces::srv::SetCurrentVelocity::Response> _res);

  bool UpdateStratCurrentVelocity(
    const std::shared_ptr<dave_interfaces::srv::SetStratifiedCurrentVelocity::Request> _req,
    std::shared_ptr<dave_interfaces::srv::SetStratifiedCurrentVelocity::Response> _res);

  bool GetCurrentVelocityModel(
    const std::shared_ptr<dave_interfaces::srv::GetCurrentModel::Request> _req,
    std::shared_ptr<dave_interfaces::srv::GetCurrentModel::Response> _res);

  bool GetCurrentHorzAngleModel(
    const std::shared_ptr<dave_interfaces::srv::GetCurrentModel::Request> _req,
    std::shared_ptr<dave_interfaces::srv::GetCurrentModel::Response> _res);

  bool GetCurrentVertAngleModel(
    const std::shared_ptr<dave_interfaces::srv::GetCurrentModel::Request> _req,
    std::shared_ptr<dave_interfaces::srv::GetCurrentModel::Response> _res);

  bool UpdateCurrentVelocityModel(
    const std::shared_ptr<dave_interfaces::srv::SetCurrentModel::Request> _req,
    std::shared_ptr<dave_interfaces::srv::SetCurrentModel::Response> _res);

  bool UpdateCurrentHorzAngleModel(
    const std::shared_ptr<dave_interfaces::srv::SetCurrentModel::Request> _req,
    std::shared_ptr<dave_interfaces::srv::SetCurrentModel::Response> _res);

  bool UpdateCurrentVertAngleModel(
    const std::shared_ptr<dave_interfaces::srv::SetCurrentModel::Request> _req,
    std::shared_ptr<dave_interfaces::srv::SetCurrentModel::Response> _res);

private:
  struct PrivateData;
  std::unique_ptr<PrivateData> dataPtr;
};
}  // namespace dave_ros_gz_plugins

#endif  // OCEAN_CURRENT_PLUGIN_H_
