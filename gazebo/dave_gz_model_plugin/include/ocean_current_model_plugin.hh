#ifndef OCEAN_CURRENT_MODEL_PLUGIN_H_
#define OCEAN_CURRENT_MODEL_PLUGIN_H_

#include <dave_gz_ros_plugins/StratifiedCurrentDatabase.hh>
#include <dave_gz_ros_plugins/gauss_markov_process.hh>
// #include <dave_gz_ros_plugins/tidal_oscillation.h>

#include <gz/sim/System.hh>
#include <gz/utilise/ImplPtr.hh>

#include <gz/common/Plugin.hh>
#include <gz/physics/World.hh>
#include <std/shared_ptr.hpp>

#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/vector3.hh>
#include <gz/common/Plugin.hh>
#include <gz/common/Time.hh>
#include <rclcpp/rclcpp.hpp>
#include <std/chrono>
#include <std/map>
#include <std/memory>
#include <std/string>

namespace gz
{
namespace sim
{
namespace systems
{
namespace dave_gz_sensor_plugins
{
class TransientCurrentPlugin : public System,
                               public ISystemConfigure,
                               public ISystemPreUpdate,
                               public ISystemUpdate,
                               public ISystemPostUpdate,
                               public ModelPlugin
{
  /// \brief Class constructor
public:
  TransientCurrentPlugin();            // constructor
  ~TransientCurrentPlugin() override;  // Destructor

  // Configure the plugin with necessary entities and component managers
  void Configure(
    const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
    gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr);

  // Function called before the simulation state updates
  void PreUpdate(const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  // Function called after the simulation state updates
  void PostUpdate(const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

  /// \brief Check if an entity is enabled or not.
  /// \param[in] _entity Target entity
  /// \param[in] _ecm Entity component manager
  /// \return True if buoyancy should be applied.
  bool IsEnabled(Entity _entity, const EntityComponentManager & _ecm) const;

  // Initialize any necessary states before the plugin starts
  virtual void Init();

  /// \brief Pointer to world
protected:
  physics::WorldPtr world;

  /// \brief Pointer to model
  gz::sim::Entity entity{gz::sim::kNullEntity};
  gz::sim::Model model{gz::sim::kNullEntity};
  gz::sim::Entity modelLink{gz::sim::kNullEntity};

  /// \brief Namespace for topics and services
  std::string ns;

  /// \brief Connects the update event callback
  virtual void Connect();

  /// \brief Pointer to a node for communication
  gz::transport::NodePtr node;

  /// \brief Map of publishers
  std::map<std::string, transport::PublisherPtr> publishers;

  /// \brief Current velocity topic and transient current velocity topic
  std::string currentVelocityTopic;
  std::string transientCurrentVelocityTopic;

  /// \brief Convey model state from gazebo topic to outside

  virtual void UpdateDatabase(
    const dave_ros_gz_plugins::StratifiedCurrentDatabase::ConstPtr & _msg);

  /// \brief Last update time stamp
  common::Time lastUpdate;

  /// \brief Last depth index
  int lastDepthIndex;

  /// \brief Current linear velocity vector
  gz::math::Vector3d currentVelocity;

  /// \brief Gauss-Markov process instance for the velocity components
  GaussMarkovProcess currentVelNorthModel;
  GaussMarkovProcess currentVelEastModel;
  GaussMarkovProcess currentVelDownModel;

  /// \brief Gauss-Markov noise
  double noiseAmp_North;
  double noiseAmp_East;
  double noiseAmp_Down;
  double noiseFreq_North;
  double noiseFreq_East;
  double noiseFreq_Down;

  /// \brief Vector to read database
  std::vector<gz::math::Vector3d> database;

  /// \brief Tidal Oscillation interpolation model
  TidalOscillation tide;

  /// \brief Tidal Oscillation flag
  bool tideFlag;

  /// \brief Vector to read timeGMT
  std::vector<std::array<int, 5>> timeGMT;

  /// \brief Vector to read tideVelocities
  std::vector<double> tideVelocities;

  /// \brief Tidal current harmonic constituents
  bool tide_Constituents;
  double M2_amp;
  double M2_phase;
  double M2_speed;
  double S2_amp;
  double S2_phase;
  double S2_speed;
  double N2_amp;
  double N2_phase;
  double N2_speed;

  /// \brief Tidal oscillation mean ebb direction
  double ebbDirection;

  /// \brief Tidal oscillation mean flood direction
  double floodDirection;

  /// \brief Tidal oscillation world start time (GMT)
  std::array<int, 5> world_start_time;

  /// \brief Private data pointer
private:
  std::unique_ptr<BuoyancyPrivate> dataPtr;
  std::shared_ptr<rclcpp::Node> ros_node_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr flowVelocityPub;

private:
  /// \brief Pointer to this ROS node's handle.
  std::shared_ptr<rclcpp::NodeHandle> rosNode;

  /// \brief Publisher for the flow velocity in the world frame
  rclcpp::Publisher flowVelocityPub;

  /// \brief Subscriber for the transient ocean current database
  rclcpp::Subscriber databaseSub;

  /// \brief A ROS callbackqueue that helps process messages
  rclcpp::CallbackQueue databaseSubQueue;

  /// \brief A thread the keeps running the rosQueue
  std::thread databaseSubQueueThread;

  /// \brief A thread mutex to lock
  std::mutex lock_;

  /// \brief Period after which we should publish a message via ROS.
  std::chrono::steady_clock::duration rosPublishPeriod{0};

  /// \brief Last time we published a message via ROS.
  std::chrono::steady_clock::duration lastRosPublishTime{0};

  /// \brief ROS helper function that processes messages
  void databaseSubThread();

  /// \brief Calculate ocean current using database and vehicle state
  void CalculateOceanCurrent();

  /// \brief Publish ocean current
  void PublishCurrentVelocity();
}
}  // namespace dave_gz_sensor_plugins
}  // namespace systems
}  // namespace sim
}  // namespace gz
