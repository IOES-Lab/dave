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

#ifndef DAVE_ROS_GZ_PLUGINS__SPHERICALCOORDS_HH_
#define DAVE_ROS_GZ_PLUGINS__SPHERICALCOORDS_HH_

#include <memory>

#include <gz/sim/System.hh>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp/service.hpp>
#include "dave_interfaces/srv/get_origin_spherical_coord.hpp"
#include "dave_interfaces/srv/set_origin_spherical_coord.hpp"
#include "dave_interfaces/srv/transform_from_spherical_coord.hpp"
#include "dave_interfaces/srv/transform_to_spherical_coord.hpp"

namespace dave_ros_gz_plugins

{
class SphericalCoords : public gz::sim::System,
                        public gz::sim::ISystemConfigure,
                        public gz::sim::ISystemPostUpdate
{
public:
  SphericalCoords();
  ~SphericalCoords() override = default;

  void Configure(
    const gz::sim::Entity & entity, const std::shared_ptr<const sdf::Element> & sdf,
    gz::sim::EntityComponentManager & ecm, gz::sim::EventManager & eventMgr) override;

  void PostUpdate(
    const gz::sim::UpdateInfo & info, const gz::sim::EntityComponentManager & ecm) override;

  bool GetOriginSphericalCoord(
    const std::shared_ptr<dave_interfaces::srv::GetOriginSphericalCoord::Request> request,
    std::shared_ptr<dave_interfaces::srv::GetOriginSphericalCoord::Response> response);

  bool SetOriginSphericalCoord(
    const std::shared_ptr<dave_interfaces::srv::SetOriginSphericalCoord::Request> request,
    std::shared_ptr<dave_interfaces::srv::SetOriginSphericalCoord::Response> response);

  bool TransformToSphericalCoord(
    const std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord::Request> request,
    std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord::Response> response);

  bool TransformFromSphericalCoord(
    const std::shared_ptr<dave_interfaces::srv::TransformFromSphericalCoord::Request> request,
    std::shared_ptr<dave_interfaces::srv::TransformFromSphericalCoord::Response> response);

private:
  std::shared_ptr<rclcpp::Node> ros_node_;

  struct PrivateData;
  std::unique_ptr<PrivateData> dataPtr;
};
}  // namespace dave_ros_gz_plugins

#endif  // DAVE_ROS_GZ_PLUGINS__SPHERICALCOORDS_HH_
