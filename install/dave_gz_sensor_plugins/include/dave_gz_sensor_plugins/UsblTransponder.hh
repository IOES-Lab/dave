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

#ifndef DAVE_GZ_SENSOR_PLUGINS__USBLTRANSPONDER_HH_
#define DAVE_GZ_SENSOR_PLUGINS__USBLTRANSPONDER_HH_

#include <memory>
#include <string>

#include <gz/sim/System.hh>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64.hpp>
#include <std_msgs/msg/string.hpp>
#include "dave_interfaces/msg/usbl_command.hpp"
#include "dave_interfaces/msg/usbl_response.hpp"

namespace dave_gz_sensor_plugins

{
class UsblTransponder : public gz::sim::System,
                        public gz::sim::ISystemConfigure,
                        public gz::sim::ISystemPostUpdate
{
public:
  UsblTransponder();
  ~UsblTransponder() override = default;

  void Configure(
    const gz::sim::Entity & entity, const std::shared_ptr<const sdf::Element> & sdf,
    gz::sim::EntityComponentManager & ecm, gz::sim::EventManager & eventMgr) override;

  void PostUpdate(
    const gz::sim::UpdateInfo & info, const gz::sim::EntityComponentManager & ecm) override;

  void sendLocation();
  void iisRosCallback(const std_msgs::msg::String::SharedPtr msg);
  void temperatureRosCallback(const std_msgs::msg::Float64::SharedPtr msg);
  void cisRosCallback(const std_msgs::msg::String::SharedPtr msg);
  void commandRosCallback(const dave_interfaces::msg::UsblCommand msg);
  void sendPing();
  void interrogationModeCallback(const std_msgs::msg::String::SharedPtr msg);

private:
  std::shared_ptr<rclcpp::Node> ros_node_;

  struct PrivateData;
  std::unique_ptr<PrivateData> dataPtr;
};
}  // namespace dave_gz_sensor_plugins

#endif  // DAVE_GZ_SENSOR_PLUGINS__USBLTRANSPONDER_HH_
