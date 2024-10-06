// Plugin includes
#include "dave_gz_model_plugins/OceanCurrentModelPlugin.hh"
#include "dave_gz_world_plugins/gauss_markov_process.hh"
#include "dave_gz_world_plugins/tidal_oscillation.hh"

// Gazebo includes
#include <gz/msgs/vector3d.pb.h>
#include <gz/physics/World.hh>
#include <gz/plugin/Register.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/System.hh>
#include <gz/sim/Util.hh>
#include <gz/sim/World.hh>
#include <gz/sim/components/Link.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/ParentEntity.hh>
#include <gz/sim/components/Pose.hh>
#include <gz/sim/components/World.hh>
#include <gz/transport/Node.hh>

// ROS2 includes
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

// Message interfaces includes
#include "dave_interfaces/msg/Stratified_Current_Database.hpp"
#include "dave_interfaces/msg/Stratified_Current_Velocity.hpp"

// Standard library includes
#include <algorithm>
#include <ament_index_cpp/get_package_share_directory.hpp>
#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Common utilities
#include "gz/common/StringUtils.hh"
#include "gz/plugin/Register.hh"

GZ_ADD_PLUGIN(
  dave_gz_model_plugins::OceanCurrentModelPlugin, gz::sim::System,
  dave_gz_model_plugins::OceanCurrentModelPlugin::ISystemConfigure,
  dave_gz_model_plugins::OceanCurrentModelPlugin::ISystemUpdate,
  dave_gz_model_plugins::OceanCurrentModelPlugin::ISystemPostUpdate)

namespace dave_gz_model_plugins
{
struct OceanCurrentModelPlugin::PrivateData
{
  // Gazebo Simulation Related Variables
  gz::sim::World world{gz::sim::kNullEntity};
  gz::sim::Entity entity{gz::sim::kNullEntity};
  gz::sim::Model model{gz::sim::kNullEntity};
  gz::sim::Entity modelLink{gz::sim::kNullEntity};
  gz::sim::Entity modelEntity;
  std::string modelName;

  // Transport and Communication
  std::shared_ptr<gz::transport::Node> gz_node;
  gz::transport::Node::Publisher gz_current_vel_pub;
  std::shared_ptr<rclcpp::Node> ros_node_;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr flowVelocityPub;
  rclcpp::Subscription<dave_interfaces::msg::StratifiedCurrentDatabase>::SharedPtr databaseSub;

  // Topics
  std::string currentVelocityTopic;
  std::string transientCurrentVelocityTopic;

  // Time and Periods
  std::chrono::steady_clock::duration lastUpdate{0};
  std::chrono::steady_clock::duration rosPublishPeriod{0};
  std::chrono::steady_clock::duration time;

  // Lock for Synchronization
  std::mutex lock_;

  // Current Velocity
  gz::math::Vector3d currentVelocity;
  int lastDepthIndex = 0;
  std::vector<gz::math::Vector3d> database;

  // Gauss-Markov Process Models
  dave_gz_world_plugins::GaussMarkovProcess currentVelNorthModel;
  dave_gz_world_plugins::GaussMarkovProcess currentVelEastModel;
  dave_gz_world_plugins::GaussMarkovProcess currentVelDownModel;

  // Noise Amplitudes and Frequencies
  double noiseAmp_North;
  double noiseAmp_East;
  double noiseAmp_Down;
  double noiseFreq_North;
  double noiseFreq_East;
  double noiseFreq_Down;

  // Tidal Oscillation Model
  dave_gz_world_plugins::TidalOscillation tide;

  // Tidal Oscillation Settings and Flags
  bool tideFlag;
  bool tide_Constituents;
  double M2_amp, M2_phase, M2_speed;
  double S2_amp, S2_phase, S2_speed;
  double N2_amp, N2_phase, N2_speed;

  // Tidal Oscillation Directions
  double ebbDirection;
  double floodDirection;

  // Tidal Oscillation World Start Time (GMT)
  std::array<int, 5> world_start_time;
  std::vector<std::array<int, 5>> timeGMT;

