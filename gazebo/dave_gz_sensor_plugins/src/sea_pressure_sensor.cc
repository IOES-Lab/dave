#include "dave_gz_sensor_plugins/sea_pressure_sensor.hh"
#include <pressure_sensor_msgs/msgs/SensorPressure.pb.h>
#include <chrono>
#include <gz/math/Pose3.hh>
#include <gz/math/Vector3.hh>
#include <gz/msgs/Utility.hh>
#include <gz/plugin/Register.hh>
#include <gz/sim/System.hh>
#include <gz/sim/World.hh>
#include <gz/sim/components/CustomSensor.hh>
#include <gz/sim/components/Link.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/Pose.hh>
#include <gz/sim/components/Sensor.hh>
#include <gz/sim/components/World.hh>
#include <gz/transport/Node.hh>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/fluid_pressure.hpp>

GZ_ADD_PLUGIN(
  dave_gz_sensor_plugins::SubseaPressureSensorPlugin, gz::sim::System,
  dave_gz_sensor_plugins::SubseaPressureSensorPlugin::ISystemConfigure,
  dave_gz_sensor_plugins::SubseaPressureSensorPlugin::ISystemPreUpdate,
  dave_gz_sensor_plugins::SubseaPressureSensorPlugin::ISystemPostUpdate)

namespace dave_gz_sensor_plugins
{
struct SubseaPressureSensorPlugin::PrivateData
{
  std::string ns;
  gz::sim::EntityComponentManager * ecm;
  /// \brief The world
public:
  gz::sim::World world{gz::sim::kNullEntity};

  /// \brief The world name;
public:
  std::string worldName;
  double saturation;

public:
  std::chrono::steady_clock::duration lastMeasurementTime{0};
  bool estimateDepth;
  double standardPressure;
  double kPaPerM;
  std::shared_ptr<gz::transport::Node> gazeboNode;
  // gz::transport::Node::Publisher<pressure_sensor_msgs::msgs::Pressure> gazeboSensorOutputPub;
  gz::transport::Node::Publisher gazeboSensorOutputPub;
  rclcpp::Publisher<sensor_msgs::msg::FluidPressure>::SharedPtr rosSensorOutputPub;
  std::shared_ptr<rclcpp::Node> rosNode;
  std::string robotNamespace;
  bool gzMsgEnabled;
  double noiseAmp;
  double noiseSigma;
  double inferredDepth;
  double pressure;
  // auto pos;
};

SubseaPressureSensorPlugin::SubseaPressureSensorPlugin() : dataPtr(std::make_unique<PrivateData>())
{
  dataPtr->inferredDepth = 0.0;
  // dataPtr->gzMsgEnabled = true;
  // dataPtr->noiseAmp = 0.1;
  // dataPtr->noiseSigma = 0.01;
}

SubseaPressureSensorPlugin::~SubseaPressureSensorPlugin() {}

void SubseaPressureSensorPlugin::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & /*_eventManager*/)
{
  this->dataPtr->world = gz::sim::World(_ecm.EntityByComponents(gz::sim::components::World()));
  // this->dataPtr->sensorOutputTopic = "sensor_output_topic";

  this->dataPtr->rosNode = std::make_shared<rclcpp::Node>("subsea_pressure_sensor");  // 2 (check)
  this->dataPtr->gazeboNode = std::make_shared<gz::transport::Node>();
  // this->dataPtr->gazeboNode->Init();

  if (!this->dataPtr->world.Valid(_ecm))
  {
    gzerr << "World entity not found" << std::endl;
    return;
  }

  if (_sdf->HasElement("saturation"))
  {
    this->dataPtr->saturation = _sdf->Get<double>("saturation");
  }
  else
  {
    this->dataPtr->saturation = 3000;
  }

  if (_sdf->HasElement("estimate_depth_on"))
  {
    this->dataPtr->estimateDepth = _sdf->Get<bool>("estimate_depth_on");
  }
  else
  {
    this->dataPtr->estimateDepth = false;
  }

  if (_sdf->HasElement("standard_pressure"))
  {
    this->dataPtr->standardPressure = _sdf->Get<double>("standard_pressure");
  }
  else
  {
    this->dataPtr->standardPressure = 101.325;
  }

  if (_sdf->HasElement("kPa_per_meter"))
  {
    this->dataPtr->kPaPerM = _sdf->Get<double>("kPa_per_meter");
  }
  else
  {
    this->dataPtr->kPaPerM = 9.80638;
  }

  // Initialize the Gazebo node
  this->dataPtr->gazeboNode = std::make_shared<gz::transport::Node>();
  // this->dataPtr->gazeboNode->Init();

  this->dataPtr->rosSensorOutputPub =
    this->dataPtr->rosNode->create_publisher<sensor_msgs::msg::FluidPressure>(
      "sensor_output_topic", rclcpp::QoS(10).reliable());

  if (this->dataPtr->gzMsgEnabled)
  {
    this->dataPtr->gazeboSensorOutputPub =
      this->dataPtr->gazeboNode->Advertise<pressure_sensor_msgs::msgs::Pressure>(
        this->dataPtr->robotNamespace + "/" + "sensor_output_topic");
  }
  // {
  //   this->dataPtr->gazeboSensorOutputPub =
  //   this->gazeboNode->Advertise<sensor_msgs::msgs::Pressure>(
  //     this->dataPtr->robotNamespace + "/" + "sensor_output_topic");
  // } (check the issue is that the sensor output topic is not defined) hence for now the custom
  // argument is passed. The same is done with the rosnode.
}
//////////////////////////////////////////

