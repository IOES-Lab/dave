#ifndef DAVE_GZ_MODEL_PLUGINS__OCEAN_CURRENT_MODEL_PLUGIN_HH_
#define DAVE_GZ_MODEL_PLUGINS__OCEAN_CURRENT_MODEL_PLUGIN_HH_

#include "dave_gz_world_plugins/gauss_markov_process.hh"
#include "dave_gz_world_plugins/tidal_oscillation.h"

#include <gz/sim/System.hh>
// #include <gz/utilise/ImplPtr.hh>
#include <gz/physics/World.hh>

#include <chrono>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <gz/transport/Node.hh>
#include <map>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <sdf/sdf.hh>
#include <string>
#include "dave_interfaces/msg/Stratified_Current_Database.hpp"
#include "dave_interfaces/msg/Stratified_Current_Velocity.hpp"

namespace dave_gz_model_plugins
{
class TransientCurrentPlugin : public gz::sim::System,
                               public gz::sim::ISystemConfigure,
                               public gz::sim::ISystemPreUpdate,
                               public gz::sim::ISystemUpdate,
                               public gz::sim::ISystemPostUpdate
{
public:
  TransientCurrentPlugin();   // constructor
  ~TransientCurrentPlugin();  // Destructor

  // Configure the plugin with necessary entities and component managers
  void Configure(
    const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
    gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr);

  // Function called before the simulation state updates
  void PreUpdate(const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  void Update(const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  // Function called after the simulation state updates
  void PostUpdate(const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

  void TransientCurrentPlugin::LoadCurrentVelocityParams(
    sdf::ElementPtr _sdf, gz::sim::EntityComponentManager & _ecm);

  gz::math::Pose3d GetModelPose(
    const gz::sim::Entity & modelEntity, gz::sim::EntityComponentManager & ecm);

  gz::sim::Entity GetModelEntity(
    const std::string & modelName, gz::sim::EntityComponentManager & ecm);

  /// \brief Check if an entity is enabled or not.
  /// \param[in] _entity Target entity
  /// \param[in] _ecm Entity component manager
  /// \return True if buoyancy should be applied.
  // bool IsEnabled(gz::sim::Entity & _entity, gz::sim::EntityComponentManager & _ecm) const;

  // Initialize any necessary states before the plugin starts
  virtual void Init();

  /// \brief ROS helper function that processes messages
  void databaseSubThread();

  /// \brief Calculate ocean current using database and vehicle state
  void CalculateOceanCurrent(double vehicleDepth);

  void Gauss_Markov_process_initialize();

  /// \brief Convey model state from gazebo topic to outside
  void UpdateDatabase();

  /// \brief Publish ocean current
  void PublishCurrentVelocity();

private:
  std::shared_ptr<rclcpp::Node> ros_node_;
  struct PrivateData;
  std::unique_ptr<PrivateData> dataPtr;
};
}  // namespace dave_gz_model_plugins

#endif  // DAVE_GZ_MODEL_PLUGINS__OCEAN_CURRENT_MODEL_PLUGIN_HH_