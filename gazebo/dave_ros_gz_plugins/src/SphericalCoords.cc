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
 */

#include <iostream>

#include <gz/common/Console.hh>
#include <gz/math/SphericalCoordinates.hh>
#include <gz/sim/World.hh>
#include <rclcpp/rclcpp.hpp>
#include "gz/plugin/Register.hh"
#include "gz/sim/components/World.hh"

#include "dave_ros_gz_plugins/SphericalCoords.hh"

GZ_ADD_PLUGIN(
  dave_ros_gz_plugins::SphericalCoords, gz::sim::System,
  dave_ros_gz_plugins::SphericalCoords::ISystemConfigure,
  dave_ros_gz_plugins::SphericalCoords::ISystemPostUpdate)

namespace dave_ros_gz_plugins
{

struct SphericalCoords::PrivateData
{
  // Add any private data members here.
  std::string ns;
  gz::sim::EntityComponentManager * ecm;
  gz::sim::World world;
  rclcpp::Service<dave_interfaces::srv::GetOriginSphericalCoord>::SharedPtr getOriginSrv;
  rclcpp::Service<dave_interfaces::srv::SetOriginSphericalCoord>::SharedPtr setOriginSrv;
  rclcpp::Service<dave_interfaces::srv::TransformToSphericalCoord>::SharedPtr transformToSrv;
  rclcpp::Service<dave_interfaces::srv::TransformFromSphericalCoord>::SharedPtr transformFromSrv;
};

SphericalCoords::SphericalCoords() : dataPtr(std::make_unique<PrivateData>()) {}

void SphericalCoords::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventManager)
{
  gzdbg << "dave_ros_gz_plugins::SphericalCoords::Configure on entity: " << _entity << std::endl;

  if (!rclcpp::ok())
  {
    rclcpp::init(0, nullptr);
  }

  this->ros_node_ = std::make_shared<rclcpp::Node>("sc_node");

  // auto model = gz::sim::Model(_entity);
  // this->dataPtr->model = model;
  // this->dataPtr->modelName = model.Name(_ecm);
  this->dataPtr->ecm = &_ecm;

  auto worldEntity = _ecm.EntityByComponents(gz::sim::components::World());
  this->dataPtr->world = gz::sim::World(worldEntity);

  auto coordsOpt = this->dataPtr->world.SphericalCoordinates(_ecm);

  if (coordsOpt.has_value())
  {
    auto coords = coordsOpt.value();
    gzmsg << "Spherical coordinates reference=" << std::endl
          << "\t- Latitude [degrees]=" << coords.LatitudeReference().Degree() << std::endl
          << "\t- Longitude [degrees]=" << coords.LongitudeReference().Degree() << std::endl
          << "\t- Altitude [m]=" << coords.ElevationReference() << std::endl
          << "\t- Heading [degrees] =" << coords.HeadingOffset().Degree() << std::endl;
  }
  else
  {
    gzmsg << "Spherical coordinates not available." << std::endl;
  }

  this->dataPtr->getOriginSrv =
    this->ros_node_->create_service<dave_interfaces::srv::GetOriginSphericalCoord>(
      "/gz/get_origin_spherical_coordinates", std::bind(
                                                &SphericalCoords::GetOriginSphericalCoord, this,
                                                std::placeholders::_1, std::placeholders::_2));

  this->dataPtr->setOriginSrv =
    this->ros_node_->create_service<dave_interfaces::srv::SetOriginSphericalCoord>(
      "/gz/set_origin_spherical_coordinates", std::bind(
                                                &SphericalCoords::SetOriginSphericalCoord, this,
                                                std::placeholders::_1, std::placeholders::_2));

  this->dataPtr->transformToSrv =
    this->ros_node_->create_service<dave_interfaces::srv::TransformToSphericalCoord>(
      "/gz/transform_to_spherical_coordinates", std::bind(
                                                  &SphericalCoords::TransformToSphericalCoord, this,
                                                  std::placeholders::_1, std::placeholders::_2));

  this->dataPtr->transformFromSrv =
    this->ros_node_->create_service<dave_interfaces::srv::TransformFromSphericalCoord>(
      "/gz/transform_from_spherical_coordinates",
      std::bind(
        &SphericalCoords::TransformFromSphericalCoord, this, std::placeholders::_1,
        std::placeholders::_2));
}