gz::math::Pose3d GetWorldPose(
  const gz::sim::Entity & _entity, gz::sim::EntityComponentManager & _ecm)
{
  // Ensure the WorldPose component exists
  if (!_ecm.Component<gz::sim::components::WorldPose>(_entity))
  {
    _ecm.CreateComponent(_entity, gz::sim::components::WorldPose());
  }

  // Get the WorldPose component
  const auto * worldPoseComp = _ecm.Component<gz::sim::components::WorldPose>(_entity);
  if (worldPoseComp)
  {
    return worldPoseComp->Data();
  }
  else
  {
    gzerr << "WorldPose component not found for entity: " << _entity << std::endl;
    return gz::math::Pose3d::Zero;
  }
}

void SubseaPressureSensorPlugin::PreUpdate(
  const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm)
{
  // this->dataPtr->PublishState();

  // if (!this->dataPtr->EnableMeasurement(_info))
  // {
  //   return;
  // }

  // const gz::sim::components::WorldPose * pComp =
  //   _ecm.Component<gz::sim::components::WorldPose>(_entity);  // check
  // gz::math::Vector3d pos;
  // if (pComp)
  // {
  //   pos = pComp->Data().Pos();
  // }
  const gz::sim::Entity & _entity = _ecm.EntityByComponents(gz::sim::components::WorldPose());
  // gz::math::Vector3 pos;
  auto pos = GetWorldPose(_entity, _ecm);

  // gz::math::Pose3d pose = GetWorldPose(_entity, _ecm);
  // gz::math::Vector3d pos = pose.Pos();
  ////////////
  // const gz::sim::components::WorldPose * worldPoseComp =
  //   _ecm.Component<gz::sim::components::WorldPose>(this->dataPtr->entity);

  // if (worldPoseComp)
  // {
  //   gz::math::Pose3d worldPose = worldPoseComp->Data();
  //   // Use 'worldPose' as needed
  // }
  ////////////

  double depth = std::abs(pos.Z());
  this->dataPtr->pressure = this->dataPtr->standardPressure;
  if (depth >= 0)
  {
    this->dataPtr->pressure += depth * this->dataPtr->kPaPerM;
  }

  // not adding gaussian noise for now
  // pressure += this->dataPtr->GetGaussianNoise(this->dataPtr->noiseAmp);
  this->dataPtr->pressure += this->dataPtr->noiseAmp;

  // double inferredDepth = 0.0;
  if (this->dataPtr->estimateDepth)
  {
    this->dataPtr->inferredDepth =
      (this->dataPtr->pressure - this->dataPtr->standardPressure) / this->dataPtr->kPaPerM;
  }
}

void SubseaPressureSensorPlugin::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  // this->dataPtr->PublishState();

  // if (!this->dataPtr->EnableMeasurement(_info))
  // {
  //   return;
  // }

  if (this->dataPtr->gzMsgEnabled)
  {
    pressure_sensor_msgs::msgs::Pressure gazeboMsg;
    gazeboMsg.set_pressure(this->dataPtr->pressure);
    gazeboMsg.set_stddev(this->dataPtr->noiseSigma);
    if (this->dataPtr->estimateDepth)
    {
      gazeboMsg.set_depth(this->dataPtr->inferredDepth);
    }
    this->dataPtr->gazeboSensorOutputPub.Publish(gazeboMsg);
  }

  sensor_msgs::msg::FluidPressure rosMsg;
  // rosMsg.header.stamp.sec = _info.simTime.sec;
  // rosMsg.header.stamp.nanosec = _info.simTime.nsec;
  rosMsg.header.stamp.sec = std::chrono::duration_cast<std::chrono::seconds>(_info.simTime).count();
  rosMsg.header.stamp.nanosec =
    std::chrono::duration_cast<std::chrono::nanoseconds>(_info.simTime).count() % 1000000000;
  this->dataPtr->worldName = this->dataPtr->world.Name(_ecm).value();
  rosMsg.fluid_pressure = this->dataPtr->pressure;
  rosMsg.variance = this->dataPtr->noiseSigma * this->dataPtr->noiseSigma;
  this->dataPtr->rosSensorOutputPub->publish(rosMsg);
  this->dataPtr->lastMeasurementTime = _info.simTime;

  if (!_info.paused)
  {
    rclcpp::spin_some(this->dataPtr->rosNode);

    if (_info.iterations % 1000 == 0)
    {
      gzmsg << "dave_ros_gz_plugins::SubseaPressureSensorPlugin::PostUpdate" << std::endl;
    }
  }
}

}  // namespace dave_gz_sensor_plugins

/////////////////////////////////////////////////
// bool SubseaPressureSensorPlugin::EnableMeasurement(const gz::sim::UpdateInfo & _info) const
// {
//   common::Time current_time = _info.simTime;
//   double dt = (current_time - this->lastMeasurementTime).Double();
//   return dt >= 1.0 / this->updateRate && this->isReferenceInit && this->isOn.data;
// }

/////////////////////////////////////////////////
// this->noiseModels["default"]: A map that likely holds different noise models, with “default”
// being a standard Gaussian distribution function.

// double ROSBasePlugin::GetGaussianNoise(double _amp)
// {
//   return _amp * this->noiseModels["default"](this->rndGen);
// }

/////////////////////////////////////////////////
// bool SubseaPressureSensorPlugin::GetSDFParam(
//   const std::shared_ptr<const sdf::Element> & _sdf, const std::string & name, T & param,
//   const T & default_value, const bool & verbose = false)
// {
//   if (sdf->HasElement(name))
//   {
//     param = sdf->GetElement(name)->Get<T>();
//     return true;
//   }
//   else
//   {
//     param = default_value;
//     if (verbose)
//     {
//       gzerr << "[uuv_sensor_plugins] Please specify a value for parameter \"" << name << "\".\n";
//     }
//   }
//   return false;
// }