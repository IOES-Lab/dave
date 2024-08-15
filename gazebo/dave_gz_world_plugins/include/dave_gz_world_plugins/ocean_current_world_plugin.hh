#ifndef DAVE_GZ_WORLD_PLUGINS__OCEAN_CURRENT_WORLD_PLUGIN_HH_
#define DAVE_GZ_WORLD_PLUGINS__OCEAN_CURRENT_WORLD_PLUGIN_HH_

#include <dave_gz_world_plugins/gauss_markov_process.hh>
#include <dave_gz_world_plugins/tidal_oscillation.hh>
#include <gz/sim/System.hh>
#include "dave_gz_world_plugins_msgs/msgs/StratifiedCurrentVelocity.pb.h"

// #include <cmath>
#include <map>
#include <string>
#include <vector>

#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/vector3.hpp>

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

  void PreUpdate(const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  void Update(const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

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

#endif  // DAVE_GZ_WORLD_PLUGINS__OCEAN_CURRENT_WORLD_PLUGIN_HH_
