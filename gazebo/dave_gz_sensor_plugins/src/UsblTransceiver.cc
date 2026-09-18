/*
 * Copyright (C) 2022 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gz/msgs/vector3d.pb.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <gz/common/Console.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/Util.hh>
#include <gz/sim/World.hh>
#include <gz/sim/components/Link.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/ParentEntity.hh>
#include <gz/transport/Node.hh>
#include <rclcpp/rclcpp.hpp>
#include "gz/common/StringUtils.hh"
#include "gz/plugin/Register.hh"
#include "gz/sim/components/Model.hh"
#include "gz/sim/components/World.hh"

#include <geometry_msgs/msg/vector3.hpp>

#include "dave_gz_sensor_plugins/UsblTransceiver.hh"

// available interrogation modes
std::vector<std::string> im = {"common", "individual"};
using std::placeholders::_1;

GZ_ADD_PLUGIN(
  dave_gz_sensor_plugins::UsblTransceiver, gz::sim::System,
  dave_gz_sensor_plugins::UsblTransceiver::ISystemConfigure,
  dave_gz_sensor_plugins::UsblTransceiver::ISystemPostUpdate)

namespace dave_gz_sensor_plugins
{

struct UsblTransceiver::PrivateData
{
  // Add any private data members here.
  gz::sim::Model model;
  std::string modelName;
  std::string ns;
  std::string m_transceiverDevice;
  gz::sim::EntityComponentManager * ecm;
  std::string m_transceiverID;
  std::vector<gz::sim::Entity> m_transponderEntities;
  std::string m_transponderDevice;
  std::vector<std::string> m_deployedTransponders;
  std::vector<std::string> m_transponderAttachedObjects;
  bool m_enablePingerScheduler;
  std::string m_transponderAttachedObject;
  std::string m_interrogationMode;
  gz::sim::Entity linkEntity;
  gz::transport::Node m_gzNode;
  rclcpp::Publisher<dave_interfaces::msg::Location>::SharedPtr m_publishTransponderRelPos;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr m_cisPinger;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr m_interrogationModePub;
  std::unordered_map<std::string, rclcpp::Publisher<std_msgs::msg::String>::SharedPtr> m_iisPinger;
  std::unordered_map<std::string, rclcpp::Publisher<dave_interfaces::msg::UsblCommand>::SharedPtr>
    m_commandPubs;
  rclcpp::Publisher<dave_interfaces::msg::Location>::SharedPtr m_publishTransponderRelPosCartesian;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr m_temperatureSub;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr m_interrogationModeSub;
  rclcpp::Subscription<dave_interfaces::msg::UsblResponse>::SharedPtr m_commandResponseSub;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr m_channelSwitchSub;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr m_commandResponseTestSub;
  std::unordered_map<std::string, std::function<void(const gz::msgs::Vector3d &)>>
    transponderCallbacks;
  std::string m_channel;
  double m_soundSpeed;
  double m_temperature;
  double m_pingFrequency;
  bool setGlobalMode;
};

// // Define constants for command IDs
const int BATTERY_LEVEL = 1;
const int GO_TO = 2;

UsblTransceiver::UsblTransceiver() : dataPtr(std::make_unique<PrivateData>())
{
  dataPtr->m_channel = "1";
  dataPtr->m_temperature = 10.0;
  dataPtr->setGlobalMode = false;
}

void UsblTransceiver::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventManager)
{
  gzdbg << "dave_gz_sensor_plugins::UsblTransceiver::Configure on entity: " << _entity << std::endl;

  if (!rclcpp::ok())
  {
    rclcpp::init(0, nullptr);
  }

  this->ros_node_ = std::make_shared<rclcpp::Node>("usbl_transceiver_node");

  auto model = gz::sim::Model(_entity);
  this->dataPtr->model = model;
  this->dataPtr->modelName = model.Name(_ecm);
  this->dataPtr->ecm = &_ecm;

  auto links = this->dataPtr->model.Links(_ecm);

  if (!links.empty())
  {
    auto linkEntity = links.front();
    auto linkName = _ecm.Component<gz::sim::components::Name>(linkEntity);
    std::string linkNameStr = linkName->Data();
    gzmsg << "Principal link name: " << linkNameStr << std::endl;
    this->dataPtr->linkEntity = model.LinkByName(_ecm, linkNameStr);
    std::cout << worldPose(this->dataPtr->linkEntity, _ecm) << std::endl;
  }
  else
  {
    gzmsg << "No links found in the model." << std::endl;
  }

  if (!model.Valid(_ecm))
  {
    gzerr << "UsblTransceiver plugin should be attached to a model entity. "
          << "Failed to initialize." << std::endl;
    return;
  }
  // Grab namespace from SDF
  if (!_sdf->HasElement("namespace"))
  {
    gzerr << "Missing required parameter <namespace>, "
          << "plugin will not be initialized." << std::endl;
    return;
  }
  this->dataPtr->ns = _sdf->Get<std::string>("namespace");

  // Grab sound speed from SDF
  if (!_sdf->HasElement("sound_speed"))
  {
    this->dataPtr->m_soundSpeed = 1540.0;
    gzmsg << "Sound speed default to  " << this->dataPtr->m_soundSpeed << std::endl;
  }
  else
  {
    this->dataPtr->m_soundSpeed = _sdf->Get<double>("sound_speed");
    gzmsg << "Sound speed: " << this->dataPtr->m_soundSpeed << std::endl;
  }

  // Obtain transceiver device name from SDF
  if (!_sdf->HasElement("transceiver_device"))
  {
    gzerr << "Missing required parameter <transceiver_device>, "
          << "plugin will not be initialized." << std::endl;
    return;
  }

  this->dataPtr->m_transceiverDevice = _sdf->Get<std::string>("transceiver_device");
  gzmsg << "Entity: " << this->dataPtr->m_transceiverDevice << std::endl;

  // Get transceiver device id
  if (!_sdf->HasElement("transceiver_ID"))
  {
    gzerr << "Missing required parameter <transceiver_ID>, "
          << "plugin will not be initialized." << std::endl;
    return;
  }

  this->dataPtr->m_transceiverID = _sdf->Get<std::string>("transceiver_ID");

  // get transponder device name
  if (!_sdf->HasElement("transponder_device"))
  {
    gzerr << "Missing required parameter <transponder_device>, "
          << "plugin will not be initialized." << std::endl;
    return;
  }
  this->dataPtr->m_transponderDevice = _sdf->Get<std::string>("transponder_device");
  gzmsg << "Transponder device: " << this->dataPtr->m_transponderDevice << std::endl;

  // get commanding transponders
  if (!_sdf->HasElement("transponder_ID"))
  {
    gzerr << "Missing required parameter <transponder_ID>, "
          << "plugin will not be initialized." << std::endl;
    return;
  }

  auto transponders = gz::common::Split(_sdf->Get<std::string>("transponder_ID"), ',');
  gzmsg << "Current deployed transponders are: \n";

  for (auto & transponder : transponders)
  {
    gzmsg << transponder << std::endl;
    this->dataPtr->m_deployedTransponders.push_back(transponder);
  }

  // Enable automation of sending pings to transponder
  if (!_sdf->HasElement("enable_ping_scheduler"))
  {
    gzerr << "Missing required parameter <enable_ping_scheduler>, "
          << "plugin will not be initialized." << std::endl;
    return;
  }

  this->dataPtr->m_enablePingerScheduler = _sdf->Get<bool>("enable_ping_scheduler");

  if (this->dataPtr->m_enablePingerScheduler)
  {
    if (!_sdf->HasElement("ping_frequency"))
    {
      gzmsg << "Ping frequency not specified, default to 1 Hz" << std::endl;
      this->dataPtr->m_pingFrequency = 1;
    }
    else
    {
      this->dataPtr->m_pingFrequency = _sdf->Get<double>("ping_frequency");
    }
  }
  gzmsg << "Pinger enable? " << this->dataPtr->m_enablePingerScheduler << std::endl;

  // Get object that transponder attached to
  if (!_sdf->HasElement("transponder_attached_object"))
  {
    gzerr << "Missing required parameter <transponder_attached_object>, "
          << "plugin will not be initialized." << std::endl;
    return;
  }

  auto transpondersObjects =
    gz::common::Split(_sdf->Get<std::string>("transponder_attached_object"), ',');
  gzmsg << "Current deployed transponders objects are: \n";

  for (auto & object : transpondersObjects)
  {
    this->dataPtr->m_transponderAttachedObjects.push_back(object);
    gzmsg << "Added " << object << " to list of transponders objects" << std::endl;
  }

  /*  interrogation mode - 2 options
   *  II (individual interrogation) <----->  CRS (common response signal)
   *  CI (common interrogation)     <----->  IRS (individual response
   *                                         signal) from transponder_01
   *                                    ͱ->  IRS from transponder_02
   *                                    ͱ->  IRS from transponder_03
   *                                            ⋮
   */

  if (_sdf->HasElement("interrogation_mode"))
  {
    std::string interrogation_mode = _sdf->Get<std::string>("interrogation_mode");
    gzmsg << interrogation_mode << " interrogation mode is requested" << std::endl;
    if (std::find(im.begin(), im.end(), interrogation_mode) != im.end())
    {
      gzmsg << interrogation_mode << " interrogation mode is used" << std::endl;
      this->dataPtr->m_interrogationMode = interrogation_mode;
    }
    else
    {
      gzmsg << "Specified interrogation mode is unavailable, "
            << "Common mode is used" << std::endl;
      this->dataPtr->m_interrogationMode = "common";
    }
  }
  else
  {
    gzmsg << "Interrogation mode is not specified, Common mode is used" << std::endl;
    this->dataPtr->m_interrogationMode = "common";
  }

  // Gazebo subscriber

  for (auto & transponder : this->dataPtr->m_deployedTransponders)
  {
    std::string transponder_position = "/" + this->dataPtr->ns + "/" +
                                       this->dataPtr->m_transceiverDevice + "_" + transponder +
                                       "/global_position";

    gzmsg << "Transponder topic" << transponder_position << std::endl;

    // Created lambda function to get the transponder ID in the created callback

    std::function<void(const gz::msgs::Vector3d &)> callback =
      [this, transponder](const gz::msgs::Vector3d & msg)
    { this->receiveGazeboCallback(transponder, msg); };

    this->dataPtr->transponderCallbacks[transponder] = callback;
    this->dataPtr->m_gzNode.Subscribe(transponder_position, callback);
  }

  // ROS2 Publishers

  std::string transponder_location_topic = "/" + this->dataPtr->ns + "/" +
                                           this->dataPtr->m_transceiverDevice + "_" +
                                           this->dataPtr->m_transceiverID + "/transponder_location";

  std::string interrogation_mode_topic = "/" + this->dataPtr->ns + "/" +
                                         this->dataPtr->m_transceiverDevice + "_" +
                                         this->dataPtr->m_transceiverID + "/interrogation_mode";

  std::string cis_pinger_topic = "/" + this->dataPtr->ns + "/common_interrogation_ping";

  this->dataPtr->m_publishTransponderRelPos =
    this->ros_node_->create_publisher<dave_interfaces::msg::Location>(
      transponder_location_topic, 1);

  this->dataPtr->m_cisPinger =
    this->ros_node_->create_publisher<std_msgs::msg::String>(cis_pinger_topic, 1);

  this->dataPtr->m_interrogationModePub =
    this->ros_node_->create_publisher<std_msgs::msg::String>(interrogation_mode_topic, 1);

  for (auto & transponder : this->dataPtr->m_deployedTransponders)
  {
    std::string ping_topic(
      "/" + this->dataPtr->ns + "/" + this->dataPtr->m_transponderDevice + "_" + transponder +
      "/individual_interrogation_ping");
    std::string command_topic(
      "/" + this->dataPtr->ns + "/" + this->dataPtr->m_transponderDevice + "_" + transponder +
      "/command_request");
    this->dataPtr->m_iisPinger[transponder] =
      this->ros_node_->create_publisher<std_msgs::msg::String>(ping_topic, 1);
    this->dataPtr->m_commandPubs[transponder] =
      this->ros_node_->create_publisher<dave_interfaces::msg::UsblCommand>(command_topic, 1);
  }
  std::string transponder_location_cartesian_topic =
    "/" + this->dataPtr->ns + "/" + this->dataPtr->m_transceiverDevice + "_" +
    this->dataPtr->m_transceiverID + "/transponder_location_cartesian";

  this->dataPtr->m_publishTransponderRelPosCartesian =
    this->ros_node_->create_publisher<dave_interfaces::msg::Location>(
      transponder_location_cartesian_topic, 1);

  // ROS2 Subscribers

  this->dataPtr->m_temperatureSub = this->ros_node_->create_subscription<std_msgs::msg::Float64>(
    "/" + this->dataPtr->ns + "/" + this->dataPtr->m_transceiverDevice + "_" +
      this->dataPtr->m_transceiverID + "/temperature",
    1, std::bind(&UsblTransceiver::temperatureRosCallback, this, _1));

  this->dataPtr->m_interrogationModeSub =
    this->ros_node_->create_subscription<std_msgs::msg::String>(
      interrogation_mode_topic, 1,
      std::bind(&UsblTransceiver::interrogationModeRosCallback, this, _1));

  this->dataPtr->m_commandResponseSub =
    this->ros_node_->create_subscription<dave_interfaces::msg::UsblResponse>(
      "/" + this->dataPtr->ns + "/" + this->dataPtr->m_transceiverDevice + "_" +
        this->dataPtr->m_transceiverID + "/command_response",
      1, std::bind(&UsblTransceiver::commandingResponseCallback, this, _1));

  this->dataPtr->m_channelSwitchSub = this->ros_node_->create_subscription<std_msgs::msg::String>(
    "/" + this->dataPtr->ns + "/" + this->dataPtr->m_transceiverDevice + "_" +
      this->dataPtr->m_transceiverID + "/channel_switch",
    1, std::bind(&UsblTransceiver::channelSwitchCallback, this, _1));

  this->dataPtr->m_commandResponseTestSub =
    this->ros_node_->create_subscription<std_msgs::msg::String>(
      "/" + this->dataPtr->ns + "/" + this->dataPtr->m_transceiverDevice + "_" +
        this->dataPtr->m_transceiverID + "/command_response_test",
      1, std::bind(&UsblTransceiver::commandingResponseTestCallback, this, _1));
}

