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
class OceanCurrentWorldPlugin : public gz::sim::System,
                                public gz::sim::ISystemConfigure,
                                public gz::sim::ISystemUpdate,
                                public gz::sim::ISystemPostUpdate
// public gz::sim::WorldPlugin

{
public:
  OceanCurrentWorldPlugin();
  ~OceanCurrentWorldPlugin();

  // ----------------------------------------------

  void Configure(
    const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
    gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr);

  virtual void LoadTidalOscillationDatabase();

  virtual void LoadStratifiedCurrentDatabase();

  virtual void LoadGlobalCurrentConfig();

  // Documentation inherited.
  virtual void Init();

  // ----------------------------------------------

  void Update(const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  // ----------------------------------------------

  void PostUpdate(const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

  void PublishCurrentVelocity();

  void PublishStratifiedCurrentVelocity();

  // ----------------------------------------------
  struct SharedData
  {
    // Gauss-Markov process instances for current models
    dave_gz_world_plugins::GaussMarkovProcess currentHorzAngleModel;  // Horizontal angle
    dave_gz_world_plugins::GaussMarkovProcess currentVelModel;        // Velocity
    dave_gz_world_plugins::GaussMarkovProcess currentVertAngleModel;  // Vertical angle

    // Stratified current database and models
    std::vector<gz::math::Vector3d> stratifiedDatabase;  // Database values for stratified currents
    std::vector<std::vector<dave_gz_world_plugins::GaussMarkovProcess>>
      stratifiedCurrentModels;  // Stratified current models

    // Tidal harmonic flag
    bool tidalHarmonicFlag;

    // Current velocity information
    gz::math::Vector3d currentVelocity;  // Current linear velocity vector
    std::vector<gz::math::Vector4d>
      currentStratifiedVelocity;  // Depth-specific linear velocity vectors for stratified current

    // Tidal harmonic data (M2, S2, N2 constituents)
    double M2_amp, M2_phase, M2_speed;  // M2 tidal constituent
    double S2_amp, S2_phase, S2_speed;  // S2 tidal constituent
    double N2_amp, N2_phase, N2_speed;  // N2 tidal constituent

    // Tidal oscillation mean directions
    double ebbDirection;    // Mean ebb direction
    double floodDirection;  // Mean flood direction

    // Tidal oscillation world start time (GMT)
    int world_start_time_day;
    int world_start_time_month;
    int world_start_time_year;
    int world_start_time_hour;
    int world_start_time_minute;

    // Topics
    std::string currentVelocityTopic;
    std::string stratifiedCurrentVelocityTopic;
    // std::string vehicleDepthTopic;

    // Date and speed information
    std::vector<std::array<int, 5>> dateGMT;  // Date in GMT (year, month, day, hour, minute)
    std::vector<double> speedcmsec;           // Speed in cm/sec
  };
  std::unique_ptr<SharedData> sharedDataPtr;

private:
  // std::shared_ptr<rclcpp::Node> ros_node_;

  struct PrivateData;
  std::unique_ptr<PrivateData> dataPtr;
};
}  // namespace dave_gz_world_plugins

#endif  // DAVE_GZ_WORLD_PLUGINS__OCEAN_CURRENT_WORLD_PLUGIN_HH_