  // Tidal Velocities
  std::vector<double> tideVelocities;
};

OceanCurrentModelPlugin::OceanCurrentModelPlugin() : dataPtr(std::make_unique<PrivateData>()) {}

OceanCurrentModelPlugin::~OceanCurrentModelPlugin() = default;

// ----------------------------------------------

/////////////////////////////////////////////////
void OceanCurrentModelPlugin::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr)
{
  // Check if ROS is initialized; if not, initialize it
  if (!rclcpp::ok())
  {
    rclcpp::init(0, nullptr);
  }

  // Initialize the ROS 2 node
  this->ros_node_ = std::make_shared<rclcpp::Node>("OceanCurrentModelPlugin");

  // Initialize the Gazebo transport node
  this->dataPtr->gz_node = std::make_shared<gz::transport::Node>();

  gzdbg << "dave_gz_model_plugins::OceanCurrentModelPlugin::Configure on entity: " << _entity
        << std::endl;

  // Clone the SDF element to allow calling non-const methods
  sdf::ElementPtr sdfClone = _sdf->Clone();

  // Get the world entity from the Entity Component Manager (ECM)
  auto worldEntity = _ecm.EntityByComponents(gz::sim::components::World());
  this->dataPtr->world = gz::sim::World(worldEntity);

  // Get the model entity
  auto model = gz::sim::Model(_entity);
  this->dataPtr->model = model;
  gzmsg << "Loading ocean current model plugin..." << std::endl;

  // Read the namespace for topics and services
  if (!_sdf->HasElement("namespace"))
  {
    gzerr << "Missing required parameter <namespace>, plugin will not be initialized." << std::endl;
    return;
  }
  this->dataPtr->modelName = _sdf->Get<std::string>("namespace");

  // Set the flow velocity topic from SDF or use the default topic name
  if (_sdf->HasElement("flow_velocity_topic"))
  {
    this->dataPtr->currentVelocityTopic = _sdf->Get<std::string>("flow_velocity_topic");
  }
  else
  {
    this->dataPtr->currentVelocityTopic =
      "hydrodynamics/current_velocity/" + this->dataPtr->modelName;
    gzwarn << "Empty flow_velocity_topic for transient_current model plugin. Default topicName "
              "definition is used."
           << std::endl;
  }
  gzmsg << "Transient velocity topic name for " << this->dataPtr->modelName << " : "
        << this->dataPtr->currentVelocityTopic << std::endl;

  // Get the model entity based on the model name
  this->dataPtr->modelEntity = GetModelEntity(this->dataPtr->modelName, _ecm);

  // Read the stratified ocean current topic name from the SDF
  LoadCurrentVelocityParams(sdfClone, _ecm);

  // Initialize the Gauss-Markov process
  //   Gauss_Markov_process_initialize(_sdf);

  // Advertise the ROS flow velocity as a stamped twist message
  this->dataPtr->flowVelocityPub =
    this->ros_node_->create_publisher<geometry_msgs::msg::TwistStamped>(
      this->dataPtr->currentVelocityTopic, rclcpp::QoS(10));

  // Advertise the current velocity topic in Gazebo
  this->dataPtr->gz_current_vel_pub =
    this->dataPtr->gz_node->Advertise<gz::msgs::Vector3d>(this->dataPtr->currentVelocityTopic);

  // Subscribe to the stratified ocean current database
  this->dataPtr->databaseSub =
    this->ros_node_->create_subscription<dave_interfaces::msg::StratifiedCurrentDatabase>(
      this->dataPtr->transientCurrentVelocityTopic, 10,
      std::bind(&OceanCurrentModelPlugin::UpdateDatabase, this, std::placeholders::_1));

  gzmsg << "Transient current model plugin loaded!" << std::endl;
}

