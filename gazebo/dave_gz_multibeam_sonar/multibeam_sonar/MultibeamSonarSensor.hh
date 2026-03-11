#ifndef GZ_SENSORS_MULTIBEAMSONAR_HH_
#define GZ_SENSORS_MULTIBEAMSONAR_HH_

#include <chrono>
#include <complex>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <valarray>
#include <vector>

#include <gz/msgs/pointcloud_packed.pb.h>
#include <gz/math/Pose3.hh>
#include <gz/math/Vector2.hh>
#include <gz/math/Vector3.hh>
#include <gz/sensors/EnvironmentalData.hh>
#include <gz/sensors/RenderingSensor.hh>
#include <gz/transport/Node.hh>
#include <marine_acoustic_msgs/msg/projected_sonar_image.hpp>
#include <opencv2/core/core.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include "AcousticBeam.hh"
#include "AxisAlignedPatch2.hh"

namespace gz
{
typedef std::complex<float> Complex;
typedef std::valarray<Complex> CArray;
typedef std::valarray<CArray> CArray2D;

typedef std::valarray<float> Array;
typedef std::valarray<Array> Array2D;

namespace sensors
{

/// \brief Kinematic state for an entity in the world.

/// All quantities are defined w.r.t. the world frame.
struct EntityKinematicState
{
  gz::math::Pose3d pose;
  gz::math::Vector3d linearVelocity;
  gz::math::Vector3d angularVelocity;
};

/// \brief Kinematic state for all entities in the world.
using WorldKinematicState = std::unordered_map<uint64_t, EntityKinematicState>;

/// \brief Kinematic state for all entities in the world.
struct WorldState
{
  WorldKinematicState kinematics;
  gz::math::SphericalCoordinates origin;
};

class MultibeamSonarSensor : public gz::sensors::RenderingSensor
{
public:
  MultibeamSonarSensor();

  ~MultibeamSonarSensor();

  /// Inherits documentation from parent class
  virtual bool Load(const sdf::Sensor & _sdf) override;

  /// Inherits documentation from parent class
  virtual bool Load(sdf::ElementPtr _sdf) override;

  /// Inherits documentation from parent class
  virtual bool Update(const std::chrono::steady_clock::duration & _now) override;

  /// Perform any sensor updates after the rendering pass
  virtual void PostUpdate(const std::chrono::steady_clock::duration & _now);

  /// Inherits documentation from parent class
  void SetScene(gz::rendering::ScenePtr _scene) override;

  /// \brief Set this sensor's entity ID (for world state lookup).
  void SetEntity(uint64_t entity);

  /// \brief Set world `_state` to support DVL water and bottom-tracking.

  void SetWorldState(const WorldState & _state);

  /// \brief Set environmental `_data` to support DVL water-tracking.

  void SetEnvironmentalData(const EnvironmentalData & _data);

  /// \brief Inherits documentation from parent class
  virtual bool HasConnections() const override;

  /// \brief Yield rendering sensors that underpin the implementation.
  ///
  /// \internal
  std::vector<gz::rendering::SensorPtr> RenderingSensors() const;

private:
  class Implementation
  {
  public:
    // Mutexes
    std::mutex lock_;

    GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
    mutable std::mutex rayMutex;
    GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
    // ROS node pointer
    std::shared_ptr<rclcpp::Node> ros_node_;

    // SDF sensor element
    sdf::ElementPtr sensorSdf;

    // Sensor entity ID
    uint64_t entityId{0};

    // Initialization flag
    bool initialized = false;

    // Initialization methods
    bool Initialize(MultibeamSonarSensor * _sensor);
    bool InitializeBeamArrangement(MultibeamSonarSensor * _sensor);

    // Ray buffer and channels
    float * rayBuffer = nullptr;
    const unsigned int kChannelCount = 3u;

