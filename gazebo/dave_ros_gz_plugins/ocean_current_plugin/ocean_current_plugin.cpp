#include <memory>
#include <ocean_current_plugin.hh>
#include <vector>

#include <dave_ros_gz_plugins/GetCurrentModel.h>
#include <dave_ros_gz_plugins/GetOriginSphericalCoord.h>
#include <dave_ros_gz_plugins/SetCurrentDirection.h>
#include <dave_ros_gz_plugins/SetCurrentModel.h>
#include <dave_ros_gz_plugins/SetCurrentVelocity.h>
#include <dave_ros_gz_plugins/SetOriginSphericalCoord.h>
#include <dave_ros_gz_plugins/SetStratifiedCurrentDirection.h>
#include <dave_ros_gz_plugins/SetStratifiedCurrentVelocity.h>
#include <dave_ros_gz_plugins/StratifiedCurrentDatabase.h>
#include <dave_ros_gz_plugins/StratifiedCurrentVelocity.h>
#include <geometry_msgs/TwistStamped.h>
#include <geometry_msgs/Vector3.h>
#include <ros/package.h>
#include <ros/ros.h>
#include <ament_index_cpp/get_package_share_directory.hpp>

#include <boost/shared_ptr.hpp>
#include <gazebo/common/Plugin.hh>
#include <gazebo/physics/World.hh>

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <chrono>
#include <gazebo/common/Plugin.hh>
#include <gazebo/common/Time.hh>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>

#include <gazebo/physics/PhysicsEngine.hh>
#include <std_msgs/msg/string.hpp>

#include <algorithm>
#include <functional>

// #include "dave_ros_gz_plugins/msg/StratifiedCurrentVelocity.hpp"
// #include "dave_ros_gz_plugins/msg/StratifiedCurrentDatabase.hpp"
// #include "dave_ros_gz_plugins/srv/SetCurrentModel.hpp"
// #include "dave_ros_gz_plugins/srv/GetCurrentModel.hpp"
// #include "dave_ros_gz_plugins/srv/SetCurrentVelocity.hpp"
// #include "dave_ros_gz_plugins/srv/SetStratifiedCurrentVelocity.hpp"
// #include "dave_ros_gz_plugins/srv/SetCurrentAngle.hpp"
// #include "dave_ros_gz_plugins/srv/SetStratifiedCurrentAngle.hpp"

// Things to consider/decide for now -
// 1. Is consistency being maintained while the use of "gzerr" and "RCLCPP_ERROR" for error logging
// ?
// 2. Do the Headers included in the code exist ?
// 3. use of gz_bridge and message types.
// 4. Integration with the rest of the system with respect to the use of smart pointers and other
// parameters like behaviour.

using namespace gz;
using namespace sim;
using namespace systems;
using namespace dave_simulator_ros;

class gz::sim::systems::dave_simulator_ros::UnderwaterCurrentROSPlugin
{
public:
  UnderwaterCurrentROSPlugin();
  ~UnderwaterCurrentROSPlugin();

private:
  rclcpp::Node::SharedPtr rosNode;  // This is a smart pointer to a ROS 2 node, facilitating
                                    // communication with other ROS nodes.
  rclcpp::TimerBase::SharedPtr rosPublishTimer;
  gz::common::Time rosPublishPeriod;
  gz::common::Time lastRosPublishTime;
  std::string db_path;
};

/////////////////////////////////////////////////
UnderwaterCurrentROSPlugin::UnderwaterCurrentROSPlugin()
{
  this->rosPublishPeriod = gz::common::Time(0.05);
  this->lastRosPublishTime = gz::common::Time(0.0);
  this->db_path =
    ament_index_cpp::get_package_share_directory("dave_worlds");  // might need to be (updated)

  // Initialize the ROS 2 node
  this->rosNode = std::make_shared<rclcpp::Node>("underwater_current_ros_plugin");

  // Setting up a timer in ROS 2
  this->rosPublishTimer = this->rosNode->create_wall_timer(
    std::chrono::milliseconds(static_cast<int>(this->rosPublishPeriod.Double() * 1000)),
    [this]() { this->PublishCurrentInfo(); });
}

/////////////////////////////////////////////////
// this can be removed as the connections are now handled by samrt pointers (to be (updated))
UnderwaterCurrentROSPlugin::~UnderwaterCurrentROSPlugin()
{
  this->rosNode.reset();  // Properly handle ROS 2 node shutdown
}

