#include "dave_ros_gz_plugins/ocean_current_plugin.hh"
#include <vector>

#include <boost/shared_ptr.hpp>
#include <gz/physics/World.hh>

#include <chrono>
#include <geometry_msgs/msg/twist_stamped.hpp>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <algorithm>
#include <functional>

#include <gz/common/Exception.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/System.hh>
#include <gz/sim/World.hh>
#include "gz/plugin/Register.hh"
#include "gz/sim/components/World.hh"

// Things to consider/decide for now -
// 1. Is consistency being maintained while the use of "gzerr" and "RCLCPP_ERROR" for error logging
// ?
// 2. Do the Headers included in the code exist ?
// 3. use of gz_bridge and message types.
// 4. Integration with the rest of the system with respect to the use of smart pointers and other
// parameters like behaviour.

GZ_ADD_PLUGIN(
  dave_ros_gz_plugins::UnderwaterCurrentROSPlugin, gz::sim::System,
  dave_ros_gz_plugins::UnderwaterCurrentROSPlugin::ISystemConfigure,
  dave_ros_gz_plugins::UnderwaterCurrentROSPlugin::ISystemPostUpdate)

namespace dave_ros_gz_plugins
{
struct UnderwaterCurrentROSPlugin::PrivateData
{
  // std::string db_path;  (check)(TODO)
  // this->dataPtr->db_path = ament_index_cpp::get_package_share_directory("dave_worlds");
  std::chrono::steady_clock::duration lastUpdate{0};
  // // rclcpp::Service<dave_interfaces::srv::>::SharedPtr;  //Template
  rclcpp::Service<dave_interfaces::srv::GetCurrentModel>::SharedPtr get_current_velocity_model;
  rclcpp::Service<dave_interfaces::srv::GetCurrentModel>::SharedPtr get_current_horz_angle_model;
  rclcpp::Service<dave_interfaces::srv::GetCurrentModel>::SharedPtr get_current_vert_angle_model;
  rclcpp::Service<dave_interfaces::srv::SetCurrentModel>::SharedPtr set_current_velocity_model;
  rclcpp::Service<dave_interfaces::srv::SetCurrentModel>::SharedPtr set_current_horz_angle_model;
  rclcpp::Service<dave_interfaces::srv::SetCurrentModel>::SharedPtr set_current_vert_angle_model;
  rclcpp::Service<dave_interfaces::srv::SetCurrentVelocity>::SharedPtr set_current_velocity;
  rclcpp::Service<dave_interfaces::srv::SetStratifiedCurrentVelocity>::SharedPtr
    set_stratified_current_velocity;
  rclcpp::Service<dave_interfaces::srv::SetStratifiedCurrentDirection>::SharedPtr
    set_stratified_current_horz_angle_model;
  rclcpp::Service<dave_interfaces::srv::SetStratifiedCurrentDirection>::SharedPtr
    set_stratified_current_vert_angle_model;