    // Sensor properties
    double maximumRange;
    double hFOV;
    double vFOV;
    int nRays;
    int nBeams;
    float * elevation_angles;
    int nFreq;
    double sonarFreq;
    double bandwidth;
    double soundSpeed;
    double maxDistance;
    double sourceLevel;
    double absorption;
    double attenuation;
    double verticalFOV;
    float * window;
    float plotScaler;
    float sensorGain;

    // Topics and frame names
    std::string pointCloudTopicName;
    std::string sonarImageRawTopicName;
    std::string sonarImageTopicName;
    std::string frameName;
    std::string frameId;  // for non-optical frame id from sensor

    // Ray parameters
    int raySkips;
    int ray_nAzimuthRays;
    int ray_nElevationRays;
    float * rangeVector;

    // Sonar image parameters
    bool blazingFlag;

    // Debug flags and reflectivity
    bool debugFlag;
    bool constMu;
    double mu;

    // Beam corrector
    float beamCorrectorSum;
    float ** beamCorrector;

    // OpenCV images
    cv::Mat pointCloudImage;
    cv::Mat reflectivityImage;
    cv::Mat randImage;

    // Angles
    std::vector<float> azimuth_angles;

    // World state pointer
    const WorldState * worldState;

    // Rendering sensors
    gz::rendering::GpuRaysPtr raySensor;
    gz::rendering::CameraPtr imageSensor;

    struct
    {
      gz::math::Vector2d offset;  /// Azimuth and elevation offsets
      gz::math::Vector2d step;    /// Azimuth and elevation steps
    } raySensorIntrinsics;

    // Callback and computation methods
    void OnNewFrame(
      const float * _scan, unsigned int _width, unsigned int _height, unsigned int _channels,
      const std::string & /*_format*/);
    void ComputeSonarImage();
    cv::Mat ComputeNormalImage(cv::Mat & depth);
    void ComputeCorrector();

    // Connections
    gz::common::ConnectionPtr rayConnection;
    gz::common::ConnectionPtr sceneChangeConnection;

    // Acoustic beams and transforms
    std::vector<AcousticBeam> beams;
    gz::math::Quaterniond referenceFrameRotation;
    const gz::math::Pose3d beamsFrameTransform{
      gz::math::Vector3d::Zero, gz::math::Quaterniond{0., GZ_PI / 2., 0.}};

    // Acoustic beam targets and patches
    std::vector<std::optional<ObjectTarget>> beamTargets;
    std::vector<AxisAlignedPatch2i> beamScanPatches;

    // Point cloud message
    msgs::PointCloudPacked pointMsg;
    void FillPointCloudMsg(const float * _rayBuffer);

    // Sonar and normal image messages
    sensor_msgs::msg::Image sonarImgMsg;
    sensor_msgs::msg::Image normalImgMsg;
    marine_acoustic_msgs::msg::ProjectedSonarImage sonarRawDataMsg;

    // Transport nodes and publishers
    gz::transport::Node node;
    gz::transport::Node::Publisher pointPub;
    gz::transport::Node::Publisher pointFloatPub;
    rclcpp::Publisher<marine_acoustic_msgs::msg::ProjectedSonarImage>::SharedPtr sonarImageRawPub;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr sonarImagePub;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr normalImagePub;

    // Publishing flags
    bool publishingPointCloud = false;
    bool publishingSonarImage = false;

    // Logging
    std::ofstream writeLog;
    u_int64_t writeCounter;
    u_int64_t writeNumber;
    u_int64_t writeInterval;
    bool writeLogFlag;

    // Time
    double lastMeasurementTime;
  };

  std::unique_ptr<Implementation> dataPtr;
};

///////////////////////////////////////////
inline double unnormalized_sinc(double t)
{
  try
  {
    double results = sin(t) / t;
    if (results != results)
    {
      return 1.0;
    }
    else
    {
      return sin(t) / t;
    }
  }
  catch (int expn)
  {
    return 1.0;
  }
}

}  // namespace sensors
}  // namespace gz

#endif  // GZ_SENSORS_MULTIBEAMSONAR_HH_
