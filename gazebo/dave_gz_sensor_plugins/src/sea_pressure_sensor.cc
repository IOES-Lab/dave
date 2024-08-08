#include "dave_gz_sensor_plugins/sea_pressure_sensor.hh"
#include <gz/msgs/fluid_pressure.pb.h>
#include <chrono>
#include <geometry_msgs/msg/point_stamped.hpp>
#include <gz/math/Pose3.hh>
#include <gz/math/Vector3.hh>
#include <gz/msgs/Utility.hh>
#include <gz/plugin/Register.hh>
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
  double saturation;
  gz::sim::EntityComponentManager * ecm = nullptr;
  std::chrono::steady_clock::duration lastMeasurementTime{0};
  bool estimateDepth;
  double standardPressure = 101.325;
  double kPaPerM = 9.80638;
  std::shared_ptr<gz::transport::Node> gazeboNode;
  gz::transport::Node::Publisher gz_pressure_sensor_pub;
  rclcpp::Publisher<sensor_msgs::msg::FluidPressure>::SharedPtr ros_pressure_sensor_pub;
  rclcpp::Publisher<geometry_msgs::msg::PointStamped>::SharedPtr ros_depth_estimate_pub;
  std::shared_ptr<rclcpp::Node> rosNode;
  std::string robotNamespace;
  double noiseAmp = 0.0;
  double noiseSigma = 3.0;
  double inferredDepth = 0.0;
  double pressure = 0.0;
  std::string modelName;
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
  this->dataPtr->gazeboNode = std::make_shared<gz::transport::Node>();

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
    this->dataPtr->saturation = 3000;
  }

  if (_sdf->HasElement("estimate_depth_on"))
  {
    this->dataPtr->estimateDepth = _sdf->Get<bool>("estimate_depth_on");
  }
  else
  {
    this->dataPtr->estimateDepth = true;
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
  this->dataPtr->modelEntity = GetModelEntity(this->dataPtr->robotNamespace, _ecm);

  this->dataPtr->ros_pressure_sensor_pub =
    this->rosNode->create_publisher<sensor_msgs::msg::FluidPressure>(
      this->dataPtr->robotNamespace + "/" + "Pressure", rclcpp::QoS(10).reliable());

  if (this->dataPtr->estimateDepth)
  {
    this->dataPtr->ros_depth_estimate_pub =
      this->rosNode->create_publisher<geometry_msgs::msg::PointStamped>(
        this->dataPtr->robotNamespace + "/" + "Pressure_depth", rclcpp::QoS(10).reliable());
  }

  this->dataPtr->gz_pressure_sensor_pub =
    this->dataPtr->gazeboNode->Advertise<gz::msgs::FluidPressure>(
      this->dataPtr->robotNamespace + "/" + "Pressure");
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
  double depth = std::abs(sea_pressure_sensor_pos.Z());
  this->dataPtr->pressure = this->dataPtr->standardPressure;
  if (depth >= 0)
  {
    this->dataPtr->pressure += depth * this->dataPtr->kPaPerM;
  }

  // not adding gaussian noise for now, Future Work (TODO)
  // pressure += this->dataPtr->GetGaussianNoise(this->dataPtr->noiseAmp);
  this->dataPtr->pressure += this->dataPtr->noiseAmp;  // noiseAmp is 0.0

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
  this->dataPtr->lastMeasurementTime = _info.simTime;

  // Publishing Sea_Pressure and depth estimate on gazebo topic
  gz::msgs::FluidPressure gzPressureMsg;
  gzPressureMsg.set_pressure(this->dataPtr->pressure);
  gzPressureMsg.set_variance(this->dataPtr->noiseSigma * this->dataPtr->noiseSigma);

  // Publishing the pressure message
  this->dataPtr->gz_pressure_sensor_pub.Publish(gzPressureMsg);

  // Publishing Sea_Pressure on Ros Topic
  sensor_msgs::msg::FluidPressure rosPressureMsg;
  rosPressureMsg.header.stamp.sec =
    std::chrono::duration_cast<std::chrono::seconds>(_info.simTime).count();  // Time in seconds
  rosPressureMsg.header.stamp.nanosec =
    std::chrono::duration_cast<std::chrono::nanoseconds>(_info.simTime).count() %
    1000000000;  // Time in nanoseconds
  rosPressureMsg.fluid_pressure = this->dataPtr->pressure;
  rosPressureMsg.variance = this->dataPtr->noiseSigma * this->dataPtr->noiseSigma;
  this->dataPtr->ros_pressure_sensor_pub->publish(rosPressureMsg);

  // publishing depth message
  if (this->dataPtr->estimateDepth)
  {
    geometry_msgs::msg::PointStamped rosDepthMsg;
    rosDepthMsg.point.z = this->dataPtr->inferredDepth;
    rosDepthMsg.header.stamp.sec =
      std::chrono::duration_cast<std::chrono::seconds>(this->dataPtr->lastMeasurementTime).count();
    this->dataPtr->ros_depth_estimate_pub->publish(rosDepthMsg);
  }

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