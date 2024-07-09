namespace gz
{
namespace sim
{
namespace systems
{
namespace dave_simulator_ros
{
class TransientCurrentPlugin : public System,
                               public ISystemConfigure,
                               public ISystemPreUpdate,
                               public ISystemPostUpdate
{
  /// \brief Class constructor
public:
  TransientCurrentPlugin();

  /// \brief Class destructor
public:
  ~TransientCurrentPlugin();

  // Documentation inherited
public:
  void Configure(
    const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
    gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr);

  // Documentation inherited
public:
  void PreUpdate(const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  // Documentation inherited
public:
  void PostUpdate(const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

  // Documentation inherited.
public:
  virtual void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf);

  // Documentation inherited.
public:
  virtual void Init();

  /// \brief Update the simulation state.
  /// \param[in] _info Information used in the update event.
public:
  void Update(const common::UpdateInfo & _info);

  /// \brief Update event
protected:
  event::ConnectionPtr updateConnection;

  /// \brief Pointer to world
protected:
  physics::WorldPtr world;

  /// \brief Pointer to model
protected:
  gz::sim::Entity entity{gz::sim::kNullEntity};
  gz::sim::Model model{gz::sim::kNullEntity};
  gz::sim::Entity modelLink{gz::sim::kNullEntity};

  /// \brief Pointer to sdf
protected:
  sdf::ElementPtr sdf;

  /// \brief Namespace for topics and services
protected:
  std::string ns;

  /// \brief Pointer to this ROS node's handle.
private:
  boost::shared_ptr<ros::NodeHandle> rosNode;

  /// \brief Connects the update event callback
protected:
  virtual void Connect();

  /// \brief Pointer to a node for communication
protected:
  transport::NodePtr node;

  /// \brief Map of publishers
protected:
  std::map<std::string, transport::PublisherPtr> publishers;

  /// \brief Current velocity topic
protected:
  std::string currentVelocityTopic;

  /// \brief Publisher for the flow velocity in the world frame
private:
  ros::Publisher flowVelocityPub;

  /// \brief transient Ocean current topic
protected:
  std::string transientCurrentVelocityTopic;

  /// \brief Subscriber for the transient ocean current database
private:
  ros::Subscriber databaseSub;

  /// \brief A ROS callbackqueue that helps process messages
private:
  ros::CallbackQueue databaseSubQueue;

  /// \brief A thread the keeps running the rosQueue
private:
  std::thread databaseSubQueueThread;

  /// \brief A thread mutex to lock
private:
  std::mutex lock_;

  /// \brief Period after which we should publish a message via ROS.
private:
  std::chrono::steady_clock::duration rosPublishPeriod{0};

  /// \brief Last time we published a message via ROS.
private:
  std::chrono::steady_clock::duration lastRosPublishTime{0};

  /// \brief Convey model state from gazebo topic to outside
protected:
  virtual void UpdateDatabase(
    const dave_gazebo_ros_plugins::StratifiedCurrentDatabase::ConstPtr & _msg);

  /// \brief ROS helper function that processes messages
private:
  void databaseSubThread();

  /// \brief Last update time stamp
protected:
  common::Time lastUpdate;

  /// \brief Last depth index
protected:
  int lastDepthIndex;

  /// \brief Calculate ocean current using database and vehicle state
private:
  void CalculateOceanCurrent();

  /// \brief Current linear velocity vector
protected:
  ignition::math::Vector3d currentVelocity;

  /// \brief Gauss-Markov process instance for the velocity north
protected:
  GaussMarkovProcess currentVelNorthModel;

  /// \brief Gauss-Markov process instance for the velocity east
protected:
  GaussMarkovProcess currentVelEastModel;

  /// \brief Gauss-Markov process instance for the velocity down
protected:
  GaussMarkovProcess currentVelDownModel;

  /// \brief Gauss-Markov noise
protected:
  double noiseAmp_North;

protected:
  double noiseAmp_East;

protected:
  double noiseAmp_Down;

protected:
  double noiseFreq_North;

protected:
  double noiseFreq_East;

protected:
  double noiseFreq_Down;

  /// \brief Publish ocean current
private:
  void PublishCurrentVelocity();

  /// \brief Vector to read database
protected:
  std::vector<ignition::math::Vector3d> database;

  /// \brief Tidal Oscillation interpolation model
protected:
  TidalOscillation tide;

  /// \brief Tidal Oscillation flag
protected:
  bool tideFlag;

  /// \brief Vector to read timeGMT
protected:
  std::vector<std::array<int, 5>> timeGMT;

  /// \brief Vector to read tideVelocities
protected:
  std::vector<double> tideVelocities;

  /// \brief Tidal current harmonic constituents
protected:
  bool tide_Constituents;

protected:
  double M2_amp;

protected:
  double M2_phase;

protected:
  double M2_speed;

protected:
  double S2_amp;

protected:
  double S2_phase;

protected:
  double S2_speed;

protected:
  double N2_amp;

protected:
  double N2_phase;

protected:
  double N2_speed;

  /// \brief Tidal oscillation mean ebb direction
protected:
  double ebbDirection;

  /// \brief Tidal oscillation mean flood direction
protected:
  double floodDirection;

  /// \brief Tidal oscillation world start time (GMT)
protected:
  std::array<int, 5> world_start_time;
}
}  // namespace dave_simulator_ros
}  // namespace systems
}  // namespace sim
}  // namespace gz
