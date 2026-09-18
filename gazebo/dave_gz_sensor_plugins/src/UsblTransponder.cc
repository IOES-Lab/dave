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

#include <algorithm>
#include <random>
#include <string>
#include <vector>

#include <gz/common/Console.hh>
#include <gz/common/StringUtils.hh>
#include <gz/math/Pose3.hh>
#include <gz/plugin/Register.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/Util.hh>
#include <gz/sim/components/Link.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/ParentEntity.hh>
#include <gz/transport/Node.hh>
#include "gz/sim/components/Model.hh"
#include "gz/sim/components/World.hh"

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include "dave_gz_sensor_plugins/UsblTransponder.hh"

using std::placeholders::_1;

GZ_ADD_PLUGIN(
  dave_gz_sensor_plugins::UsblTransponder, gz::sim::System,
  dave_gz_sensor_plugins::UsblTransponder::ISystemConfigure,
  dave_gz_sensor_plugins::UsblTransponder::ISystemPostUpdate)

namespace dave_gz_sensor_plugins
{

struct UsblTransponder::PrivateData
{
  // Add any private data members here.
  gz::sim::Model model;
  gz::sim::Entity transceiverEntity;
  gz::sim::EntityComponentManager * ecm;
  std::string modelName;
  std::string m_interrogationMode;
  std::string m_transceiverModelName;
  std::string ns;  // 'namespace' is a reserved word in C++, using 'ns' instead.
  std::string m_transceiverDevice;
  std::string m_transceiverID;
  std::string m_transponderDevice;
  std::string m_transponderID;
  double m_noiseSigma;
  double m_noiseMu;
  gz::transport::Node m_gzNode;
  gz::transport::Node::Publisher m_globalPosPub;
  gz::sim::Entity linkEntity;
  rclcpp::Publisher<dave_interfaces::msg::UsblResponse>::SharedPtr m_commandResponsePub;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr m_iisSub;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr m_cisSub;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr m_interrogationModeSub;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr m_temperatureSub;
  rclcpp::Subscription<dave_interfaces::msg::UsblCommand>::SharedPtr m_commandSub;
  double m_soundSpeed;
  double m_temperature;
};

UsblTransponder::UsblTransponder() : dataPtr(std::make_unique<PrivateData>())
{
  dataPtr->m_temperature = 10.0;  // Default initialization
  dataPtr->m_noiseSigma = 1.0;    // Default initialization
  dataPtr->m_noiseMu = 0.0;       // Default initialization
}

void UsblTransponder::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventManager)
{
  gzdbg << "dave_gz_sensor_plugins::UsblTransponder::Configure on entity: " << _entity << std::endl;

  if (!rclcpp::ok())
  {
    rclcpp::init(0, nullptr);
  }

  this->dataPtr->ecm = &_ecm;

  auto model = gz::sim::Model(_entity);
  this->dataPtr->model = model;
  this->dataPtr->modelName = model.Name(_ecm);

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
    gzerr << "UsblTransponder plugin should be attached to a model entity. "
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

  // Get transponder device name
  if (!_sdf->HasElement("transponder_device"))
  {
    gzerr << "Missing required parameter <transponder_device>, "
          << "plugin will not be initialized." << std::endl;
    return;
  }
  this->dataPtr->m_transponderDevice = _sdf->Get<std::string>("transponder_device");
  gzmsg << "Transponder device: " << this->dataPtr->m_transponderDevice << std::endl;

  // Get commanding transponders
  if (!_sdf->HasElement("transponder_ID"))
  {
    gzerr << "Missing required parameter <transponder_ID>, "
          << "plugin will not be initialized." << std::endl;
    return;
  }
  this->dataPtr->m_transponderID = _sdf->Get<std::string>("transponder_ID");
  this->ros_node_ =
    std::make_shared<rclcpp::Node>("usbl_transponder_" + this->dataPtr->m_transponderID + "_node");

  // Get the mean of normal distribution for the noise model
  if (_sdf->HasElement("mu"))
  {
    this->dataPtr->m_noiseMu = _sdf->Get<double>("mu");
  }

  // Get the standard deviation of normal distribution for the noise model
  if (_sdf->HasElement("sigma"))
  {
    this->dataPtr->m_noiseSigma = _sdf->Get<double>("sigma");
    if (this->dataPtr->m_noiseSigma < 0.0)
    {
      gzwarn << "Negative USBL sigma is invalid; using deterministic sigma=0" << std::endl;
      this->dataPtr->m_noiseSigma = 0.0;
    }
  }

  // Get transceiver model name
  if (!_sdf->HasElement("transceiver_model"))
  {
    gzerr << "Missing required parameter <transceiver_model>, "
          << "plugin will not be initialized." << std::endl;
    return;
  }
  this->dataPtr->m_transceiverModelName = _sdf->Get<std::string>("transceiver_model");
  gzmsg << "Transceiver model: " << this->dataPtr->m_transceiverModelName << std::endl;

  auto worldEntity = _ecm.EntityByComponents(gz::sim::components::World());
  this->dataPtr->transceiverEntity = _ecm.EntityByComponents(
    gz::sim::components::Name(this->dataPtr->m_transceiverModelName), gz::sim::components::Model());
  // this->dataPtr->transceiverModel = worldEntity.ModelByName(_ecm,
  // this->dataPtr->transceiverModelName);

  // Define Gazebo publisher for entity's global position
  this->dataPtr->m_globalPosPub = this->dataPtr->m_gzNode.Advertise<gz::msgs::Vector3d>(
    "/" + this->dataPtr->ns + "/" + this->dataPtr->m_transceiverDevice + "_" +
    this->dataPtr->m_transponderID + "/global_position");

  std::string commandResponseTopic(
    "/" + this->dataPtr->ns + "/" + this->dataPtr->m_transceiverDevice + "_" +
    this->dataPtr->m_transceiverID + "/command_response");

  this->dataPtr->m_commandResponsePub =
    this->ros_node_->create_publisher<dave_interfaces::msg::UsblResponse>(commandResponseTopic, 1);

  this->dataPtr->m_iisSub = this->ros_node_->create_subscription<std_msgs::msg::String>(
    "/" + this->dataPtr->ns + "/" + this->dataPtr->m_transponderDevice + "_" +
      this->dataPtr->m_transponderID + "/individual_interrogation_ping",
    1, std::bind(&UsblTransponder::iisRosCallback, this, _1));

  this->dataPtr->m_cisSub = this->ros_node_->create_subscription<std_msgs::msg::String>(
    "/" + this->dataPtr->ns + "/common_interrogation_ping", 1,
    std::bind(&UsblTransponder::cisRosCallback, this, _1));

  this->dataPtr->m_temperatureSub = this->ros_node_->create_subscription<std_msgs::msg::Float64>(
    "/" + this->dataPtr->ns + "/" + this->dataPtr->m_transponderDevice + "_" +
      this->dataPtr->m_transponderID + "/temperature",
    1, std::bind(&UsblTransponder::temperatureRosCallback, this, _1));

  this->dataPtr->m_commandSub =
    this->ros_node_->create_subscription<dave_interfaces::msg::UsblCommand>(
      "/" + this->dataPtr->ns + "/" + this->dataPtr->m_transponderDevice + "_" +
        this->dataPtr->m_transponderID + "/command_request",
      1, std::bind(&UsblTransponder::commandRosCallback, this, _1));

  std::string interrogation_mode_topic = "/" + this->dataPtr->ns + "/" +
                                         this->dataPtr->m_transceiverDevice + "_" +
                                         this->dataPtr->m_transceiverID + "/interrogation_mode";

  this->dataPtr->m_interrogationModeSub =
    this->ros_node_->create_subscription<std_msgs::msg::String>(
      interrogation_mode_topic, 1,
      std::bind(&UsblTransponder::interrogationModeCallback, this, _1));
}

