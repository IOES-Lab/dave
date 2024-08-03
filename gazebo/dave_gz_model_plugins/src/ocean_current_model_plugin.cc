// Copyright (c) 2016 The UUV Simulator Authors.
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

/// \file ocean_current_model_plugin.cc
#include "dave_gz_model_plugins/ocean_current_model_plugin.hh"
// #include <dave_gz_world_plugins/gauss_markov_process.hh>
// #include <dave_gz_world_plugins/tidal_oscillation.hh>
#include <algorithm>
#include <chrono>
#include <gz/physics/World.hh>
#include <gz/plugin/Register.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/Util.hh>
#include <gz/sim/World.hh>
#include <gz/sim/components/Link.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/ParentEntity.hh>
#include <gz/transport/Node.hh>
#include <iostream>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64.hpp>
#include <std_msgs/msg/string.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include "gz/common/StringUtils.hh"
#include "gz/plugin/Register.hh"

// #include <ament_index_cpp/get_package_share_directory.hpp> //TODO
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <sdf/sdf.hh>

// Register plugin
GZ_ADD_PLUGIN(
  dave_gz_model_plugins::TransientCurrentPlugin, gz::sim::System,
  dave_gz_model_plugins::TransientCurrentPlugin::ISystemConfigure,
  dave_gz_model_plugins::TransientCurrentPlugin::ISystemPreUpdate,
  dave_gz_model_plugins::TransientCurrentPlugin::ISystemUpdate,
  dave_gz_model_plugins::TransientCurrentPlugin::ISystemPostUpdate)

namespace dave_gz_model_plugins
{
struct TransientCurrentPlugin::PrivateData
{
  // starts here

  /// \brief Check if an entity is enabled or not.
  /// \param[in] _entity Target entity
  /// \param[in] _ecm Entity component manager
  /// \return True if buoyancy should be applied.
  // bool IsEnabled(Entity _entity, const EntityComponentManager & _ecm) const;

  // Initialize any necessary states before the plugin starts
  virtual void Init();

  std::shared_ptr<rclcpp::Node> rosNode;  // This is a smart pointer to a ROS 2 node, facilitating
                                          // communication with other ROS nodes.
  std::string transientCurrentVelocityTopic;  // Declare the variable (updated)

protected:
  /// \brief Pointer to world
  gz::sim::World world{gz::sim::kNullEntity};

  /// \brief Pointer to model
  gz::sim::Entity entity{gz::sim::kNullEntity};
  gz::sim::Model model{gz::sim::kNullEntity};
  gz::sim::Entity modelLink{gz::sim::kNullEntity};

  /// \brief Namespace for topics and services
  std::string ns;

  /// \brief Connects the update event callback
  virtual void Connect();

  /// \brief Pointer to a node for communication
  std::shared_ptr<gz::transport::Node> gz_node;

  /// \brief Map of publishers
  std::map<std::string, gz::transport::Publisher> publishers;

  /// \brief Current velocity topic and transient current velocity topic
  std::string currentVelocityTopic;

  /// \brief Last update time stamp
  std::chrono::steady_clock::duration lastUpdate{0};

  /// \brief Last depth index
  int lastDepthIndex;

  /// \brief Current linear velocity vector
  gz::math::Vector3d currentVelocity;

  /// \brief Gauss-Markov process instance for the velocity components // TODO
  // gz::GaussMarkovProcess currentVelNorthModel; // TODO
  // gz::GaussMarkovProcess currentVelEastModel; // TODO
  // gz::GaussMarkovProcess currentVelDownModel; // TODO

  /// \brief Gauss-Markov noise
  double noiseAmp_North;
  double noiseAmp_East;
  double noiseAmp_Down;
  double noiseFreq_North;
  double noiseFreq_East;
  double noiseFreq_Down;

  /// \brief Vector to read database
  std::vector<gz::math::Vector3d> database;

  /// \brief Tidal Oscillation interpolation model
  // gz::TidalOscillation tide;

  /// \brief Tidal Oscillation flag
  bool tideFlag;

  /// \brief Vector to read timeGMT
  std::vector<std::array<int, 5>> timeGMT;