void UsblTransceiver::commandingResponseTestCallback(const std_msgs::msg::String::SharedPtr msg)
{
  std::string transponder_id = msg->data;
  sendCommand(BATTERY_LEVEL, transponder_id);
}

void UsblTransceiver::sendCommand(int command_id, std::string & transponder_id)
{
  dave_interfaces::msg::UsblCommand command;
  command.command_id = command_id;
  command.transponder_id = std::stoi(transponder_id);

  if (command_id == BATTERY_LEVEL)
  {
    command.data = "Report battery level";
  }
  else if (command_id == GO_TO)
  {
    command.data = "Go to this location";
  }
  else
  {
    command.data = "This is dummy message";
  }

  this->dataPtr->m_commandPubs[transponder_id]->publish(command);
}

void UsblTransceiver::sendPing()
{
  std_msgs::msg::String ping_msg;
  ping_msg.data = "ping";

  for (auto & object : this->dataPtr->m_transponderAttachedObjects)
  {
    auto transponder = this->dataPtr->ecm->EntityByComponents(
      gz::sim::components::Name(object), gz::sim::components::Model());

    gz::math::Pose3d pose_transponder = worldPose(transponder, *this->dataPtr->ecm);
    gzmsg << "Transponder pose" << pose_transponder << std::endl;

    gz::math::Pose3d pose_transceiver = worldPose(this->dataPtr->linkEntity, *this->dataPtr->ecm);
    gzmsg << "Transceiver pose" << pose_transceiver << std::endl;

    double dist = (pose_transponder.Pos() - pose_transceiver.Pos()).Length();
    this->dataPtr->m_soundSpeed = 1540.4 + pose_transceiver.Z() / 1000 * 17;

    sleep(dist / this->dataPtr->m_soundSpeed);

    if (this->dataPtr->m_interrogationMode.compare("common") == 0)
    {
      this->dataPtr->m_cisPinger->publish(ping_msg);
    }
    else if (this->dataPtr->m_interrogationMode.compare("individual") == 0)
    {
      this->dataPtr->m_iisPinger[this->dataPtr->m_channel]->publish(ping_msg);
    }
    else
    {
      gzmsg << "Interrogation mode not recognized\n";
    }
  }
}

