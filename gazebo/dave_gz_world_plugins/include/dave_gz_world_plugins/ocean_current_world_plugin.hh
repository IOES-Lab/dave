#ifndef DAVE_GZ_WORLD_PLUGINS__OCEAN_CURRENT_WORLD_PLUGIN_HH_
#define DAVE_GZ_WORLD_PLUGINS__OCEAN_CURRENT_WORLD_PLUGIN_HH_

#include <dave_gz_world_plugins/gauss_markov_process.hh>
#include <dave_gz_world_plugins/tidal_oscillation.hh>
#include <gz/sim/System.hh>
#include "dave_gz_world_plugins_msgs/msgs/StratifiedCurrentVelocity.pb.h"

// #include <cmath>
#include <gz/math/Vector4.hh>
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
                                public gz::sim::ISystemUpdate,
                                public gz::sim::ISystemPostUpdate
// public gz::sim::WorldPlugin

{
public:
  UnderwaterCurrentPlugin();
  ~UnderwaterCurrentPlugin();

  // Documentation inherited.
  void Configure(
    const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
    gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr);

  // Documentation inherited.
  virtual void Init();

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

  struct SharedData
  {
    dave_gz_world_plugins::GaussMarkovProcess currentHorzAngleModel;
    /// \brief Gauss-Markov process instance for the current velocity
    dave_gz_world_plugins::GaussMarkovProcess currentVelModel;
    dave_gz_world_plugins::GaussMarkovProcess currentVertAngleModel;

    /// \brief Vector for read stratified current database values
    std::vector<gz::math::Vector3d> stratifiedDatabase;  // check
    std::vector<std::vector<dave_gz_world_plugins::GaussMarkovProcess>> stratifiedCurrentModels;
    bool tidalHarmonicFlag;

    /// \brief Current linear velocity vector
    gz::math::Vector3d currentVelocity;

    /// \brief Vector of current depth-specific linear velocity vectors
    std::vector<gz::math::Vector4d> currentStratifiedVelocity;

    double M2_amp;
    double M2_phase;
    double M2_speed;
    double S2_amp;
    double S2_phase;
    double S2_speed;
    double N2_amp;
    double N2_phase;
    double N2_speed;

    /// \brief Tidal oscillation mean ebb direction
    double ebbDirection;

    /// \brief Tidal oscillation mean flood direction
    double floodDirection;

    /// \brief Tidal oscillation world start time (GMT)
    int world_start_time_day;
    int world_start_time_month;
    int world_start_time_year;
    int world_start_time_hour;
    int world_start_time_minute;

    std::vector<std::array<int, 5>> dateGMT;
    std::vector<double> speedcmsec;
  };
  std::unique_ptr<SharedData> sharedDataPtr;

private:
  // std::shared_ptr<rclcpp::Node> ros_node_;

  struct PrivateData;
  std::unique_ptr<PrivateData> dataPtr;
};
}  // namespace dave_gz_world_plugins

#endif  // DAVE_GZ_WORLD_PLUGINS__OCEAN_CURRENT_WORLD_PLUGIN_HH_
