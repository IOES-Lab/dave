// Copyright (c) 2016 The dave Simulator Authors.
// All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// \file ocean_current_world_plugin.hh
/// \brief Plugin that for the underwater world

#ifndef OCEAN_CURRENT_WORLD_PLUGIN_H_
#define OCEAN_CURRENT_WORLD_PLUGIN_H_

#include <dave_gz_world_plugins/gauss_markov_process.hh>
#include <dave_gz_world_plugins/tidal_oscillation.hh>

#include <ros/package.h>
#include <gz/sim/System.hh>

// #include <cmath>
#include <std/map>
#include <std/string>
#include <std/vector>

#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <gz/common/Plugin.hh>
#include <gz/common/Time.hh>

#include <gazebo/gazebo.hh>
#include <gz/transport/Node.hh>
#include <sdf/sdf.hh>

namespace gz
{
namespace sim
{
namespace systems
{
namespace dave_simulator_ros
{

/// \brief Class for the underwater current plugin

// typedef const boost::shared_ptr<const gz::msgs::Any> AnyPtr;

// public WorldPlugin,
class UnderwaterCurrentPlugin : public System,
                                public ISystemConfigure,
                                public ISystemPreUpdate,
                                public ISystemUpdate,
                                public ISystemPostUpdate,
                                public WorldPlugin

{
  /// \brief Class constructor
public:
  UnderwaterCurrentPlugin();

  /// \brief Class destructor
  virtual ~UnderwaterCurrentPlugin();

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

  void PostUpdate(const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

  /// \brief Load global current velocity configuration
protected:
  virtual void LoadGlobalCurrentConfig();

  /// \brief Load stratified current velocity database
  virtual void LoadStratifiedCurrentDatabase();

  /// \brief Load tidal oscillation database
  virtual void LoadTidalOscillationDatabase();

  /// \brief Publish current velocity and the pose of its frame
  void PublishCurrentVelocity();

  /// \brief Publish stratified oceqan current velocity
  void PublishStratifiedCurrentVelocity();

  /// \brief Update event
  event::ConnectionPtr updateConnection;

  /// \brief Pointer to world
  physics::WorldPtr world;

  /// \brief Pointer to sdf
  sdf::ElementPtr sdf;

  /// \brief True if the sea surface is present
  bool hasSurface;

  /// \brief Pointer to a node for communication
  transport::NodePtr node;

  /// \brief Map of publishers
  std::map<std::string, transport::PublisherPtr> publishers;

  /// \brief Vehicle Depth Subscriber
  transport::SubscriberPtr subscriber;

  /// \brief Current velocity topic
  std::string currentVelocityTopic;

  /// \brief Stratified Ocean current topic
  std::string stratifiedCurrentVelocityTopic;

  /// \brief Vehicle depth topic
  std::string vehicleDepthTopic;

  /// \brief Ocean Current Database file path for csv file
  std::string databaseFilePath;

  /// \brief Tidal Oscillation Database file path for txt file
  std::string tidalFilePath;

  /// \brief Vector for read stratified current database values
  std::vector<gz::math::Vector3d> stratifiedDatabase;

  /// \brief Namespace for topics and services
  std::string ns;

  /// \brief Gauss-Markov process instance for the current velocity
  GaussMarkovProcess currentVelModel;

  /// \brief Gauss-Markov process instance for horizontal angle model
  GaussMarkovProcess currentHorzAngleModel;

  /// \brief Gauss-Markov process instance for vertical angle model
  GaussMarkovProcess currentVertAngleModel;

  /// \brief Vector of Gauss-Markov process instances for stratified velocity
  std::vector<std::vector<GaussMarkovProcess>> stratifiedCurrentModels;

  /// \brief Vector of dateGMT for tidal oscillation
  std::vector<std::array<int, 5>> dateGMT;

  /// \brief Vector of speedcmsec for tidal oscillation
  std::vector<double> speedcmsec;

  /// \brief Tidal current harmonic constituents
  bool tidalHarmonicFlag;

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

  /// \brief Tidal Oscillation flag
  bool tideFlag;

  /// \brief Tidal Oscillation interpolation model
  TidalOscillation tide;

  /// \brief Last update time stamp
  common::Time lastUpdate;

  /// \brief Current linear velocity vector
  gz::math::Vector3d currentVelocity;

  /// \brief Vector of current depth-specific linear velocity vectors
  std::vector<gz::math::Vector4d> currentStratifiedVelocity;

  /// \brief File path for stratified current database
  std::string db_path;
};
}  // namespace dave_simulator_ros
}  // namespace systems
}  // namespace sim
}  // namespace gz

#endif  // OCEAN_CURRENT_WORLD_PLUGIN_H_