void UsblTransceiver::channelSwitchCallback(const std_msgs::msg::String::SharedPtr msg)
{
  gzmsg << "Switching to transponder_" << msg->data << " channel\n";
  this->dataPtr->m_channel = msg->data;

  this->dataPtr->m_interrogationMode = "individual";

  std_msgs::msg::String mode;
  mode.data = this->dataPtr->m_interrogationMode;

  this->dataPtr->m_interrogationModePub->publish(mode);

  gzmsg << "Interrogation mode set to individual, because channel switch was requested"
        << std::endl;
}

void UsblTransceiver::commandingResponseCallback(const dave_interfaces::msg::UsblResponse msg)
{
  gzmsg << "Response_id: " << msg.response_id << ", transponder id: " << msg.transceiver_id
        << ", data: " << msg.data << std::endl;
}

void UsblTransceiver::interrogationModeRosCallback(const std_msgs::msg::String::SharedPtr msg)
{
  std::string mode = msg->data;
  if (std::find(im.begin(), im.end(), mode) != im.end())
  {
    this->dataPtr->m_interrogationMode = mode;
    gzmsg << "Interrogation mode set to " << mode << std::endl;
  }
  else
  {
    gzmsg << "The input mode is not available\n";
  }
}

void UsblTransceiver::temperatureRosCallback(const std_msgs::msg::Float64::SharedPtr msg)
{
  gz::math::Pose3d pose = worldPose(this->dataPtr->linkEntity, *this->dataPtr->ecm);
  gz::math::Vector3<double> position = pose.Pos();

  this->dataPtr->m_temperature = msg->data;

  // Base on https://dosits.org/tutorials/science/tutorial-speed/
  this->dataPtr->m_soundSpeed =
    1540.4 + position.Z() / 1000 * 17 + (this->dataPtr->m_temperature - 10) * 4;
  gzmsg << "Detected change of temperature, transceiver sound speed is now: "
        << this->dataPtr->m_soundSpeed << " m/s\n";
}

