#ifndef DAVE_GZ_MODEL_PLUGINS__OCEANCURRENTMODELPLUGIN_HH_
#define DAVE_GZ_MODEL_PLUGINS__OCEANCURRENTMODELPLUGIN_HH_

#include "dave_gz_world_plugins/gauss_markov_process.hh"
#include "dave_gz_world_plugins/tidal_oscillation.hh"
#include "dave_interfaces/msg/Stratified_Current_Database.hpp"
#include "dave_interfaces/msg/Stratified_Current_Velocity.hpp"

// OceanCurrentModelPlugin

#include <gz/sim/System.hh>
// #include <gz/utilise/ImplPtr.hh>
#include <gz/physics/World.hh>

#include <chrono>
#include <map>
#include <memory>
#include <string>

#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <rclcpp/rclcpp.hpp>

#include <gz/transport/Node.hh>
#include <sdf/sdf.hh>

namespace dave_gz_model_plugins
{
class OceanCurrentModelPlugin : public gz::sim::System,
                                public gz::sim::ISystemConfigure,
                                public gz::sim::ISystemUpdate,
                                public gz::sim::ISystemPostUpdate
{
public:
  OceanCurrentModelPlugin();
  ~OceanCurrentModelPlugin() override;

  // ----------------------------------------------

  void Configure(
    const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
    gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr);

  void LoadCurrentVelocityParams(sdf::ElementPtr _sdf, gz::sim::EntityComponentManager & _ecm);

  void UpdateDatabase(
    const std::shared_ptr<const dave_interfaces::msg::StratifiedCurrentDatabase> & _msg);

  gz::sim::Entity GetModelEntity(
    const std::string & modelName, gz::sim::EntityComponentManager & ecm);

  gz::math::Pose3d GetModelPose(
    const gz::sim::Entity & modelEntity, gz::sim::EntityComponentManager & ecm);

  // ----------------------------------------------

  void Update(const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  void CalculateOceanCurrent(double vehicleDepth);

  // ----------------------------------------------

  // Function called after the simulation state updates
  void PostUpdate(const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

  void PublishCurrentVelocity(const gz::sim::UpdateInfo & _info);

private:
  std::shared_ptr<rclcpp::Node> ros_node_;
  struct PrivateData;
  std::unique_ptr<PrivateData> dataPtr;
};
}  // namespace dave_gz_model_plugins

#endif  // DAVE_GZ_MODEL_PLUGINS__OCEANCURRENTMODELPLUGIN_HH_