bool SphericalCoords::TransformFromSphericalCoord(
  const std::shared_ptr<dave_interfaces::srv::TransformFromSphericalCoord::Request> request,
  std::shared_ptr<dave_interfaces::srv::TransformFromSphericalCoord::Response> response)
{
  gz::math::Angle lat, lon;
  lat.SetDegree(request->latitude_deg);
  lon.SetDegree(request->longitude_deg);
  auto scVec = gz::math::CoordinateVector3::Spherical(lat, lon, request->altitude);

  gzmsg << "Called FROM and latitude: " << request->latitude_deg << std::endl;

  auto coords = this->dataPtr->world.SphericalCoordinates(*this->dataPtr->ecm);
  gz::math::Vector3d cartVec = gz::math::Vector3d::Zero;
  auto cartOpt = coords->LocalFromSphericalPosition(scVec);
  if (cartOpt && cartOpt->AsMetricVector())
  {
    cartVec = *cartOpt->AsMetricVector();
  }

  response->output.x = cartVec.X();
  response->output.y = cartVec.Y();
  response->output.z = cartVec.Z();

  return true;
}

bool SphericalCoords::TransformToSphericalCoord(
  const std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord::Request> request,
  std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord::Response> response)
{
  auto cartVec =
    gz::math::CoordinateVector3::Metric(request->input.x, request->input.y, request->input.z);

  gzmsg << "Called TO and X: " << request->input.x << std::endl;

  auto coords = this->dataPtr->world.SphericalCoordinates(*this->dataPtr->ecm);
  auto scOpt = coords->SphericalFromLocalPosition(cartVec);

  response->latitude_deg = (scOpt && scOpt->Lat()) ? scOpt->Lat()->Degree() : 0.0;
  response->longitude_deg = (scOpt && scOpt->Lon()) ? scOpt->Lon()->Degree() : 0.0;
  response->altitude = (scOpt && scOpt->Z()) ? *scOpt->Z() : 0.0;

  return true;
}

bool SphericalCoords::GetOriginSphericalCoord(
  const std::shared_ptr<dave_interfaces::srv::GetOriginSphericalCoord::Request> request,
  std::shared_ptr<dave_interfaces::srv::GetOriginSphericalCoord::Response> response)
{
  response->latitude_deg =
    this->dataPtr->world.SphericalCoordinates(*this->dataPtr->ecm)->LatitudeReference().Degree();
  response->longitude_deg =
    this->dataPtr->world.SphericalCoordinates(*this->dataPtr->ecm)->LongitudeReference().Degree();
  response->altitude =
    this->dataPtr->world.SphericalCoordinates(*this->dataPtr->ecm)->ElevationReference();

  gzmsg << "Called Get" << std::endl;
  return true;
}

bool SphericalCoords::SetOriginSphericalCoord(
  const std::shared_ptr<dave_interfaces::srv::SetOriginSphericalCoord::Request> request,
  std::shared_ptr<dave_interfaces::srv::SetOriginSphericalCoord::Response> response)
{
  auto optionalSphericalCoords = this->dataPtr->world.SphericalCoordinates(*this->dataPtr->ecm);

  if (!optionalSphericalCoords.has_value())
  {
    gzmsg << "Failed to get spherical coordinates" << std::endl;
    response->success = false;
    return false;
  }

  auto sphericalCoords = optionalSphericalCoords.value();

  // gzmsg << "Current Spherical Coordinates Before Setting: "
  //       << "Latitude: " << sphericalCoords.LatitudeReference().Degree()
  //       << ", Longitude: " << sphericalCoords.LongitudeReference().Degree()
  //       << ", Altitude: " << sphericalCoords.ElevationReference() << std::endl;

  gz::math::Angle angle;

  angle.SetDegree(request->latitude_deg);
  sphericalCoords.SetLatitudeReference(angle);

  angle.SetDegree(request->longitude_deg);
  sphericalCoords.SetLongitudeReference(angle);

  sphericalCoords.SetElevationReference(request->altitude);

  // Apply the modified spherical coordinates back to the world
  this->dataPtr->world.SetSphericalCoordinates(*this->dataPtr->ecm, sphericalCoords);

  response->success = true;

  auto newSphericalCoords = this->dataPtr->world.SphericalCoordinates(*this->dataPtr->ecm);

  if (newSphericalCoords.has_value())
  {
    gzmsg << "Current Spherical Coordinates after Setting: "
          << "Latitude: " << newSphericalCoords->LatitudeReference().Degree()
          << ", Longitude: " << newSphericalCoords->LongitudeReference().Degree()
          << ", Altitude: " << newSphericalCoords->ElevationReference() << std::endl;
  }
  else
  {
    gzmsg << "Failed to get new spherical coordinates" << std::endl;
  }

  return true;
}

void SphericalCoords::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  if (!_info.paused)
  {
    rclcpp::spin_some(this->ros_node_);

    if (_info.iterations % 1000 == 0)
    {
      gzmsg << "dave_ros_gz_plugins::SphericalCoords::PostUpdate" << std::endl;
    }
  }
}

}  // namespace dave_ros_gz_plugins