void UsblTransceiver::receiveGazeboCallback(
  const std::string & transponder, const gz::msgs::Vector3d & transponder_position)
{
  bool publish;

  if (this->dataPtr->m_interrogationMode.compare("common") == 0)
  {
    gzmsg << "In common mode, publishing position..." << std::endl;

    publish = true;
  }
  else if (
    this->dataPtr->m_interrogationMode.compare("individual") == 0 &&
    transponder.compare(this->dataPtr->m_channel) == 0)
  {
    publish = true;

    gzmsg << "In individual mode, publishing position for transponder_" << transponder << std::endl;
  }
  else
  {
    publish = false;

    gzmsg << "Interrogation mode is not set to common and wrong channel is being pinged"
          << std::endl;
  }

  if (publish)
  {
    gzmsg << "Transceiver acquires transponder_" << transponder
          << " position: " << transponder_position.x() << " " << transponder_position.y() << " "
          << transponder_position.z() << std::endl;

    int transponder_id;

    try
    {
      transponder_id = std::stoi(transponder);
    }
    catch (const std::invalid_argument & e)
    {
      gzmsg << "Invalid transponder ID: " << transponder_id << std::endl;

      return;
    }
    catch (const std::out_of_range & e)
    {
      gzmsg << "Transponder ID out of range: " << transponder_id << std::endl;

      return;
    }

    gz::math::Vector3<double> transponder_position_ign = gz::math::Vector3<double>(
      transponder_position.x(), transponder_position.y(), transponder_position.z());

    double bearing = 0, range = 0, elevation = 0;
    calculateRelativePose(transponder_position_ign, bearing, range, elevation);

    publishPosition(transponder_id, bearing, range, elevation);
  }
}