/////////////////////////////////////////////////
void UnderwaterCurrentROSPlugin::Load(gz::sim::WorldPtr _world, sdf::ElementPtr _sdf)
{  // make sure this is closed
  try
  {
    UnderwaterCurrentPlugin::Load(_world, _sdf);
    this->stratifiedCurrentVelocityDatabaseTopic =
      this->stratifiedCurrentVelocityTopic + "_database";
  }
  catch (const gz::common::Exception & e)  // this is a gazebo error hence -gzerr, check for "_e" or
                                           // "e" (to be (updated))
                                           // findings- we can use both "_e" and "e" but "e" is now
                                           // the conventional way to display errors in C++. further
                                           // writing "&e" or "& e" is a personal preference.
  {
    gzerr << "Error loading plugin: " << e.what() << '\n';
    return;
  }

  if (!rclcpp::ok())  // this is a ROS 2 error hence -RCLCPP_ERROR to be (considered) (updated)
  {
    gzerr << "ROS 2 has not been properly initialized. Please make sure you have initialized your "
             "ROS 2 environment.\n";
    return;
  }

  this->ns = _sdf->HasElement("namespace") ? _sdf->Get<std::string>("namespace") : "";
  this->rosNode = std::make_shared<rclcpp::Node>("underwater_current_ros_plugin", this->ns);

  // Create and advertise Messages
  // Advertise the flow velocity as a stamped twist message
  this->flowVelocityPub = this->rosNode->create_publisher<geometry_msgs::msg::TwistStamped>(
    this->currentVelocityTopic, rclcpp::QoS(10));

  // Advertise the stratified ocean current message
  this->stratifiedCurrentVelocityPub =
    this->rosNode->create_publisher<dave_ros_gz_plugins::msg::StratifiedCurrentVelocity>(
      this->stratifiedCurrentVelocityTopic, rclcpp::QoS(10));

  // Advertise the stratified ocean current database message
  this->stratifiedCurrentDatabasePub =
    this->rosNode->create_publisher<dave_ros_gz_plugins::msg::StratifiedCurrentDatabase>(
      this->stratifiedCurrentVelocityDatabaseTopic, rclcpp::QoS(10));

  // Create and advertise services
  // In this setup : std::placeholders::_1 and std::placeholders::_2 are used to represent the
  // request and response objects that the service callback expects to receive. This allows the
  // serviceCallback method of <MyClass> to be used directly as a ROS 2 service handler without
  // having to wrap it in another function that manually passes the request and response.

  // check for the number of arg being passed to the function

  // Advertise the service to update the current velocity model
  this->worldServices["set_current_velocity_model"] =
    this->rosNode->create_service<dave_ros_gz_plugins::srv::SetCurrentModel>(
      "set_current_velocity_model", std::bind(
                                      &UnderwaterCurrentROSPlugin::UpdateCurrentVelocityModel, this,
                                      std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to get the current velocity model
  this->worldServices["get_current_velocity_model"] =
    this->rosNode->create_service<dave_ros_gz_plugins::srv::GetCurrentModel>(
      "get_current_velocity_model", std::bind(
                                      &UnderwaterCurrentROSPlugin::GetCurrentVelocityModel, this,
                                      std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to update the current horizontal angle model
  this->worldServices["set_current_horz_angle_model"] =
    this->rosNode->create_service<dave_ros_gz_plugins::srv::SetCurrentModel>(
      "set_current_horz_angle_model", std::bind(
                                        &UnderwaterCurrentROSPlugin::UpdateCurrentHorzAngleModel,
                                        this, std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to get the current horizontal angle model
  this->worldServices["get_current_horz_angle_model"] =
    this->rosNode->create_service<dave_ros_gz_plugins::srv::GetCurrentModel>(
      "get_current_horz_angle_model", std::bind(
                                        &UnderwaterCurrentROSPlugin::GetCurrentHorzAngleModel, this,
                                        std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to update the current vertical angle model
  this->worldServices["set_current_vert_angle_model"] =
    this->rosNode->create_service<dave_ros_gz_plugins::srv::SetCurrentModel>(
      "set_current_vert_angle_model", std::bind(
                                        &UnderwaterCurrentROSPlugin::UpdateCurrentVertAngleModel,
                                        this, std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to get the current vertical angle model
  this->worldServices["get_current_vert_angle_model"] =
    this->rosNode->create_service<dave_ros_gz_plugins::srv::GetCurrentModel>(
      "get_current_vert_angle_model", std::bind(
                                        &UnderwaterCurrentROSPlugin::GetCurrentHorzAngleModel, this,
                                        std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to update the current velocity mean value
  this->worldServices["set_current_velocity"] =
    this->rosNode->create_service<dave_ros_gz_plugins::srv::SetCurrentVelocity>(
      "set_current_velocity", std::bind(
                                &UnderwaterCurrentROSPlugin::UpdateCurrentVelocity, this,
                                std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to update the stratified current velocity
  this->worldServices["set_stratified_current_velocity"] =
    this->rosNode->create_service<dave_ros_gz_plugins::srv::SetStratifiedCurrentVelocity>(
      "set_stratified_current_velocity", std::bind(
                                           &UnderwaterCurrentROSPlugin::UpdateStratCurrentVelocity,
                                           this, std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to update the current horizontal angle
  this->worldServices["set_current_horz_angle"] =
    this->rosNode->create_service<dave_ros_gz_plugins::srv::SetCurrentAngle>(
      "set_current_horz_angle", std::bind(
                                  &UnderwaterCurrentROSPlugin::UpdateHorzAngle, this,
                                  std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to update the stratified current horizontal angle
  this->worldServices["set_stratified_current_horz_angle"] =
    this->rosNode->create_service<dave_ros_gz_plugins::srv::SetStratifiedCurrentAngle>(
      "set_stratified_current_horz_angle", std::bind(
                                             &UnderwaterCurrentROSPlugin::UpdateStratHorzAngle,
                                             this, std::placeholders::_1, std::placeholders::_2));
  // Advertise the service to update the current vertical angle
  this->worldServices["set_current_vert_angle"] =
    this->rosNode->create_service<dave_ros_gz_plugins::srv::SetCurrentAngle>(
      "set_current_vert_angle", std::bind(
                                  &UnderwaterCurrentROSPlugin::UpdateVertAngle, this,
                                  std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to update the stratified current vertical angle
  this->worldServices["set_stratified_current_vert_angle"] =
    this->rosNode->create_service<dave_ros_gz_plugins::srv::SetStratifiedCurrentAngle>(
      "set_stratified_current_vert_angle", std::bind(
                                             &UnderwaterCurrentROSPlugin::UpdateStratVertAngle,
                                             this, std::placeholders::_1, std::placeholders::_2));

  // Connect to the Gazebo Harmonic update event
  this->rosPublishConnection = _world->OnUpdateBegin([this](const gz::sim::UpdateInfo & info)
                                                     { this->OnUpdateCurrentVel(info); });
}

/////////////////////////////////////////////////
void UnderwaterCurrentROSPlugin::OnUpdateCurrentVel()
{
  if (this->lastUpdate - this->lastRosPublishTime >= this->rosPublishPeriod)
  {
    // Update the time tracking for publication
    this->lastRosPublishTime = this->lastUpdate;

    // Generate and publish current velocity according to the vehicle depth
    geometry_msgs::msg::TwistStamped flowVelMsg;
    flowVelMsg.header.stamp = this->rosNode->get_clock()->now();
    flowVelMsg.header.frame_id =
      "world";  // Changed from "/world" to be consistent with ROS 2 TF2 conventions

    flowVelMsg.twist.linear.x = this->currentVelocity.X();
    flowVelMsg.twist.linear.y = this->currentVelocity.Y();
    flowVelMsg.twist.linear.z = this->currentVelocity.Z();

    this->flowVelocityPub->publish(flowVelMsg);

    // Generate and publish stratified current velocity
    dave_ros_gz_plugins::msg::StratifiedCurrentVelocity stratCurrentVelocityMsg;
    stratCurrentVelocityMsg.header.stamp = this->rosNode->get_clock()->now();
    stratCurrentVelocityMsg.header.frame_id = "world";

    // Updating for stratified behaviour of Ocean Currents
    // What is the .size value over here, to be (updated)(checked)
    for (size_t i = 0; i < this->currentStratifiedVelocity.size(); i++)
    {
      geometry_msgs::msg::Vector3 velocity;
      velocity.x = this->currentStratifiedVelocity[i].X();
      velocity.y = this->currentStratifiedVelocity[i].Y();
      velocity.z = this->currentStratifiedVelocity[i].Z();
      stratCurrentVelocityMsg.velocities.push_back(velocity);
      stratCurrentVelocityMsg.depths.push_back(this->currentStratifiedVelocity[i].W());
    }
    // updated till here
    this->stratifiedCurrentVelocityPub->publish(stratCurrentVelocityMsg);

    // Generate and publish stratified current database
    dave_ros_gz_plugins::msg::StratifiedCurrentDatabase currentDatabaseMsg;
    for (int i = 0; i < this->stratifiedDatabase.size(); i++)
    {
      // Stratified current database entry preparation
      geometry_msgs::msg::Vector3 velocity;
      velocity.x = this->stratifiedDatabase[i].X();
      velocity.y = this->stratifiedDatabase[i].Y();
      velocity.z = 0.0;  // Assuming z is intentionally set to 0.0
      currentDatabaseMsg.velocities.push_back(velocity);
      currentDatabaseMsg.depths.push_back(this->stratifiedDatabase[i].Z());
    }

    if (this->tidalHarmonicFlag)
    {
      // Tidal harmonic constituents
      currentDatabaseMsg.M2amp = this->M2_amp;
      currentDatabaseMsg.M2phase = this->M2_phase;
      currentDatabaseMsg.M2speed = this->M2_speed;
      currentDatabaseMsg.S2amp = this->S2_amp;
      currentDatabaseMsg.S2phase = this->S2_phase;
      currentDatabaseMsg.S2speed = this->S2_speed;
      currentDatabaseMsg.N2amp = this->N2_amp;
      currentDatabaseMsg.N2phase = this->N2_phase;
      currentDatabaseMsg.N2speed = this->N2_speed;
      currentDatabaseMsg.tideConstituents = true;
    }
    else
    {
      for (int i = 0; i < this->dateGMT.size(); i++)
      {
        // Tidal oscillation database
        currentDatabaseMsg.timeGMTYear.push_back(this->dateGMT[i][0]);
        currentDatabaseMsg.timeGMTMonth.push_back(this->dateGMT[i][1]);
        currentDatabaseMsg.timeGMTDay.push_back(this->dateGMT[i][2]);
        currentDatabaseMsg.timeGMTHour.push_back(this->dateGMT[i][3]);
        currentDatabaseMsg.timeGMTMinute.push_back(this->dateGMT[i][4]);

        currentDatabaseMsg.tideVelocities.push_back(this->speedcmsec[i]);
      }
      currentDatabaseMsg.tideConstituents = false;
    }

    currentDatabaseMsg.ebbDirection = this->ebbDirection;
    currentDatabaseMsg.floodDirection = this->floodDirection;

    currentDatabaseMsg.worldStartTimeYear = this->world_start_time_year;
    currentDatabaseMsg.worldStartTimeMonth = this->world_start_time_month;
    currentDatabaseMsg.worldStartTimeDay = this->world_start_time_day;
    currentDatabaseMsg.worldStartTimeHour = this->world_start_time_hour;
    currentDatabaseMsg.worldStartTimeMinute = this->world_start_time_minute;

    this->stratifiedCurrentDatabasePub->publish(currentDatabaseMsg);
  }
}
// updated till here

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateHorzAngle(
  const std::shared_ptr<dave_ros_gz_plugins::srv::SetCurrentDirection::Request> _req,
  std::shared_ptr<dave_ros_gz_plugins::srv::SetCurrentDirection::Response> _res)
{
  _res->success = this->currentHorzAngleModel.SetMean(_req->angle);

  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateStratHorzAngle(
  const std::shared_ptr<dave_ros_gz_plugins::srv::SetStratifiedCurrentDirection::Request> _req,
  std::shared_ptr<dave_ros_gz_plugins::srv::SetStratifiedCurrentDirection::Response> _res)
{
  if (_req->layer >= this->stratifiedDatabase.size())
  {
    _res->success = false;
    return true;
  }

  _res->success = this->stratifiedCurrentModels[_req->layer][1].SetMean(_req->angle);
  if (_res->success)
  {
    // Update the database values (new angle, unchanged velocity)
    double velocity =
      hypot(this->stratifiedDatabase[_req->layer].X(), this->stratifiedDatabase[_req->layer].Y());
    this->stratifiedDatabase[_req->layer].X() = cos(_req->angle) * velocity;
    this->stratifiedDatabase[_req->layer].Y() = sin(_req->angle) * velocity;
  }
  return true;
}
// -updated till here td
/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateVertAngle(
  const std::shared_ptr<dave_ros_gz_plugins::srv::SetCurrentDirection::Request> _req,
  std::shared_ptr<dave_ros_gz_plugins::srv::SetCurrentDirection::Response> _res)
{
  _res->success = this->currentVertAngleModel.SetMean(_req->angle);
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateStratVertAngle(
  const std::shared_ptr<dave_ros_gz_plugins::srv::SetStratifiedCurrentDirection::Request> _req,
  std::shared_ptr<dave_ros_gz_plugins::srv::SetStratifiedCurrentDirection::Response> _res)
{
  if (_req->layer >= this->stratifiedDatabase.size())
  {
    _res->success = false;
    return true;
  }
  _res->success = this->stratifiedCurrentModels[_req->layer][2].SetMean(_req->angle);
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateCurrentVelocity(
  const std::shared_ptr<dave_ros_gz_plugins::srv::SetCurrentVelocity::Request> _req,
  std::shared_ptr<dave_ros_gz_plugins::srv::SetCurrentVelocity::Response> _res)
{
  if (
    this->currentVelModel.SetMean(_req->velocity) &&
    this->currentHorzAngleModel.SetMean(_req->horizontal_angle) &&
    this->currentVertAngleModel.SetMean(_req->vertical_angle))
  {
    RCLCPP_INFO(
      this->rosNode->get_logger(),
      "Current velocity [m/s] = %f\nCurrent horizontal angle [rad] = %f\nCurrent vertical angle "
      "[rad] = %f\n\tWARNING: Current velocity calculated in the ENU frame",
      _req->velocity, _req->horizontal_angle, _req->vertical_angle);
    _res->success = true;
  }
  else
  {
    RCLCPP_ERROR(this->rosNode->get_logger(), "Error while updating the current velocity");
    _res->success = false;
  }
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateStratCurrentVelocity(
  const std::shared_ptr<dave_ros_gz_plugins::srv::SetStratifiedCurrentVelocity::Request> _req,
  std::shared_ptr<dave_ros_gz_plugins::srv::SetStratifiedCurrentVelocity::Response> _res)
{
  if (_req->layer >= this->stratifiedDatabase.size())
  {
    _res->success = false;
    return true;
  }
  if (
    this->stratifiedCurrentModels[_req->layer][0].SetMean(_req->velocity) &&
    this->stratifiedCurrentModels[_req->layer][1].SetMean(_req->horizontal_angle) &&
    this->stratifiedCurrentModels[_req->layer][2].SetMean(_req->vertical_angle))
  {
    // Update the database values as well
    this->stratifiedDatabase[_req->layer].X() = cos(_req->horizontal_angle) * _req->velocity;
    this->stratifiedDatabase[_req->layer].Y() = sin(_req->horizontal_angle) * _req->velocity;
    RCLCPP_INFO(
      this->rosNode->get_logger(),
      "Layer %d current velocity [m/s] = %f\n  Horizontal angle [rad] = %f\n  Vertical angle [rad] "
      "= %f\n\tWARNING: Current velocity calculated in the ENU frame",
      _req->layer, _req->velocity, _req->horizontal_angle, _req->vertical_angle);
    _res->success = true;
  }
  else
  {
    RCLCPP_ERROR(this->rosNode->get_logger(), "Error while updating the current velocity");
    _res->success = false;
  }
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::GetCurrentVelocityModel(
  const std::shared_ptr<dave_ros_gz_plugins::srv::GetCurrentModel::Request> _req,
  std::shared_ptr<dave_ros_gz_plugins::srv::GetCurrentModel::Response> _res)
{
  _res->mean = this->currentVelModel.mean;
  _res->min = this->currentVelModel.min;
  _res->max = this->currentVelModel.max;
  _res->noise = this->currentVelModel.noiseAmp;
  _res->mu = this->currentVelModel.mu;
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::GetCurrentHorzAngleModel(
  const std::shared_ptr<dave_ros_gz_plugins::srv::GetCurrentModel::Request> _req,
  std::shared_ptr<dave_ros_gz_plugins::srv::GetCurrentModel::Response> _res)
{
  _res->mean = this->currentHorzAngleModel.mean;
  _res->min = this->currentHorzAngleModel.min;
  _res->max = this->currentHorzAngleModel.max;
  _res->noise = this->currentHorzAngleModel.noiseAmp;
  _res->mu = this->currentHorzAngleModel.mu;
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::GetCurrentVertAngleModel(
  const std::shared_ptr<dave_ros_gz_plugins::srv::GetCurrentModel::Request> _req,
  std::shared_ptr<dave_ros_gz_plugins::srv::GetCurrentModel::Response> _res)
{
  _res->mean = this->currentVertAngleModel.mean;
  _res->min = this->currentVertAngleModel.min;
  _res->max = this->currentVertAngleModel.max;
  _res->noise = this->currentVertAngleModel.noiseAmp;
  _res->mu = this->currentVertAngleModel.mu;
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateCurrentVelocityModel(
  const std::shared_ptr<dave_ros_gz_plugins::srv::SetCurrentModel::Request> _req,
  std::shared_ptr<dave_ros_gz_plugins::srv::SetCurrentModel::Response> _res)
{
  _res->success = this->currentVelModel.SetModel(
    std::max(0.0, _req->mean), std::min(0.0, _req->min), std::max(0.0, _req->max), _req->mu,
    _req->noise);

  for (int i = 0; i < this->stratifiedCurrentModels.size(); i++)
  {
    gz::GaussMarkovProcess & model = this->stratifiedCurrentModels[i][0];
    model.SetModel(
      model.mean, std::max(0.0, _req->min), std::max(0.0, _req->max), _req->mu, _req->noise);
  }

  RCLCPP_INFO(
    this->rosNode->get_logger(),
    "Current velocity model updated\n\tWARNING: Current velocity calculated in the ENU frame");

  // Assuming currentVelModel.Print() prints model details, you would instead log these details
  // For instance, you could log mean, min, max etc.
  // Example of logging additional model details:
  RCLCPP_INFO(
    this->rosNode->get_logger(), "Model details: Mean: %f, Min: %f, Max: %f",
    this->currentVelModel.mean, this->currentVelModel.min, this->currentVelModel.max);

  return true;

  // gzmsg << "Current velocity model updated" << std::endl
  //       << "\tWARNING: Current velocity calculated in the ENU frame" << std::endl;
  // this->currentVelModel.Print();
  // return true;
}
// -updated till here td
/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateCurrentHorzAngleModel(
  const std::shared_ptr<dave_ros_gz_plugins::srv::SetCurrentModel::Request> _req,
  std::shared_ptr<dave_ros_gz_plugins::srv::SetCurrentModel::Response> _res)
{
  _res->success =
    this->currentHorzAngleModel.SetModel(_req->mean, _req->min, _req->max, _req->mu, _req->noise);

  for (int i = 0; i < this->stratifiedCurrentModels.size(); i++)
  {
    gz::GaussMarkovProcess & model = this->stratifiedCurrentModels[i][1];
    model.SetModel(
      model.mean, std::max(-M_PI, _req->min), std::min(M_PI, _req->max), _req->mu, _req->noise);
  }

  RCLCPP_INFO(
    this->rosNode->get_logger(),
    "Horizontal angle model updated\n\tWARNING: Current velocity calculated in the ENU frame");

  // Assuming currentHorzAngleModel.Print() prints model details, you would instead log these
  // details Example of logging additional model details:
  RCLCPP_INFO(
    this->rosNode->get_logger(), "Model details: Mean: %f, Min: %f, Max: %f",
    this->currentHorzAngleModel.mean, this->currentHorzAngleModel.min,
    this->currentHorzAngleModel.max);

  return true;

  // gzmsg << "Horizontal angle model updated" << std::endl
  //       << "\tWARNING: Current velocity calculated in the ENU frame" << std::endl;
  // this->currentHorzAngleModel.Print();
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateCurrentVertAngleModel(
  const std::shared_ptr<dave_ros_gz_plugins::srv::SetCurrentModel::Request> _req,
  std::shared_ptr<dave_ros_gz_plugins::srv::SetCurrentModel::Response> _res)
{
  _res->success =
    this->currentVertAngleModel.SetModel(_req->mean, _req->min, _req->max, _req->mu, _req->noise);

  RCLCPP_INFO(
    this->rosNode->get_logger(),
    "Vertical angle model updated\n\tWARNING: Current velocity calculated in the ENU frame");

  // Assuming currentVertAngleModel.Print() prints model details, you would instead log these
  // details Example of logging additional model details:
  RCLCPP_INFO(
    this->rosNode->get_logger(), "Model details: Mean: %f, Min: %f, Max: %f, Mu: %f, Noise: %f",
    this->currentVertAngleModel.mean, this->currentVertAngleModel.min,
    this->currentVertAngleModel.max, this->currentVertAngleModel.mu,
    this->currentVertAngleModel.noiseAmp);
  return true;
  // gzmsg << "Vertical angle model updated" << std::endl
  //       << "\tWARNING: Current velocity calculated in the ENU frame" << std::endl;
  // this->currentVertAngleModel.Print();
}

/////////////////////////////////////////////////
GZ_REGISTER_WORLD_PLUGIN(UnderwaterCurrentROSPlugin)

// #endif