void UsblTransponder::sendLocation()
{
  double noiseX = this->dataPtr->m_noiseMu;
  double noiseY = this->dataPtr->m_noiseMu;
  double noiseZ = this->dataPtr->m_noiseMu;
  if (this->dataPtr->m_noiseSigma > 0.0)
  {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<> distribution(this->dataPtr->m_noiseMu, this->dataPtr->m_noiseSigma);
    noiseX = distribution(gen);
    noiseY = distribution(gen);
    noiseZ = distribution(gen);
  }

  gz::math::Pose3d pose = worldPose(this->dataPtr->linkEntity, *this->dataPtr->ecm);
  gz::math::Vector3<double> position = pose.Pos();
  auto pub_msg = gz::msgs::Vector3d();
  // std::cout << position.X() << " " << position.Y() << " "
  //           << position.Z() << std::endl;
  pub_msg.set_x(position.X() + noiseX);
  pub_msg.set_y(position.Y() + noiseY);
  pub_msg.set_z(position.Z() + noiseZ);
  this->dataPtr->m_globalPosPub.Publish(pub_msg);
}

void UsblTransponder::iisRosCallback(const std_msgs::msg::String::SharedPtr msg)
{
  if (this->dataPtr->m_interrogationMode.compare("individual") == 0)
  {
    gz::math::Pose3d pose_transponder = worldPose(this->dataPtr->linkEntity, *this->dataPtr->ecm);
    gz::math::Pose3d pose_transceiver =
      worldPose(this->dataPtr->transceiverEntity, *this->dataPtr->ecm);
    gz::math::Vector3<double> position_transponder = pose_transponder.Pos();
    gz::math::Vector3<double> position_transceiver = pose_transceiver.Pos();

    // For each kilometer increase in depth (pressure), the sound speed increases by 17 m/s
    // Base on https://dosits.org/tutorials/science/tutorial-speed/
    this->dataPtr->m_soundSpeed = 1540.4 + position_transponder.Z() / 1000 * 17;
    double dist = (position_transponder - position_transceiver).Length();
    std::string command = msg->data;

    if (!command.compare("ping"))
    {
      // gzmsg << this->dataPtr->m_transponderDevice + "_" + this->dataPtr->m_transponderID +
      //            ": Received iis_ping, responding\n";
      // gzmsg << "Distance " << dist << std::endl;
      // gzmsg << "Pose transponder " << position_transponder << std::endl;
      // gzmsg << "Pose transceiver " << position_transceiver << std::endl;
      sleep(dist / this->dataPtr->m_soundSpeed);
      sendLocation();
    }
    else
    {
      gzmsg << "Unknown command, ignore\n";
    }
  }
  else
  {
    gzmsg << "Not in individual mode: cannot send location" << std::endl;
  }
}

