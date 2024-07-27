#ifndef dave_gz_sensor_plugins__SUBSEA_PRESSURE_SENSOR_HH_
#define dave_gz_sensor_plugins__SUBSEA_PRESSURE_SENSOR_HH_

// #include <sensor_msgs/FluidPressure.h>
#include <pressure_sensor_msgs/msgs/SensorPressure.pb.h>
#include <gz/sim/System.hh>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/service.hpp>
#include <sensor_msgs/msg/fluid_pressure.hpp>

namespace dave_gz_sensor_plugins
{
class SubseaPressureSensorPlugin : public gz::sim::System,
                                   public gz::sim::ISystemConfigure,
                                   public gz::sim::ISystemPreUpdate,
                                   public gz::sim::ISystemPostUpdate
{
public:
  SubseaPressureSensorPlugin();
  ~SubseaPressureSensorPlugin();

  /// \brief Load the plugin
  void Configure(
    const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
    gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr);

  /// \brief Update sensor measurement
  void PreUpdate(const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  void PostUpdate(const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

private:
  struct PrivateData;
  std::unique_ptr<PrivateData> dataPtr;
};
}  // namespace dave_gz_sensor_plugins

#endif  // __UUV_SUBSEA_PRESSURE_ROS_PLUGIN_HH__