  /// \brief Vector to read tideVelocities
  std::vector<double> tideVelocities;

  /// \brief Tidal current harmonic constituents
  bool tide_Constituents;
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
  std::array<int, 5> world_start_time;

  /// \brief Private data pointer
private:
  std::shared_ptr<rclcpp::Node> ros_node_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr flowVelocityPub;

  /// \brief Pointer to this ROS node's handle.
  std::shared_ptr<rclcpp::NodeHandle> rosNode;

  /// \brief Publisher for the flow velocity in the world frame

  /// \brief Subscriber for the transient ocean current database
  rclcpp::Subscriber databaseSub;

  /// \brief A ROS callbackqueue that helps process messages
  rclcpp::CallbackQueue databaseSubQueue;

  /// \brief A thread the keeps running the rosQueue
  std::thread databaseSubQueueThread;

  /// \brief A thread mutex to lock
  std::mutex lock_;

  /// \brief Period after which we should publish a message via ROS.
  std::chrono::steady_clock::duration rosPublishPeriod{0};

  /// \brief Last time we published a message via ROS.
  // std::chrono::steady_clock::duration lastRosPublishTime{0};

  std::shared_ptr<rclcpp::Node> ros_node_;
  // std::chrono::steady_clock::duration currentTime{0};
  this->dataPtr->rosPublishPeriod = 0.05;
  // ends here
};

// constructor
TransientCurrentPlugin::TransientCurrentPlugin() : dataPtr(std::make_unique<PrivateData>()) {}

TransientCurrentPlugin::~TransientCurrentPlugin()
{
  // Shutdown the ROS 2 node
  this->rosNode.reset();
}

void TransientCurrentPlugin::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr)
{
  this->dataPtr->world = _entity;
  this->dataPtr->entity = _entity;
  this->dataPtr->model = gz::sim::Model(_entity);
  this->dataPtr->modelName = this->dataPtr->model.Name(_ecm);
  this->dataPtr->sdf = _sdf;

  gzmsg << "Loading transient ocean current model plugin..." << std::endl;

  if (_sdf->HasElement("flow_velocity_topic"))
  {
    this->dataPtr->currentVelocityTopic = _sdf->Get<std::string>("flow_velocity_topic");
  }
  else
  {
    this->dataPtr->currentVelocityTopic =
      "hydrodynamics/current_velocity/" + this->dataPtr->model.Name();
    gz::sim::log::error() << "Empty flow_velocity_topic for transient_current model plugin. "
                          << "Default topicName definition is used" << std::endl;
  }
  gz::sim::log::info() << "Transient velocity topic name for " << this->dataPtr->model.Name()
                       << " : " << this->dataPtr->currentVelocityTopic << std::endl;
  // Read the namespace for topics and services
  this->dataPtr->ns = _sdf->Get<std::string>("namespace");

  // Initialize the ROS 2 node
  this->dataPtr->ros_node_ = std::make_shared<rclcpp::Node>("TransirentCurrentPlugin");

  // Advertise the ROS flow velocity as a stamped twist message
  this->dataPtr->flowVelocityPub =
    this->dataPtr->ros_node_->create_publisher<geometry_msgs::msg::TwistStamped>(
      this->dataPtr->currentVelocityTopic, rclcpp::QOS(10));

  // Initializing the Gazebo transport node
  this->dataPtr->gz_node = transport::NodePtr(new transport::Node());  // check

  // Advertise the current velocity topic in ROS 2
  this->dataPtr->publishers[this->dataPtr->currentVelocityTopic] =
    this->dataPtr->gz_node->create_publisher<geometry_msgs::msg::Vector3>(
      this->dataPtr->currentVelocityTopic, rclcpp::QoS(10));

  // Read topic name of stratified ocean current from SDF
  if (_sdf->HasElement("transient_current"))
  {
    sdf::ElementPtr currentVelocityParams = _sdf->GetElement("transient_current");
    if (currentVelocityParams->HasElement("topic_stratified"))
    {
      this->dataPtr->transientCurrentVelocityTopic =
        "/hydrodynamics/" + currentVelocityParams->Get<std::string>("topic_stratified") +
        "_database";
    }
    else
    {
      this->dataPtr->transientCurrentVelocityTopic =
        "/hydrodynamics/stratified_current_velocity_database";
    }

    // Tidal Oscillation
    if (
      this->dataPtr->sdf->HasElement("tide_oscillation") &&
      this->dataPtr->sdf->Get<bool>("tide_oscillation") == true)
    {
      this->dataPtr->tideFlag = true;
    }
    else
    {
      this->dataPtr->tideFlag = false;
    }

    this->dataPtr->lastDepthIndex = 0;
  }
}
/////////////////////////////////////////////////
void TransientCurrentPlugin::Init()
{
  // Doing nothing for now
}
/////////////////////////////////////////////////
void TransientCurrentPlugin::CalculateOceanCurrent()
{
  this->dataPtr->lock_.lock();

  // Update vehicle position
  double vehicleDepth = -this->dataPtr->model->WorldPose().Pos().Z();

  if (this->dataPtr->database.size() == 0)
  {
    // skip for next time (waiting for valid database subscrition)
  }
  else
  {
    double northCurrent = 0.0;
    double eastCurrent = 0.0;

    //--- Interpolate velocity from database ---//
    // find current depth index from database
    // (X: north-direction, Y: east-direction, Z: depth)
    int depthIndex = 0;
    for (int i = 1; i < this->dataPtr->database.size(); i++)
    {
      if (this->dataPtr->database[i].Z() > vehicleDepth)
      {
        depthIndex = i;
        break;
      }
    }

    // If sudden change found, use the one before
    if (this->dataPtr->lastDepthIndex == 0)
    {
      this->dataPtr->lastDepthIndex = depthIndex;
    }
    else
    {
      if (abs(depthIndex - this->dataPtr->lastDepthIndex) > 2)
      {
        depthIndex = this->dataPtr->lastDepthIndex;
      }
      this->dataPtr->lastDepthIndex = depthIndex;
    }

    // interpolate
    if (depthIndex == 0)
    {  // Deeper than database use deepest value
      northCurrent = this->dataPtr->database[this->dataPtr->database.size() - 1].X();
      eastCurrent = this->dataPtr->database[this->dataPtr->database.size() - 1].Y();
    }
    else
    {
      double rate =
        (vehicleDepth - this->dataPtr->database[depthIndex - 1].Z()) /
        (this->dataPtr->database[depthIndex].Z() - this->dataPtr->database[depthIndex - 1].Z());
      northCurrent =
        (this->dataPtr->database[depthIndex].X() - this->dataPtr->database[depthIndex - 1].X()) *
          rate +
        this->dataPtr->database[depthIndex - 1].X();
      eastCurrent =
        (this->dataPtr->database[depthIndex].Y() - this->dataPtr->database[depthIndex - 1].Y()) *
          rate +
        this->dataPtr->database[depthIndex - 1].Y();
    }
    this->dataPtr->currentVelNorthModel.mean = northCurrent;
    this->dataPtr->currentVelEastModel.mean = eastCurrent;
    this->dataPtr->currentVelDownModel.mean = 0.0;

    // Tidal oscillation
    if (this->dataPtr->tideFlag)
    {
      // Update tide oscillation
      this->dataPtr->time = this->dataPtr->currentTime;

      if (this->dataPtr->tide_Constituents)
      {
        this->dataPtr->tide.M2_amp = this->dataPtr->M2_amp;
        this->dataPtr->tide.M2_phase = this->dataPtr->M2_phase;
        this->dataPtr->tide.M2_speed = this->dataPtr->M2_speed;
        this->dataPtr->tide.S2_amp = this->dataPtr->S2_amp;
        this->dataPtr->tide.S2_phase = this->dataPtr->S2_phase;
        this->dataPtr->tide.S2_speed = this->dataPtr->S2_speed;
        this->dataPtr->tide.N2_amp = this->dataPtr->N2_amp;
        this->dataPtr->tide.N2_phase = this->dataPtr->N2_phase;
        this->dataPtr->tide.N2_speed = this->dataPtr->N2_speed;
      }
      else
      {
        this->dataPtr->tide.dateGMT = this->dataPtr->timeGMT;
        this->dataPtr->tide.speedcmsec = this->dataPtr->tideVelocities;
      }
      this->dataPtr->tide.ebbDirection = this->dataPtr->ebbDirection;
      this->dataPtr->tide.floodDirection = this->dataPtr->floodDirection;
      this->dataPtr->tide.worldStartTime = this->dataPtr->world_start_time;
      this->dataPtr->tide.Initiate(this->dataPtr->tide_Constituents);
      std::pair<double, double> currents =
        this->dataPtr->tide.Update((this->dataPtr->time).Double(), northCurrent);
      this->dataPtr->currentVelNorthModel.mean = currents.first;
      this->dataPtr->currentVelEastModel.mean = currents.second;
      this->dataPtr->currentVelDownModel.mean = 0.0;
    }
    else
    {
      this->dataPtr->currentVelNorthModel.mean = northCurrent;
      this->dataPtr->currentVelEastModel.mean = eastCurrent;
      this->dataPtr->currentVelDownModel.mean = 0.0;
    }

    // Change min max accordingly
    currentVelNorthModel.max = currentVelNorthModel.mean + this->dataPtr->noiseAmp_North;
    currentVelNorthModel.min = currentVelNorthModel.mean - this->dataPtr->noiseAmp_North;
    currentVelEastModel.max = currentVelEastModel.mean + this->dataPtr->noiseAmp_East;
    currentVelEastModel.min = currentVelEastModel.mean - this->dataPtr->noiseAmp_East;
    currentVelDownModel.max = currentVelDownModel.mean + this->dataPtr->noiseAmp_Down;
    currentVelDownModel.min = currentVelDownModel.mean - this->dataPtr->noiseAmp_Down;

    // Assign values to the model
    this->dataPtr->currentVelNorthModel.var = this->dataPtr->currentVelNorthModel.mean;
    this->dataPtr->currentVelEastModel.var = this->dataPtr->currentVelEastModel.mean;
    this->dataPtr->currentVelDownModel.var = this->dataPtr->currentVelDownModel.mean;

    // Update time
    this->dataPtr->time = this->dataPtr->lastUpdate;

    // Update current velocity
    double velocityNorth =
      this->dataPtr->currentVelNorthModel.Update((this->dataPtr->time).Double());

    // Update current horizontal direction around z axis of flow frame
    double velocityEast = this->dataPtr->currentVelEastModel.Update((this->dataPtr->time).Double());

    // Update current horizontal direction around z axis of flow frame
    double velocityDown = this->dataPtr->currentVelDownModel.Update((this->dataPtr->time).Double());

    // Update current Velocity
    this->dataPtr->currentVelocity = gz::math::Vector3d(velocityNorth, velocityEast, velocityDown);

    // Update time stamp
    this->dataPtr->lastUpdate = (this->dataPtr->time).Double();
  }

  this->dataPtr->lock_.unlock();
}
/////////////////////////////////////////////////
void TransientCurrentPlugin::PublishCurrentVelocity()
{
  geometry_msgs::TwistStamped flowVelMsg;
  flowVelMsg.header.stamp = rclcpp::Time().now();
  flowVelMsg.header.frame_id = "/world";
  flowVelMsg.twist.linear.x = this->dataPtr->currentVelocity.X();
  flowVelMsg.twist.linear.y = this->dataPtr->currentVelocity.Y();
  flowVelMsg.twist.linear.z = this->dataPtr->currentVelocity.Z();
  this->dataPtr->flowVelocityPub.publish(flowVelMsg);

  // Generate and publish Gazebo topic according to the vehicle depth
  msgs::Vector3d currentVel;
  msgs::Set(
    &currentVel, gz::math::Vector3d(
                   this->dataPtr->currentVelocity.X(), this->dataPtr->currentVelocity.Y(),
                   this->dataPtr->currentVelocity.Z()));
  this->dataPtr->publishers[this->dataPtr->currentVelocityTopic]->Publish(currentVel);
}