/////////////////////////////////////////////////
void OceanCurrentModelPlugin::LoadCurrentVelocityParams(
  sdf::ElementPtr _sdf, gz::sim::EntityComponentManager & _ecm)
{
  // Read topic name of stratified ocean current from SDF
  sdf::ElementPtr currentVelocityParams;
  if (_sdf->HasElement("transient_current"))
  {
    currentVelocityParams = _sdf->GetElement("transient_current");
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

    // Read Gauss-Markov parameters
    // sdf::ElementPtr currentVelocityDirection;

    // initialize velocity_north_model parameters
    if (currentVelocityParams->HasElement("velocity_north"))
    {
      sdf::ElementPtr elem = currentVelocityParams->GetElement("velocity_north");
      if (elem->HasElement("mean"))
      {
        this->dataPtr->currentVelNorthModel.mean = 0.0;
      }
      if (elem->HasElement("mu"))
      {
        this->dataPtr->currentVelNorthModel.mu = 0.0;
      }
      if (elem->HasElement("noiseAmp"))
      {
        this->dataPtr->noiseAmp_North = elem->Get<double>("noiseAmp");
      }
      this->dataPtr->currentVelNorthModel.min =
        this->dataPtr->currentVelNorthModel.mean - this->dataPtr->noiseAmp_North;
      this->dataPtr->currentVelNorthModel.max =
        this->dataPtr->currentVelNorthModel.mean + this->dataPtr->noiseAmp_North;
      if (elem->HasElement("noiseFreq"))
      {
        this->dataPtr->noiseFreq_North = elem->Get<double>("noiseFreq");
      }
      this->dataPtr->currentVelNorthModel.noiseAmp = this->dataPtr->noiseFreq_North;
    }

    this->dataPtr->currentVelNorthModel.var = this->dataPtr->currentVelNorthModel.mean;
    gzmsg << "For vehicle " << this->dataPtr->modelName
          << " -> Current north-direction velocity [m/s] "
          << "Gauss-Markov process model:" << std::endl;
    this->dataPtr->currentVelNorthModel.Print();

    // initialize velocity_east_model parameters
    if (currentVelocityParams->HasElement("velocity_east"))
    {
      sdf::ElementPtr elem = currentVelocityParams->GetElement("velocity_east");
      if (elem->HasElement("mean"))
      {
        this->dataPtr->currentVelEastModel.mean = 0.0;
      }
      if (elem->HasElement("mu"))
      {
        this->dataPtr->currentVelEastModel.mu = 0.0;
      }
      if (elem->HasElement("noiseAmp"))
      {
        this->dataPtr->noiseAmp_East = elem->Get<double>("noiseAmp");
      }
      this->dataPtr->currentVelEastModel.min =
        this->dataPtr->currentVelEastModel.mean - this->dataPtr->noiseAmp_East;
      this->dataPtr->currentVelEastModel.max =
        this->dataPtr->currentVelEastModel.mean + this->dataPtr->noiseAmp_East;
      if (elem->HasElement("noiseFreq"))
      {
        this->dataPtr->noiseFreq_East = elem->Get<double>("noiseFreq");
      }
      this->dataPtr->currentVelEastModel.noiseAmp = this->dataPtr->noiseFreq_East;
    }

    this->dataPtr->currentVelEastModel.var = this->dataPtr->currentVelEastModel.mean;
    gzmsg << "For vehicle " << this->dataPtr->modelName
          << " -> Current east-direction velocity [m/s] "
          << "Gauss-Markov process model:" << std::endl;
    this->dataPtr->currentVelEastModel.Print();

    // initialize velocity_down_model parameters
    if (currentVelocityParams->HasElement("velocity_down"))
    {
      sdf::ElementPtr elem = currentVelocityParams->GetElement("velocity_down");
      if (elem->HasElement("mean"))
      {
        this->dataPtr->currentVelDownModel.mean = 0.0;
      }
      if (elem->HasElement("mu"))
      {
        this->dataPtr->currentVelDownModel.mu = 0.0;
      }
      if (elem->HasElement("noiseAmp"))
      {
        this->dataPtr->noiseAmp_Down = elem->Get<double>("noiseAmp");
      }
      this->dataPtr->currentVelDownModel.min =
        this->dataPtr->currentVelDownModel.mean - this->dataPtr->noiseAmp_Down;
      this->dataPtr->currentVelDownModel.max =
        this->dataPtr->currentVelDownModel.mean + this->dataPtr->noiseAmp_Down;
      if (elem->HasElement("noiseFreq"))
      {
        this->dataPtr->noiseFreq_Down = elem->Get<double>("noiseFreq");
      }
      this->dataPtr->currentVelDownModel.noiseAmp = this->dataPtr->noiseFreq_Down;
    }

    this->dataPtr->currentVelDownModel.var = this->dataPtr->currentVelDownModel.mean;
    gzmsg << "For vehicle " << this->dataPtr->modelName
          << " -> Current down-direction velocity [m/s]"
          << "Gauss-Markov process model:" << std::endl;
    this->dataPtr->currentVelDownModel.Print();

    this->dataPtr->currentVelNorthModel.lastUpdate = this->dataPtr->lastUpdate.count();
    this->dataPtr->currentVelEastModel.lastUpdate = this->dataPtr->lastUpdate.count();
    this->dataPtr->currentVelDownModel.lastUpdate = this->dataPtr->lastUpdate.count();

    // Tidal Oscillation
    if (_sdf->HasElement("tide_oscillation"))
    {
      this->dataPtr->tideFlag = _sdf->Get<bool>("tide_oscillation");
    }
    else
    {
      this->dataPtr->tideFlag = false;
    }
  }
}

