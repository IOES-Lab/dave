#include "dave_gz_sensor_plugins/sea_pressure_sensor.hh"
#include <pressure_sensor_msgs/msgs/SensorPressure.pb.h>
#include <chrono>
#include <gz/math/Pose3.hh>
#include <gz/math/Vector3.hh>
#include <gz/msgs/Utility.hh>
#include <gz/plugin/Register.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/System.hh>
#include <gz/sim/World.hh>
#include <gz/sim/components/CustomSensor.hh>
#include <gz/sim/components/Link.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/ParentEntity.hh>
#include <gz/sim/components/Pose.hh>
#include <gz/sim/components/Sensor.hh>
#include <gz/sim/components/World.hh>
#include <gz/transport/Node.hh>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/fluid_pressure.hpp>
#include "gz/sim/components/Model.hh"

GZ_ADD_PLUGIN(
  dave_gz_sensor_plugins::SubseaPressureSensorPlugin, gz::sim::System,
  dave_gz_sensor_plugins::SubseaPressureSensorPlugin::ISystemConfigure,
  dave_gz_sensor_plugins::SubseaPressureSensorPlugin::ISystemPreUpdate,
  dave_gz_sensor_plugins::SubseaPressureSensorPlugin::ISystemPostUpdate)

namespace dave_gz_sensor_plugins
{
struct SubseaPressureSensorPlugin::PrivateData
{
public:
  /// \brief The world
  gz::sim::World world{gz::sim::kNullEntity};
  /// \brief The world name;
  std::string worldName;
  double saturation;
  gz::sim::EntityComponentManager * ecm = nullptr;
  std::chrono::steady_clock::duration lastMeasurementTime{0};
  bool estimateDepth = false;
  double standardPressure = 101.325;
  double kPaPerM = 9.80638;
  std::shared_ptr<gz::transport::Node> gazeboNode;
  gz::transport::Node::Publisher gazeboSensorOutputPub;
  rclcpp::Publisher<sensor_msgs::msg::FluidPressure>::SharedPtr rosSensorOutputPub;
  std::shared_ptr<rclcpp::Node> rosNode;
  std::string robotNamespace;
  bool gzMsgEnabled = false;
  double noiseAmp = 0.0;
  double noiseSigma = 3.0;
  double inferredDepth = 0.0;
  double pressure = 0.0;
  std::string modelName;
  gz::sim::Model model;
  gz::sim::Entity modelEntity;
};

SubseaPressureSensorPlugin::SubseaPressureSensorPlugin() : dataPtr(std::make_unique<PrivateData>())
{
}

SubseaPressureSensorPlugin::~SubseaPressureSensorPlugin() {}

void SubseaPressureSensorPlugin::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventManager)
{
  if (!rclcpp::ok())
  {
    rclcpp::init(0, nullptr);
  }

  gzdbg << "dave_gz_sensor_plugins::SubseaPressureSensorPlugin::Configure on entity: " << _entity
        << std::endl;

  // Initialize the ROS 2 node
  this->rosNode = std::make_shared<rclcpp::Node>("subsea_pressure_sensor");

  // Initialize the Gazebo node
  this->dataPtr->gazeboNode = std::make_shared<gz::transport::Node>();  // check idts it's needed

  if (!_sdf->HasElement("namespace"))
  {
    gzerr << "Missing required parameter <namespace>, "
          << "plugin will not be initialized." << std::endl;
    return;
  }
  this->dataPtr->robotNamespace = _sdf->Get<std::string>("namespace");

  if (_sdf->HasElement("saturation"))
  {
    this->dataPtr->saturation = _sdf->Get<double>("saturation");
  }
  else
  {
    this->dataPtr->saturation = 3000;  // original xacro file has it as range set at 30000
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

  // this->dataPtr->gazeboNode->Init();
  this->dataPtr->modelEntity = GetModelEntity(this->dataPtr->robotNamespace, _ecm);  // check TODO

  this->dataPtr->rosSensorOutputPub =
    this->rosNode->create_publisher<sensor_msgs::msg::FluidPressure>(
      "sensor_output_topic", rclcpp::QoS(10).reliable());

  if (this->dataPtr->gzMsgEnabled)
  {
    this->dataPtr->gazeboSensorOutputPub =
      this->dataPtr->gazeboNode->Advertise<pressure_sensor_msgs::msgs::Pressure>(
        this->dataPtr->robotNamespace + "/" + "sensor_output_topic");
  }
}
//////////////////////////////////////////

gz::sim::Entity SubseaPressureSensorPlugin::GetModelEntity(
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

gz::math::Pose3d SubseaPressureSensorPlugin::GetModelPose(
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

void SubseaPressureSensorPlugin::PreUpdate(
  const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm)
{
  // Get model pose

  gz::math::Pose3d sea_pressure_sensor_pos = GetModelPose(this->dataPtr->modelEntity, _ecm);

  // const gz::sim::Entity & _entity = _ecm.EntityByComponents(gz::sim::components::WorldPose());
  // gz::math::Vector3 pos;
  // auto pos = GetWorldPose(_entity, _ecm);

  double depth = std::abs(sea_pressure_sensor_pos.Z());
  this->dataPtr->pressure = this->dataPtr->standardPressure;
  if (depth >= 0)
  {
    this->dataPtr->pressure += depth * this->dataPtr->kPaPerM;
  }

  // not adding gaussian noise for now
  // pressure += this->dataPtr->GetGaussianNoise(this->dataPtr->noiseAmp);
  this->dataPtr->pressure += this->dataPtr->noiseAmp;  // noiseAmp is 0.0

  // double inferredDepth = 0.0;
  if (this->dataPtr->estimateDepth)  // estimateDepth is false by default
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
  rosMsg.header.stamp.sec =
    std::chrono::duration_cast<std::chrono::seconds>(_info.simTime).count();  // Time in seconds
  rosMsg.header.stamp.nanosec =
    std::chrono::duration_cast<std::chrono::nanoseconds>(_info.simTime).count() %
    1000000000;  // Time in nanoseconds
  // this->dataPtr->worldName = this->dataPtr->world.Name(_ecm).value();
  rosMsg.fluid_pressure = this->dataPtr->pressure;
  rosMsg.variance = this->dataPtr->noiseSigma * this->dataPtr->noiseSigma;
  this->dataPtr->rosSensorOutputPub->publish(rosMsg);
  this->dataPtr->lastMeasurementTime = _info.simTime;

  if (!_info.paused)
  {
    rclcpp::spin_some(this->rosNode);

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