/////////////////////////////////////////////////
TransientCurrentPlugin::UpdateDatabase()
{
  this->dataPtr->lock_.lock();

  this->dataPtr->database.clear();
  for (int i = 0; i < _msg->depths.size(); i++)
  {
    gz::math::Vector3d data(_msg->velocities[i].x, _msg->velocities[i].y, _msg->depths[i]);
    this->dataPtr->database.push_back(data);
  }
  if (this->dataPtr->tideFlag)
  {
    this->dataPtr->timeGMT.clear();
    this->dataPtr->tideVelocities.clear();
    if (_msg->tideConstituents == true)
    {
      this->dataPtr->M2_amp = _msg->M2amp;
      this->dataPtr->M2_phase = _msg->M2phase;
      this->dataPtr->M2_speed = _msg->M2speed;
      this->dataPtr->S2_amp = _msg->S2amp;
      this->dataPtr->S2_phase = _msg->S2phase;
      this->dataPtr->S2_speed = _msg->S2speed;
      this->dataPtr->N2_amp = _msg->N2amp;
      this->dataPtr->N2_phase = _msg->N2phase;
      this->dataPtr->N2_speed = _msg->N2speed;
      this->dataPtr->tide_Constituents = true;
    }
    else
    {
      std::array<int, 5> tmpDateVals;
      for (int i = 0; i < _msg->timeGMTYear.size(); i++)
      {
        tmpDateVals[0] = _msg->timeGMTYear[i];
        tmpDateVals[1] = _msg->timeGMTMonth[i];
        tmpDateVals[2] = _msg->timeGMTDay[i];
        tmpDateVals[3] = _msg->timeGMTHour[i];
        tmpDateVals[4] = _msg->timeGMTMinute[i];

        this->dataPtr->timeGMT.push_back(tmpDateVals);
        this->dataPtr->tideVelocities.push_back(_msg->tideVelocities[i]);
      }
      this->dataPtr->tide_Constituents = false;
    }
    this->dataPtr->ebbDirection = _msg->ebbDirection;
    this->dataPtr->floodDirection = _msg->floodDirection;
    this->dataPtr->world_start_time[0] = _msg->worldStartTimeYear;
    this->dataPtr->world_start_time[1] = _msg->worldStartTimeMonth;
    this->dataPtr->world_start_time[2] = _msg->worldStartTimeDay;
    this->dataPtr->world_start_time[3] = _msg->worldStartTimeHour;
    this->dataPtr->world_start_time[4] = _msg->worldStartTimeMinute;
  }

  this->dataPtr->lock_.unlock();
}

