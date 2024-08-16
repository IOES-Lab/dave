#ifndef DAVE_ROS_GZ_PLUGINS__OCEAN_CURRENT_PLUGIN_HH_
#define DAVE_ROS_GZ_PLUGINS__OCEAN_CURRENT_PLUGIN_HH_

#include <gz/sim/System.hh>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/service.hpp>

#include <map>
#include <memory>
#include <string>

#include "dave_gz_world_plugins/ocean_current_world_plugin.hh"
#include "dave_interfaces/msg/Stratified_Current_Database.hpp"
#include "dave_interfaces/msg/Stratified_Current_Velocity.hpp"
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
                                   public gz::sim::ISystemPostUpdate
{
public:
  UnderwaterCurrentROSPlugin();
  ~UnderwaterCurrentROSPlugin();

  // Documentation inherited
  void Configure(
    const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
    gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr);

  // void Update(const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

  // // Documentation inherited
  // void PreUpdate(
  //   const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm) override;

  // Documentation inherited
  void PostUpdate(const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

  bool UpdateHorzAngle(
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

#endif  // DAVE_ROS_GZ_PLUGINS__OCEAN_CURRENT_PLUGIN_HH_