  std::shared_ptr<rclcpp::Node> rosNode;
  gz::sim::World world;
  gz::sim::Model model;
  gz::sim::Entity entity;  // Added entity member
  std::string modelName;
  std::shared_ptr<const sdf::Element> sdf;
  std::string stratifiedCurrentVelocityTopic;
  std::string stratifiedCurrentVelocityDatabaseTopic;
  std::string currentVelocityTopic;
  std::string ns;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr flowVelocityPub;
  rclcpp::Publisher<dave_interfaces::msg::StratifiedCurrentVelocity>::SharedPtr
    stratifiedCurrentVelocityPub;
  rclcpp::Publisher<dave_interfaces::msg::StratifiedCurrentDatabase>::SharedPtr
    stratifiedCurrentDatabasePub;
};

/////////////////////////////////////////////////
UnderwaterCurrentROSPlugin::UnderwaterCurrentROSPlugin() : dataPtr(std::make_unique<PrivateData>())
{
}
UnderwaterCurrentROSPlugin::~UnderwaterCurrentROSPlugin()
{
  this->dataPtr->rosNode.reset();  // Properly handle ROS 2 node shutdown
}

/////////////////////////////////////////////////
void UnderwaterCurrentROSPlugin::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr)
{
  this->dataPtr->rosNode = std::make_shared<rclcpp::Node>("underwater_current_ros_plugin");
  auto worldEntity = _ecm.EntityByComponents(gz::sim::components::World());
  this->dataPtr->world = gz::sim::World(worldEntity);
  this->dataPtr->entity = _entity;
  this->dataPtr->model = gz::sim::Model(_entity);
  this->dataPtr->modelName = this->dataPtr->model.Name(_ecm);
  this->dataPtr->sdf = _sdf;
  std::chrono::steady_clock::duration lastUpdate{0};

  // auto lastUpdate = gz::sim::simTime;
  try
  {
    this->dataPtr->stratifiedCurrentVelocityDatabaseTopic =
      this->dataPtr->stratifiedCurrentVelocityTopic + "_database";
  }
  catch (const gz::common::Exception & e)
  // we can use both "_e" and "e" but "e" is now
  // the conventional way to display errors in C++.
  {
    gzerr << "Error loading plugin: " << e.what() << '\n';
    return;
  }

  if (!rclcpp::ok())
  {
    gzerr << "ROS 2 has not been properly initialized. Please make sure you have initialized your "
             "ROS 2 environment.\n";
    return;
  }

  this->dataPtr->ns = _sdf->HasElement("namespace") ? _sdf->Get<std::string>("namespace") : "";

  // Initialising ROS 2 node
  this->dataPtr->rosNode =
    std::make_shared<rclcpp::Node>("underwater_current_ros_plugin", this->dataPtr->ns);

  // Create and advertise Messages
  // Advertise the flow velocity as a stamped twist message
  this->dataPtr->flowVelocityPub =
    this->dataPtr->rosNode->create_publisher<geometry_msgs::msg::TwistStamped>(
      this->dataPtr->currentVelocityTopic, rclcpp::QoS(10));

  // Advertise the stratified ocean current message
  this->dataPtr->stratifiedCurrentVelocityPub =
    this->dataPtr->rosNode->create_publisher<dave_interfaces::msg::StratifiedCurrentVelocity>(
      this->dataPtr->stratifiedCurrentVelocityTopic, rclcpp::QoS(10));

  // Advertise the stratified ocean current database message
  this->dataPtr->stratifiedCurrentDatabasePub =
    this->dataPtr->rosNode->create_publisher<dave_interfaces::msg::StratifiedCurrentDatabase>(
      this->dataPtr->stratifiedCurrentVelocityDatabaseTopic, rclcpp::QoS(10));

  // Create and advertise services
  // In this setup : std::placeholders::_1 and std::placeholders::_2 are used to represent the
  // request and response objects that the service callback expects to receive. This allows the
  // serviceCallback method of <MyClass> to be used directly as a ROS 2 service handler without
  // having to wrap it in another function that manually passes the request and response.

  // check for the number of arg being passed to the function

  // Advertise the service to get the current velocity model
  this->dataPtr->get_current_velocity_model =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::GetCurrentModel>(
      "get_current_velocity_model", std::bind(
                                      &UnderwaterCurrentROSPlugin::GetCurrentVelocityModel, this,
                                      std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to get the current horizontal angle model
  this->dataPtr->get_current_horz_angle_model =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::GetCurrentModel>(
      "get_current_horz_angle_model", std::bind(
                                        &UnderwaterCurrentROSPlugin::GetCurrentHorzAngleModel, this,
                                        std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to get the current vertical angle model
  this->dataPtr->get_current_vert_angle_model =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::GetCurrentModel>(
      "get_current_vert_angle_model", std::bind(
                                        &UnderwaterCurrentROSPlugin::GetCurrentHorzAngleModel, this,
                                        std::placeholders::_1, std::placeholders::_2));

  // Connect to the Gazebo Harmonic update event
  // this->dataPtr->rosPublishConnection = _world->OnUpdateBegin([this](const gz::sim::UpdateInfo &
  // info)
  //                                                    { this->dataPtr->OnUpdateCurrentVel(info);
  //                                                    });
}

/////////////////////////////////////////////////
void UnderwaterCurrentROSPlugin::PreUpdate(
  const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm)
{
  // Advertise the service to get the current velocity model
  this->dataPtr->get_current_velocity_model =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::GetCurrentModel>(
      "get_current_velocity_model", std::bind(
                                      &UnderwaterCurrentROSPlugin::GetCurrentVelocityModel, this,
                                      std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to get the current horizontal angle model
  this->dataPtr->get_current_horz_angle_model =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::GetCurrentModel>(
      "get_current_horz_angle_model", std::bind(
                                        &UnderwaterCurrentROSPlugin::GetCurrentHorzAngleModel, this,
                                        std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to get the current vertical angle model
  this->dataPtr->get_current_vert_angle_model =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::GetCurrentModel>(
      "get_current_vert_angle_model", std::bind(
                                        &UnderwaterCurrentROSPlugin::GetCurrentVertAngleModel, this,
                                        std::placeholders::_1, std::placeholders::_2));
}
/////////////////////////////////////////////////
void UnderwaterCurrentROSPlugin::Update(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  if (_info.simTime > this->dataPtr->lastUpdate)
  {
    // Generate and publish current velocity according to the vehicle depth
    geometry_msgs::msg::TwistStamped flowVelMsg;
    flowVelMsg.header.stamp = this->dataPtr->rosNode->get_clock()->now();
    flowVelMsg.header.frame_id =
      "world";  // Changed from "/world" to be consistent with ROS 2 TF2 conventions

    flowVelMsg.twist.linear.x = this->dataPtr->currentVelocity.X();
    flowVelMsg.twist.linear.y = this->dataPtr->currentVelocity.Y();
    flowVelMsg.twist.linear.z = this->dataPtr->currentVelocity.Z();

    this->dataPtr->flowVelocityPub->publish(flowVelMsg);

    // Generate and publish stratified current velocity
    dave_interfaces::msg::StratifiedCurrentVelocity stratCurrentVelocityMsg;
    stratCurrentVelocityMsg.header.stamp = this->dataPtr->rosNode->get_clock()->now();
    stratCurrentVelocityMsg.header.frame_id = "world";

    // Updating for stratified behaviour of Ocean Currents
    // What is the .size value over here, to be (checked)
    for (size_t i = 0; i < this->dataPtr->currentStratifiedVelocity.size(); i++)
    {
      geometry_msgs::msg::Vector3 velocity;
      velocity.x = this->dataPtr->currentStratifiedVelocity[i].X();
      velocity.y = this->dataPtr->currentStratifiedVelocity[i].Y();
      velocity.z = this->dataPtr->currentStratifiedVelocity[i].Z();
      stratCurrentVelocityMsg.velocities.push_back(velocity);
      stratCurrentVelocityMsg.depths.push_back(this->dataPtr->currentStratifiedVelocity[i].W());
    }

    this->dataPtr->stratifiedCurrentVelocityPub->publish(stratCurrentVelocityMsg);

    // Generate and publish stratified current database
    dave_interfaces::msg::StratifiedCurrentDatabase currentDatabaseMsg;
    for (int i = 0; i < this->dataPtr->stratifiedDatabase.size(); i++)
    {
      // Stratified current database entry preparation
      geometry_msgs::msg::Vector3 velocity;
      velocity.x = this->dataPtr->stratifiedDatabase[i].X();
      velocity.y = this->dataPtr->stratifiedDatabase[i].Y();
      velocity.z = 0.0;  // Assuming z is intentionally set to 0.0
      currentDatabaseMsg.velocities.push_back(velocity);
      currentDatabaseMsg.depths.push_back(this->dataPtr->stratifiedDatabase[i].Z());
    }

    if (this->dataPtr->tidalHarmonicFlag)
    {
      // Tidal harmonic constituents
      currentDatabaseMsg.m2amp = this->dataPtr->M2_amp;
      currentDatabaseMsg.m2phase = this->dataPtr->M2_phase;
      currentDatabaseMsg.m2speed = this->dataPtr->M2_speed;
      currentDatabaseMsg.s2amp = this->dataPtr->S2_amp;
      currentDatabaseMsg.s2phase = this->dataPtr->S2_phase;
      currentDatabaseMsg.s2speed = this->dataPtr->S2_speed;
      currentDatabaseMsg.n2amp = this->dataPtr->N2_amp;
      currentDatabaseMsg.n2phase = this->dataPtr->N2_phase;
      currentDatabaseMsg.n2speed = this->dataPtr->N2_speed;
      currentDatabaseMsg.tideConstituents = true;
    }
    else
    {
      for (int i = 0; i < this->dataPtr->dateGMT.size(); i++)
      {
        // Tidal oscillation database
        currentDatabaseMsg.timegmtyear.push_back(this->dataPtr->dateGMT[i][0]);
        currentDatabaseMsg.timegmtmonth.push_back(this->dataPtr->dateGMT[i][1]);
        currentDatabaseMsg.timegmtday.push_back(this->dataPtr->dateGMT[i][2]);
        currentDatabaseMsg.timegmthour.push_back(this->dataPtr->dateGMT[i][3]);
        currentDatabaseMsg.timegmtminute.push_back(this->dataPtr->dateGMT[i][4]);

        currentDatabaseMsg.tidevelocities.push_back(this->dataPtr->speedcmsec[i]);
      }
      currentDatabaseMsg.tideConstituents = false;
    }

    currentDatabaseMsg.ebbdirection = this->dataPtr->ebbDirection;
    currentDatabaseMsg.flooddirection = this->dataPtr->floodDirection;

    currentDatabaseMsg.worldstarttimeyear = this->dataPtr->world_start_time_year;
    currentDatabaseMsg.worldstarttimemonth = this->dataPtr->world_start_time_month;
    currentDatabaseMsg.worldstarttimeday = this->dataPtr->world_start_time_day;
    currentDatabaseMsg.worldstarttimehour = this->dataPtr->world_start_time_hour;
    currentDatabaseMsg.worldstarttimeminute = this->dataPtr->world_start_time_minute;

    this->dataPtr->stratifiedCurrentDatabasePub->publish(currentDatabaseMsg);
  }
}
/////////////////////////////////////////////////
void UnderwaterCurrentROSPlugin::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  // Advertise the service to update the current velocity model
  this->dataPtr->set_current_velocity_model =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::SetCurrentModel>(
      "set_current_velocity_model", std::bind(
                                      &UnderwaterCurrentROSPlugin::UpdateCurrentVelocityModel, this,
                                      std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to update the current horizontal angle model
  this->dataPtr->set_current_horz_angle_model =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::SetCurrentModel>(
      "set_current_horz_angle_model", std::bind(
                                        &UnderwaterCurrentROSPlugin::UpdateCurrentHorzAngleModel,
                                        this, std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to update the current vertical angle model
  this->dataPtr->set_current_vert_angle_model =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::SetCurrentModel>(
      "set_current_vert_angle_model", std::bind(
                                        &UnderwaterCurrentROSPlugin::UpdateCurrentVertAngleModel,
                                        this, std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to update the current velocity mean value
  this->dataPtr->set_current_velocity =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::SetCurrentVelocity>(
      "set_current_velocity", std::bind(
                                &UnderwaterCurrentROSPlugin::UpdateCurrentVelocity, this,
                                std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to update the stratified current velocity
  this->dataPtr->set_stratified_current_velocity =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::SetStratifiedCurrentVelocity>(
      "set_stratified_current_velocity", std::bind(
                                           &UnderwaterCurrentROSPlugin::UpdateStratCurrentVelocity,
                                           this, std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to update the current horizontal angle
  this->dataPtr->set_current_horz_angle =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::SetCurrentAngle>(
      "set_current_horz_angle", std::bind(
                                  &UnderwaterCurrentROSPlugin::UpdateHorzAngle, this,
                                  std::placeholders::_1, std::placeholders::_2));
  // Advertise the service to update the stratified current horizontal angle
  this->dataPtr->set_stratified_current_horz_angle_model =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::SetStratifiedCurrentDirection>(
      "set_stratified_current_horz_angle_model",
      std::bind(
        &UnderwaterCurrentROSPlugin::UpdateStratHorzAngleModel, this, std::placeholders::_1,
        std::placeholders::_2));

  // Advertise the service to update the current vertical angle
  this->dataPtr->set_current_vert_angle_model =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::SetCurrentAngle>(
      "set_current_vert_angle_model", std::bind(
                                        &UnderwaterCurrentROSPlugin::UpdateVertAngleModel, this,
                                        std::placeholders::_1, std::placeholders::_2));

  // Advertise the service to update the stratified current vertical angle
  this->dataPtr->set_stratified_current_vert_angle_model =
    this->dataPtr->rosNode->create_service<dave_interfaces::srv::SetStratifiedCurrentDirection>(
      "set_stratified_current_vert_angle_model",
      std::bind(
        &UnderwaterCurrentROSPlugin::UpdateStratVertAngleModel, this, std::placeholders::_1,
        std::placeholders::_2));

  // Update the time tracking for publication
  this->dataPtr->lastUpdate = _info.simTime;
}
/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateHorzAngle(
  const std::shared_ptr<dave_interfaces::srv::SetCurrentDirection::Request> _req,
  std::shared_ptr<dave_interfaces::srv::SetCurrentDirection::Response> _res)
{
  _res->success = this->dataPtr->currentHorzAngleModel.SetMean(_req->angle);

  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateStratHorzAngleModel(
  const std::shared_ptr<dave_interfaces::srv::SetStratifiedCurrentDirection::Request> _req,
  std::shared_ptr<dave_interfaces::srv::SetStratifiedCurrentDirection::Response> _res)
{
  if (_req->layer >= this->dataPtr->stratifiedDatabase.size())
  {
    _res->success = false;
    return true;
  }

  _res->success = this->dataPtr->stratifiedCurrentModels[_req->layer][1].SetMean(_req->angle);
  if (_res->success)
  {
    // Update the database values (new angle, unchanged velocity)
    double velocity = hypot(
      this->dataPtr->stratifiedDatabase[_req->layer].X(),
      this->dataPtr->stratifiedDatabase[_req->layer].Y());
    this->dataPtr->stratifiedDatabase[_req->layer].X() = cos(_req->angle) * velocity;
    this->dataPtr->stratifiedDatabase[_req->layer].Y() = sin(_req->angle) * velocity;
  }
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateVertAngle(
  const std::shared_ptr<dave_interfaces::srv::SetCurrentDirection::Request> _req,
  std::shared_ptr<dave_interfaces::srv::SetCurrentDirection::Response> _res)
{
  _res->success = this->dataPtr->currentVertAngleModel.SetMean(_req->angle);
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateStratVertAngleModel(
  const std::shared_ptr<dave_interfaces::srv::SetStratifiedCurrentDirection::Request> _req,
  std::shared_ptr<dave_interfaces::srv::SetStratifiedCurrentDirection::Response> _res)
{
  if (_req->layer >= this->dataPtr->stratifiedDatabase.size())
  {
    _res->success = false;
    return true;
  }
  _res->success = this->dataPtr->stratifiedCurrentModels[_req->layer][2].SetMean(_req->angle);
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateCurrentVelocity(
  const std::shared_ptr<dave_interfaces::srv::SetCurrentVelocity::Request> _req,
  std::shared_ptr<dave_interfaces::srv::SetCurrentVelocity::Response> _res)
{
  if (
    this->dataPtr->currentVelModel.SetMean(_req->velocity) &&
    this->dataPtr->currentHorzAngleModel.SetMean(_req->horizontal_angle) &&
    this->dataPtr->currentVertAngleModel.SetMean(_req->vertical_angle))
  {
    gzmsg << "Current velocity [m/s] = " << _req.velocity << std::endl
          << "Current horizontal angle [rad] = " << _req.horizontal_angle << std::endl
          << "Current vertical angle [rad] = " << _req.vertical_angle << std::endl
          << "\tWARNING: Current velocity calculated in the ENU frame" << std::endl;
    _res.success = true;
  }
  else
  {
    gzmsg << "Error while updating the current velocity" << std::endl;
    _res.success = false;
  }
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateStratCurrentVelocity(
  const std::shared_ptr<dave_interfaces::srv::SetStratifiedCurrentVelocity::Request> _req,
  std::shared_ptr<dave_interfaces::srv::SetStratifiedCurrentVelocity::Response> _res)
{
  if (_req->layer >= this->dataPtr->stratifiedDatabase.size())
  {
    _res->success = false;
    return true;
  }
  if (
    this->dataPtr->stratifiedCurrentModels[_req->layer][0].SetMean(_req->velocity) &&
    this->dataPtr->stratifiedCurrentModels[_req->layer][1].SetMean(_req->horizontal_angle) &&
    this->dataPtr->stratifiedCurrentModels[_req->layer][2].SetMean(_req->vertical_angle))
  {
    // Update the database values as well
    this->dataPtr->stratifiedDatabase[_req->layer].X() =
      cos(_req->horizontal_angle) * _req->velocity;
    this->dataPtr->stratifiedDatabase[_req->layer].Y() =
      sin(_req->horizontal_angle) * _req->velocity;
    gzmsg << "Layer " << _req.layer << " current velocity [m/s] = " << _req.velocity << std::endl
          << "  Horizontal angle [rad] = " << _req.horizontal_angle << std::endl
          << "  Vertical angle [rad] = " << _req.vertical_angle << std::endl
          << "\tWARNING: Current velocity calculated in the ENU frame" << std::endl;
    _res.success = true;
  }
  else
  {
    gzmsg << "Error while updating the current velocity" << std::endl;
    _res.success = false;
  }
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::GetCurrentVelocityModel(
  const std::shared_ptr<dave_interfaces::srv::GetCurrentModel::Request> _req,
  std::shared_ptr<dave_interfaces::srv::GetCurrentModel::Response> _res)
{
  _res->mean = this->dataPtr->currentVelModel.mean;
  _res->min = this->dataPtr->currentVelModel.min;
  _res->max = this->dataPtr->currentVelModel.max;
  _res->noise = this->dataPtr->currentVelModel.noiseAmp;
  _res->mu = this->dataPtr->currentVelModel.mu;
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::GetCurrentHorzAngleModel(
  const std::shared_ptr<dave_interfaces::srv::GetCurrentModel::Request> _req,
  std::shared_ptr<dave_interfaces::srv::GetCurrentModel::Response> _res)
{
  _res->mean = this->dataPtr->currentHorzAngleModel.mean;
  _res->min = this->dataPtr->currentHorzAngleModel.min;
  _res->max = this->dataPtr->currentHorzAngleModel.max;
  _res->noise = this->dataPtr->currentHorzAngleModel.noiseAmp;
  _res->mu = this->dataPtr->currentHorzAngleModel.mu;
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::GetCurrentVertAngleModel(
  const std::shared_ptr<dave_interfaces::srv::GetCurrentModel::Request> _req,
  std::shared_ptr<dave_interfaces::srv::GetCurrentModel::Response> _res)
{
  _res->mean = this->dataPtr->currentVertAngleModel.mean;
  _res->min = this->dataPtr->currentVertAngleModel.min;
  _res->max = this->dataPtr->currentVertAngleModel.max;
  _res->noise = this->dataPtr->currentVertAngleModel.noiseAmp;
  _res->mu = this->dataPtr->currentVertAngleModel.mu;
  return true;
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateCurrentVelocityModel(
  const std::shared_ptr<dave_interfaces::srv::SetCurrentModel::Request> _req,
  std::shared_ptr<dave_interfaces::srv::SetCurrentModel::Response> _res)
{
  _res->success = this->dataPtr->currentVelModel.SetModel(
    std::max(0.0, _req->mean), std::min(0.0, _req->min), std::max(0.0, _req->max), _req->mu,
    _req->noise);

  for (int i = 0; i < this->dataPtr->stratifiedCurrentModels.size(); i++)
  {
    gz::GaussMarkovProcess & model = this->dataPtr->stratifiedCurrentModels[i][0];  //(updated)
    model.SetModel(
      model.mean, std::max(0.0, _req->min), std::max(0.0, _req->max), _req->mu, _req->noise);
  }

  gzmsg << "Current velocity model updated" << std::endl
        << "\tWARNING: Current velocity calculated in the ENU frame" << std::endl;
  this->dataPtr->currentVelModel.Print();

  return true;
}
/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateCurrentHorzAngleModel(
  const std::shared_ptr<dave_interfaces::srv::SetCurrentModel::Request> _req,
  std::shared_ptr<dave_interfaces::srv::SetCurrentModel::Response> _res)
{
  _res->success = this->dataPtr->currentHorzAngleModel.SetModel(
    _req->mean, _req->min, _req->max, _req->mu, _req->noise);

  for (int i = 0; i < this->dataPtr->stratifiedCurrentModels.size(); i++)
  {
    gz::GaussMarkovProcess & model = this->dataPtr->stratifiedCurrentModels[i][1];
    model.SetModel(
      model.mean, std::max(-M_PI, _req->min), std::min(M_PI, _req->max), _req->mu, _req->noise);
  }

  gzmsg << "Horizontal angle model updated" << std::endl
        << "\tWARNING: Current velocity calculated in the ENU frame" << std::endl;
  this->dataPtr->currentHorzAngleModel.Print();
  return true;

  // gzmsg << "Horizontal angle model updated" << std::endl
  //       << "\tWARNING: Current velocity calculated in the ENU frame" << std::endl;
  // this->dataPtr->currentHorzAngleModel.Print();
}

/////////////////////////////////////////////////
bool UnderwaterCurrentROSPlugin::UpdateCurrentVertAngleModel(
  const std::shared_ptr<dave_interfaces::srv::SetCurrentModel::Request> _req,
  std::shared_ptr<dave_interfaces::srv::SetCurrentModel::Response> _res)
{
  _res->success = this->dataPtr->currentVertAngleModel.SetModel(
    _req->mean, _req->min, _req->max, _req->mu, _req->noise);

  gzmsg << "Vertical angle model updated" << std::endl
        << "\tWARNING: Current velocity calculated in the ENU frame" << std::endl;
  this->dataPtr->currentVertAngleModel.Print();
  return true;
}

/////////////////////////////////////////////////
GZ_REGISTER_WORLD_PLUGIN(UnderwaterCurrentROSPlugin)
}  // namespace dave_ros_gz_plugins

// #endif