///////////////////////////////////////////////// (check this,dpr)
void TransientCurrentPlugin::Gauss_Markov_process_initialize()
{
  // Read Gauss-Markov parameters

  // initialize velocity_north_model parameters
  if (currentVelocityParams->HasElement("velocity_north"))
  {
    sdf::ElementPtr elem = currentVelocityParams->GetElement("velocity_north");
    if (elem->HasElement("mean"))
    {
      this->currentVelNorthModel.mean = 0.0;
    }
    if (elem->HasElement("mu"))
    {
      this->currentVelNorthModel.mu = 0.0;
    }
    if (elem->HasElement("noiseAmp"))
    {
      this->noiseAmp_North = elem->Get<double>("noiseAmp");
    }
    this->currentVelNorthModel.min = this->currentVelNorthModel.mean - this->noiseAmp_North;
    this->currentVelNorthModel.max = this->currentVelNorthModel.mean + this->noiseAmp_North;
    if (elem->HasElement("noiseFreq"))
    {
      this->noiseFreq_North = elem->Get<double>("noiseFreq");
    }
    this->currentVelNorthModel.noiseAmp = this->noiseFreq_North;
  }

  this->dataPtr->currentVelNorthModel.var = this->dataPtr->currentVelNorthModel.mean;
  gzmsg << "For vehicle " << this->dataPtr->model->GetName()
        << " -> Current north-direction velocity [m/s] "
        << "Gauss-Markov process model:" << std::endl;
  this->dataPtr->currentVelNorthModel.Print();

  // initialize velocity_east_model parameters
  if (currentVelocityParams->HasElement("velocity_east"))
  {
    sdf::ElementPtr elem = currentVelocityParams->GetElement("velocity_east");
    if (elem->HasElement("mean"))
    {
      this->currentVelEastModel.mean = 0.0;
    }
    if (elem->HasElement("mu"))
    {
      this->currentVelEastModel.mu = 0.0;
    }
    if (elem->HasElement("noiseAmp"))
    {
      this->noiseAmp_East = elem->Get<double>("noiseAmp");
    }
    this->currentVelEastModel.min = this->currentVelEastModel.mean - this->noiseAmp_East;
    this->currentVelEastModel.max = this->currentVelEastModel.mean + this->noiseAmp_East;
    if (elem->HasElement("noiseFreq"))
    {
      this->noiseFreq_East = elem->Get<double>("noiseFreq");
    }
    this->currentVelEastModel.noiseAmp = this->noiseFreq_East;
  }

  this->dataPtr->currentVelEastModel.var = this->dataPtr->currentVelEastModel.mean;
  gzmsg << "For vehicle " << this->dataPtr->model->GetName()
        << " -> Current east-direction velocity [m/s] "
        << "Gauss-Markov process model:" << std::endl;
  this->dataPtr->currentVelEastModel.Print();

  // initialize velocity_down_model parameters
  if (currentVelocityParams->HasElement("velocity_down"))
  {
    sdf::ElementPtr elem = currentVelocityParams->GetElement("velocity_down");
    if (elem->HasElement("mean"))
    {
      this->currentVelDownModel.mean = 0.0;
    }
    if (elem->HasElement("mu"))
    {
      this->currentVelDownModel.mu = 0.0;
    }
    if (elem->HasElement("noiseAmp"))
    {
      this->noiseAmp_Down = elem->Get<double>("noiseAmp");
    }
    this->currentVelDownModel.min = this->currentVelDownModel.mean - this->noiseAmp_Down;
    this->currentVelDownModel.max = this->currentVelDownModel.mean + this->noiseAmp_Down;
    if (elem->HasElement("noiseFreq"))
    {
      this->noiseFreq_Down = elem->Get<double>("noiseFreq");
    }
    this->currentVelDownModel.noiseAmp = this->noiseFreq_Down;
  }

  this->dataPtr->currentVelDownModel.var = this->dataPtr->currentVelDownModel.mean;
  gzmsg << "For vehicle " << this->dataPtr->modelName << " -> Current down-direction velocity [m/s]"
        << "Gauss-Markov process model:" << std::endl;
  this->dataPtr->currentVelDownModel.Print();

  // this->dataPtr->lastUpdate = _info.simTime; This isn't needed because the last update is being
  // initialised to 0 and is being updated at postupdate
  this->dataPtr->currentVelNorthModel.lastUpdate = this->dataPtr->lastUpdate.Double();
  this->dataPtr->currentVelEastModel.lastUpdate = this->dataPtr->lastUpdate.Double();
  this->dataPtr->currentVelDownModel.lastUpdate = this->dataPtr->lastUpdate.Double();
}
/////////////////////////////////////////////////
void TransientCurrentPlugin::PreUpdate(
  const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm)
{
  this->dataPtr->currentTime = _info.simTime;
  this->dataPtr->time = this->dataPtr->currentTime;
  this->dataPtr->Gauss_Markov_process_initialize();  // something is wrong here (check)

  // Subscribe stratified ocean current database
  this->dataPtr->databaseSub =
    this->dataPtr->rosNode
      ->create_subscription<dave_ros_gz_plugins::msg::StratifiedCurrentDatabase>(
        this->dataPtr->transientCurrentVelocityTopic, 10,
        std::bind(&TransientCurrentPlugin::UpdateDatabase, this, _1));

  // Connect the update event callback for ROS and ocean current calculation
  this->dataPtr->Connect();

  gzmsg << "Transient current model plugin loaded!" << std::endl;
}

/////////////////////////////////////////////////
void TransientCurrentPlugin::Update(
  const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm)
{
  this->dataPtr->calculateOceanCurrent();
}
/////////////////////////////////////////////////
void TransientCurrentPlugin::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  // Publish the Current Velocity.
  if (!_info.paused && _info.simTime > this->dataPtr->lastUpdate)
  {
    this->dataPtr->lastUpdate = _info.simTime;
    PostPublishCurrentVelocity();
  }
}
}  // namespace dave_gz_model_plugins