void OceanCurrentModelPlugin::UpdateDatabase(
  const std::shared_ptr<const dave_interfaces::msg::StratifiedCurrentDatabase> & _msg)
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
    if (_msg->tideconstituents == true)
    {
      this->dataPtr->M2_amp = _msg->m2_amp;
      this->dataPtr->M2_phase = _msg->m2_phase;
      this->dataPtr->M2_speed = _msg->m2_speed;
      this->dataPtr->S2_amp = _msg->s2_amp;
      this->dataPtr->S2_phase = _msg->s2_phase;
      this->dataPtr->S2_speed = _msg->s2_speed;
      this->dataPtr->N2_amp = _msg->n2_amp;
      this->dataPtr->N2_phase = _msg->n2_phase;
      this->dataPtr->N2_speed = _msg->n2_speed;
      this->dataPtr->tide_Constituents = true;
    }
    else
    {
      std::array<int, 5> tmpDateVals;
      for (int i = 0; i < _msg->time_gmt_year.size(); i++)
      {
        tmpDateVals[0] = _msg->time_gmt_year[i];
        tmpDateVals[1] = _msg->time_gmt_month[i];
        tmpDateVals[2] = _msg->time_gmt_day[i];
        tmpDateVals[3] = _msg->time_gmt_hour[i];
        tmpDateVals[4] = _msg->time_gmt_minute[i];

        this->dataPtr->timeGMT.push_back(tmpDateVals);
        this->dataPtr->tideVelocities.push_back(_msg->tidevelocities[i]);
      }
      this->dataPtr->tide_Constituents = false;
    }
    this->dataPtr->ebbDirection = _msg->ebb_direction;
    this->dataPtr->floodDirection = _msg->flood_direction;
    this->dataPtr->world_start_time[0] = _msg->world_start_time_year;
    this->dataPtr->world_start_time[1] = _msg->world_start_time_month;
    this->dataPtr->world_start_time[2] = _msg->world_start_time_day;
    this->dataPtr->world_start_time[3] = _msg->world_start_time_hour;
    this->dataPtr->world_start_time[4] = _msg->world_start_time_minute;
  }

  this->dataPtr->lock_.unlock();
}

//////////////////////////////////////////
gz::sim::Entity OceanCurrentModelPlugin::GetModelEntity(
  const std::string & modelName, gz::sim::EntityComponentManager & ecm)
{
  gz::sim::Entity modelEntity = gz::sim::kNullEntity;

  ecm.Each<gz::sim::components::Name>(
    [&](const gz::sim::Entity & entity, const gz::sim::components::Name * nameComp) -> bool
    {
      if (nameComp->Data() == modelName)
      {
        modelEntity = entity;
        return false;  // Stop iteration
      }
      return true;  // Continue iteration
    });

  return modelEntity;
}

/////////////////////////////////////////////////
gz::math::Pose3d OceanCurrentModelPlugin::GetModelPose(
  const gz::sim::Entity & modelEntity, gz::sim::EntityComponentManager & ecm)
{
  const auto * poseComp = ecm.Component<gz::sim::components::Pose>(modelEntity);
  if (poseComp)
  {
    return poseComp->Data();
  }
  else
  {
    gzerr << "Pose component not found for entity: " << modelEntity << std::endl;
    return gz::math::Pose3d::Zero;
  }
}

// ----------------------------------------------

/////////////////////////////////////////////////
void OceanCurrentModelPlugin::Update(
  const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm)
{
  // Update vehicle position
  gz::math::Pose3d vehicle_pos = GetModelPose(this->dataPtr->modelEntity, _ecm);
  double vehicleDepth = std::abs(vehicle_pos.Z());
  this->dataPtr->time = this->dataPtr->lastUpdate;
  CalculateOceanCurrent(vehicleDepth);
}