void UsblTransponder::cisRosCallback(const std_msgs::msg::String::SharedPtr msg)
{
  if (this->dataPtr->m_interrogationMode.compare("common") == 0)
  {
    gz::math::Pose3d pose_transponder = worldPose(this->dataPtr->linkEntity, *this->dataPtr->ecm);
    gz::math::Pose3d pose_transceiver =
      worldPose(this->dataPtr->transceiverEntity, *this->dataPtr->ecm);
    gz::math::Vector3<double> position_transponder = pose_transponder.Pos();
    gz::math::Vector3<double> position_transceiver = pose_transceiver.Pos();

    // For each kilometer increase in depth (pressure), the sound speed increases by 17 m/s
    // Base on https://dosits.org/tutorials/science/tutorial-speed/
    this->dataPtr->m_soundSpeed = 1540.4 + position_transponder.Z() / 1000 * 17;
    double dist = (position_transponder - position_transceiver).Length();
    std::string command = msg->data;

    if (!command.compare("ping"))
    {
      gzmsg << this->dataPtr->m_transponderDevice + "_" + this->dataPtr->m_transponderID +
                 ": Received cis_ping, responding\n";
      sleep(dist / this->dataPtr->m_soundSpeed);
      sendLocation();
    }
    else
    {
      gzmsg << "Unknown command, ignore\n";
    }
  }
  else
  {
    gzmsg << "Not in common mode: cannot send location";
  }
}

void UsblTransponder::interrogationModeCallback(const std_msgs::msg::String::SharedPtr msg)
{
  this->dataPtr->m_interrogationMode = msg->data;

  gzmsg << "Transponders changed interrogation mode to " << this->dataPtr->m_interrogationMode
        << std::endl;
}

void UsblTransponder::temperatureRosCallback(const std_msgs::msg::Float64::SharedPtr msg)
{
  gz::math::Pose3d pose_transponder = worldPose(this->dataPtr->linkEntity, *this->dataPtr->ecm);
  gz::math::Vector3<double> position_transponder = pose_transponder.Pos();

  this->dataPtr->m_temperature = msg->data;

  // Base on https://dosits.org/tutorials/science/tutorial-speed/
  this->dataPtr->m_soundSpeed =
    1540.4 + position_transponder.Z() / 1000 * 17 + (this->dataPtr->m_temperature - 10) * 4;
  gzmsg << "Detected change of temperature, transponder sound speed is now: "
        << this->dataPtr->m_soundSpeed << " m/s\n";
}

void UsblTransponder::commandRosCallback(const dave_interfaces::msg::UsblCommand msg)
{
  dave_interfaces::msg::UsblResponse response_msg;
  response_msg.data = "Hi from transponder_" + this->dataPtr->m_transponderID;
  response_msg.response_id = 1;

  response_msg.transceiver_id = this->dataPtr->m_transceiverID.back() - '0';
  this->dataPtr->m_commandResponsePub->publish(response_msg);
}

void UsblTransponder::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  // ROS callbacks are wall-time events and must remain responsive while paused.
  rclcpp::spin_some(this->ros_node_);
}

}  // namespace dave_gz_sensor_plugins