void UsblTransceiver::publishPosition(
  int & transponder_id, double & bearing, double & range, double & elevation)
{
  dave_interfaces::msg::Location location;

  location.x = bearing;
  location.y = range;
  location.z = elevation;

  dave_interfaces::msg::Location location_cartesian;
  location_cartesian.x = range * cos(elevation * M_PI / 180) * cos(bearing * M_PI / 180);
  location_cartesian.y = range * cos(elevation * M_PI / 180) * sin(bearing * M_PI / 180);
  location_cartesian.z = range * sin(elevation * M_PI / 180);

  location_cartesian.transponder_id = transponder_id;
  location.transponder_id = transponder_id;

  // gzmsg << "Spherical Coordinate: \n\tBearing: " << location.x
  //       << " degree(s)\n\tRange: " << location.y << " m\n\tElevation: " << location.z
  //       << " degree(s)\n";
  // gzmsg << "Cartesian Coordinate: \n\tX: " << location_cartesian.x
  //       << " m\n\tY: " << location_cartesian.y << " m\n\tZ: " << location_cartesian.z << "
  //       m\n\n";

  this->dataPtr->m_publishTransponderRelPos->publish(location);
  this->dataPtr->m_publishTransponderRelPosCartesian->publish(location_cartesian);
}

void UsblTransceiver::calculateRelativePose(
  gz::math::Vector3<double> position, double & bearing, double & range, double & elevation)
{
  gz::math::Pose3d pose = worldPose(this->dataPtr->linkEntity, *this->dataPtr->ecm);
  gz::math::Vector3<double> my_pos = pose.Pos();
  auto direction = position - my_pos;

  bearing = atan2(direction.Y(), direction.X()) * 180 / M_PI;
  range = sqrt(
    direction.X() * direction.X() + direction.Y() * direction.Y() + direction.Z() * direction.Z());
  elevation = asin(direction.Z() / direction.Length()) * 180 / M_PI;

  // gzmsg << "bearing: " << bearing << "\n";
  // gzmsg << "range: " << range << "\n";
  // gzmsg << "elevation: " << elevation << "\n\n";
}

void UsblTransceiver::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  // ROS callbacks are wall-time events and must remain responsive while paused.
  rclcpp::spin_some(this->ros_node_);
  if (!_info.paused)
  {
    if (!this->dataPtr->setGlobalMode)
    {
      std_msgs::msg::String mode;
      mode.data = this->dataPtr->m_interrogationMode;

      this->dataPtr->m_interrogationModePub->publish(mode);
      gzmsg << "Published mode" << std::endl;
      this->dataPtr->setGlobalMode = true;
    }

    if (this->dataPtr->m_enablePingerScheduler)
    {
      // Calculate the time interval between pings based on frequency
      double pingInterval = 1.0 / this->dataPtr->m_pingFrequency;

      // Check if it's time to send a ping
      if (_info.iterations % static_cast<int>(pingInterval * 1000) == 0)
      {
        sendPing();
      }
    }
  }
}

}  // namespace dave_gz_sensor_plugins