/////////////////////////////////////////////////
void OceanCurrentModelPlugin::CalculateOceanCurrent(double vehicleDepth)
{
  this->dataPtr->lock_.lock();

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
        this->dataPtr->tide.Update((this->dataPtr->time).count(), northCurrent);
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
    this->dataPtr->currentVelNorthModel.max =
      this->dataPtr->currentVelNorthModel.mean + this->dataPtr->noiseAmp_North;
    this->dataPtr->currentVelNorthModel.min =
      this->dataPtr->currentVelNorthModel.mean - this->dataPtr->noiseAmp_North;
    this->dataPtr->currentVelEastModel.max =
      this->dataPtr->currentVelEastModel.mean + this->dataPtr->noiseAmp_East;
    this->dataPtr->currentVelEastModel.min =
      this->dataPtr->currentVelEastModel.mean - this->dataPtr->noiseAmp_East;
    this->dataPtr->currentVelDownModel.max =
      this->dataPtr->currentVelDownModel.mean + this->dataPtr->noiseAmp_Down;
    this->dataPtr->currentVelDownModel.min =
      this->dataPtr->currentVelDownModel.mean - this->dataPtr->noiseAmp_Down;

    // Assign values to the model
    this->dataPtr->currentVelNorthModel.var = this->dataPtr->currentVelNorthModel.mean;
    this->dataPtr->currentVelEastModel.var = this->dataPtr->currentVelEastModel.mean;
    this->dataPtr->currentVelDownModel.var = this->dataPtr->currentVelDownModel.mean;

    // Update current velocity
    double velocityNorth =
      this->dataPtr->currentVelNorthModel.Update((this->dataPtr->time).count());

    // Update current horizontal direction around z axis of flow frame
    double velocityEast = this->dataPtr->currentVelEastModel.Update((this->dataPtr->time).count());

    // Update current horizontal direction around z axis of flow frame
    double velocityDown = this->dataPtr->currentVelDownModel.Update((this->dataPtr->time).count());

    // Update current Velocity
    this->dataPtr->currentVelocity = gz::math::Vector3d(velocityNorth, velocityEast, velocityDown);
  }

  this->dataPtr->lock_.unlock();
}

// ----------------------------------------------

/////////////////////////////////////////////////
void OceanCurrentModelPlugin::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  // Publish the Current Velocity. TODO find if we really need this
  // if (!_info.paused && _info.simTime > this->dataPtr->lastUpdate)
  // {
  //   this->dataPtr->lastUpdate = _info.simTime;
  //   PostPublishCurrentVelocity();
  // }
  this->dataPtr->lastUpdate = _info.simTime;
  PublishCurrentVelocity(_info);
  if (!_info.paused)
  {
    rclcpp::spin_some(this->ros_node_);

    if (_info.iterations % 1000 == 0)
    {
      gzmsg << "dave_gz_model_plugins::OceanCurrentModelPlugin::PostUpdate" << std::endl;
    }
  }
}

/////////////////////////////////////////////////
void OceanCurrentModelPlugin::PublishCurrentVelocity(const gz::sim::UpdateInfo & _info)
{
  geometry_msgs::msg::TwistStamped flowVelMsg;
  flowVelMsg.header.stamp.sec =
    std::chrono::duration_cast<std::chrono::seconds>(_info.simTime).count();
  flowVelMsg.header.frame_id = "/world";
  flowVelMsg.twist.linear.x = this->dataPtr->currentVelocity.X();
  flowVelMsg.twist.linear.y = this->dataPtr->currentVelocity.Y();
  flowVelMsg.twist.linear.z = this->dataPtr->currentVelocity.Z();
  this->dataPtr->flowVelocityPub->publish(flowVelMsg);

  // Generate and publish Gazebo topic according to the vehicle depth
  gz::msgs::Vector3d currentVel;
  currentVel.set_x(this->dataPtr->currentVelocity.X());
  currentVel.set_y(this->dataPtr->currentVelocity.Y());
  currentVel.set_z(this->dataPtr->currentVelocity.Z());
  this->dataPtr->gz_current_vel_pub.Publish(currentVel);
}

}  // namespace dave_gz_model_plugins

// Notes -
// 1. Relplace "TransientCurrentPlugin" with OceanCurrentModelPlugin
// 2.