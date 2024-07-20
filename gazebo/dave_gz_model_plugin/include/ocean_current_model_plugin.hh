#ifndef OCEAN_CURRENT_MODEL_PLUGIN_H_
#define OCEAN_CURRENT_MODEL_PLUGIN_H_

#include <dave_gz_ros_plugins/StratifiedCurrentDatabase.hh>
#include <dave_gz_ros_plugins/gauss_markov_process.hh>
// #include <dave_gz_ros_plugins/tidal_oscillation.h>

#include <gz/sim/System.hh>
#include <gz/utilise/ImplPtr.hh>

#include <gz/common/Plugin.hh>
#include <gz/physics/World.hh>
#include <std/shared_ptr.hpp>

#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/vector3.hh>
#include <gz/common/Plugin.hh>
#include <gz/common/Time.hh>
#include <rclcpp/rclcpp.hpp>
#include <std/chrono>
#include <std/map>
#include <std/memory>
#include <std/string>

namespace gz
{
namespace sim
{
namespace systems
{
namespace dave_gz_model_plugins
{
class TransientCurrentPlugin : public gz::sim::System,
                               public gz::sim::ISystemConfigure,
                               public gz::sim::ISystemPreUpdate,
                               public gz::sim::ISystemUpdate,
                               public gz::sim::ISystemPostUpdate,
                               public ModelPlugin
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

  void TransientCurrentPlugin::Update(
    const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  // Function called after the simulation state updates
  void PostUpdate(const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

  /// \brief Check if an entity is enabled or not.
  /// \param[in] _entity Target entity
  /// \param[in] _ecm Entity component manager
  /// \return True if buoyancy should be applied.
  bool IsEnabled(Entity _entity, const EntityComponentManager & _ecm) const;

  // Initialize any necessary states before the plugin starts
  virtual void Init();

  /// \brief ROS helper function that processes messages
  void databaseSubThread();

  /// \brief Calculate ocean current using database and vehicle state
  void TransientCurrentPlugin::CalculateOceanCurrent(
    const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  void TransientCurrentPlugin::PublishCurrentVelocity(
    const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

  void TransientCurrentPlugin::Gauss_Markov_process_initialize(
    const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  TransientCurrentPlugin::UpdateDatabase(
    const dave_ros_gz_plugins::StratifiedCurrentDatabase::ConstPtr & _msg)

    /// \brief Publish ocean current
    void TransientCurrentPlugin::PublishCurrentVelocity(
      const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
}
}  // namespace dave_gz_model_plugins
}  // namespace systems
}  // namespace sim
}  // namespace gz
