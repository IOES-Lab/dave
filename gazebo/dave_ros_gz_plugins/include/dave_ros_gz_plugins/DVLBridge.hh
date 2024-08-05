/*
 * Copyright (C) 2024 Rakesh Vivekanandan
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

#ifndef DAVE_ROS_GZ_PLUGINS__DVLBRIDGE_HH_
#define DAVE_ROS_GZ_PLUGINS__DVLBRIDGE_HH_

#include <memory>

#include <gz/msgs/dvl_velocity_tracking.pb.h>
#include <gz/sim/System.hh>
#include <rclcpp/rclcpp.hpp>
#include "dave_interfaces/msg/dvl.hpp"
#include "dave_interfaces/msg/dvl_beam.hpp"

namespace dave_ros_gz_plugins

{
class DVLBridge : public gz::sim::System,
                  public gz::sim::ISystemConfigure,
                  public gz::sim::ISystemPostUpdate
{
public:
  DVLBridge();
  ~DVLBridge() override = default;

  void Configure(
    const gz::sim::Entity & entity, const std::shared_ptr<const sdf::Element> & sdf,
    gz::sim::EntityComponentManager & ecm, gz::sim::EventManager & eventMgr) override;

  void PostUpdate(
    const gz::sim::UpdateInfo & info, const gz::sim::EntityComponentManager & ecm) override;

  void receiveGazeboCallback(const gz::msgs::DVLVelocityTracking & dvl_velocity_tracking);

private:
  std::shared_ptr<rclcpp::Node> ros_node_;

  struct PrivateData;
  enum DVLType
  {
    DVL_TYPE_UNSPECIFIED = 0,
    DVL_TYPE_PISTON = 1,
    DVL_TYPE_PHASED_ARRAY = 2
  };
  enum DVLTargetType
  {
    DVL_TARGET_UNSPECIFIED = 0,
    DVL_TARGET_BOTTOM = 1,
    DVL_TARGET_WATER = 2
  };
  enum ReferenceType
  {
    DVL_REFERENCE_UNSPECIFIED = 0,
    DVL_REFERENCE_EARTH = 1,
    DVL_REFERENCE_SHIP = 2
  };
  std::unique_ptr<PrivateData> dataPtr;
};
}  // namespace dave_ros_gz_plugins

#endif  // DAVE_ROS_GZ_PLUGINS__DVLBRIDGE_HH_
