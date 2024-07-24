#include "dave_gz_sensor_plugins/Sea_pressure_sensor.hh"
#include <gz/math/Pose3.hh>
#include <gz/math/Vector3.hh>
#include <gz/msgs/Utility.hh>
#include <gz/plugin/Register.hh>
#include <gz/sim/World.hh>
#include <gz/sim/components/CustomSensor.hh>
#include <gz/sim/components/Link.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/Pose.hh>
#include <gz/sim/components/Sensor.hh>
#include <gz/sim/components/World.hh>

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
  std::shared_ptr < rclcpp::Publisher<sensor_msgs::msg::FluidPressure> rosSensorOutputPub;
  std::shared_ptr<gz::transport::Node> gazeboNode;
  std::shared_ptr<rclcpp::Node> rosNode;
  std::shared_ptr < gz::transport::Publisher<sensor_msgs::msgs::Pressure> gazeboSensorOutputPub;
  std::string robotNamespace;
  bool gzMsgEnabled;
  double noiseAmp;
  double noiseSigma;
  double lastMeasurementTime;
};

SubseaPressureSensorPlugin::SubseaPressureSensorPlugin() : dataPtr(std::make_unique<PrivateData>())
{
}

SubseaPressureSensorPlugin::~SubseaPressureSensorPlugin() {}

void SubseaPressureSensorPlugin::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & /*_eventManager*/)
{
  this->dataPtr->world = gz::sim::World(_ecm.EntityByComponents(gz::sim::components::World()));
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

  this->dataPtr->rosSensorOutputPub =
    this->rosNode->create_publisher<sensor_msgs::msg::FluidPressure>(
      this->dataPtr->sensorOutputTopic, rclcpp::QoS(10).reliable());

  if (this->dataPtr->gzMsgEnabled)
  {
    this->dataPtr->gazeboSensorOutputPub = this->gazeboNode->Advertise<sensor_msgs::msgs::Pressure>(
      this->dataPtr->robotNamespace + "/" + this->dataPtr->sensorOutputTopic);
  }
}

void SubseaPressureSensorPlugin::PreUpdate(
  const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm)
{
  this->dataPtr->PublishState();

  if (!this->dataPtr->EnableMeasurement(_info))
  {
    return;
  }

  const gz::sim::components::WorldPose * pComp =
    _ecm.Component<gz::sim::components::WorldPose>(this->dataPtr->linkEntity);
  gz::math::Vector3d pos;
  if (pComp)
  {
    pos = pComp->Data().Pos();
  }

  double depth = std::abs(pos.Z());
  double pressure = this->dataPtr->standardPressure;
  if (depth >= 0)
  {
    pressure += depth * this->dataPtr->kPaPerM;
  }

  pressure += this->dataPtr->GetGaussianNoise(this->dataPtr->noiseAmp);

  double inferredDepth = 0.0;
  if (this->dataPtr->estimateDepth)
  {
    inferredDepth = (pressure - this->dataPtr->standardPressure) / this->dataPtr->kPaPerM;
  }
}

void SubseaPressureSensorPlugin::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  this->dataPtr->PublishState();

  if (!this->dataPtr->EnableMeasurement(_info))
  {
    return;
  }

  if (this->dataPtr->gzMsgEnabled)
  {
    sensor_msgs::msgs::Pressure gazeboMsg;
    gazeboMsg.set_pressure(pressure);
    gazeboMsg.set_stddev(this->dataPtr->noiseSigma);
    if (this->dataPtr->estimateDepth)
    {
      gazeboMsg.set_depth(inferredDepth);
    }
    this->dataPtr->gazeboSensorOutputPub->Publish(gazeboMsg);
  }

  sensor_msgs::msg::FluidPressure rosMsg;
  rosMsg.header.stamp.sec = _info.simTime.sec;
  rosMsg.header.stamp.nanosec = _info.simTime.nsec;
  this->dataPtr->worldName = this->dataPtr->world.Name(_ecm).value();
  rosMsg.fluid_pressure = pressure;
  rosMsg.variance = this->dataPtr->noiseSigma * this->dataPtr->noiseSigma;
  this->dataPtr->rosSensorOutputPub->publish(rosMsg);
  this->dataPtr->lastMeasurementTime = _info.simTime;
}

}  // namespace dave_gz_sensor_plugins