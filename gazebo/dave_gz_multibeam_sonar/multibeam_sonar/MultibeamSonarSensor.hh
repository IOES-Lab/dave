/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
 *
*/
#ifndef MULTIBEAMSONAR_HH_
#define MULTIBEAMSONAR_HH_

#include <memory>
#include <cstdint>
#include <string>

#include <sdf/sdf.hh>

#include <gz/common/Event.hh>
#include <gz/utils/SuppressWarning.hh>

#include <gz/msgs/image.pb.h>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
#include <gz/rendering/DepthCamera.hh>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "gz/sensors/depth_camera/Export.hh"
#include "gz/sensors/CameraSensor.hh"
#include "gz/sensors/Export.hh"
#include "gz/sensors/Sensor.hh"

#include <gz/sensors/SensorTypes.hh>
#include <gz/transport/Node.hh>

#include "./gz_sensor/PointCloudUtil.hh"
#include "./gz_sensor/CameraSensorUtil.hh"

namespace custom
{

  inline namespace GZ_SENSORS_VERSION_NAMESPACE {
  /// \brief Private data for MultibeamSonarSensor
  class MultibeamSonarSensorPrivate
  {
    /// \brief Save an image
    /// \param[in] _data the image data to be saved
    /// \param[in] _width width of image in pixels
    /// \param[in] _height height of image in pixels
    /// \param[in] _format The format the data is in
    /// \return True if the image was saved successfully. False can mean
    /// that the path provided to the constructor does exist and creation
    /// of the path was not possible.
    /// \sa ImageSaver
    public: bool SaveImage(const float *_data, unsigned int _width,
      unsigned int _height, gz::common::Image::PixelFormatType _format);

    /// \brief Helper function to convert depth data to depth image
    /// \param[in] _data depth data
    /// \param[out] _imageBuffer resulting depth image data
    /// \param[in] _width width of image
    /// \param[in] _height height of image
    public: bool ConvertDepthToImage(const float *_data,
      unsigned char *_imageBuffer, unsigned int _width, unsigned int _height);

    /// \brief node to create publisher
    public: gz::transport::Node node;

    /// \brief publisher to publish images
    public: gz::transport::Node::Publisher pub;

    /// \brief true if Load() has been called and was successful
    public: bool initialized = false;

      /// \brief Rendering camera
    public: gz::rendering::DepthCameraPtr depthCamera;

    /// \brief Depth data buffer.
    public: float *depthBuffer = nullptr;

    /// \brief point cloud data buffer.
    public: float *pointCloudBuffer = nullptr;

    /// \brief xyz data buffer.
    public: float *xyzBuffer = nullptr;

    /// \brief Near clip distance.
    public: float near = 0.0;

    /// \brief Pointer to an image to be published
    public: gz::rendering::Image image;

    /// \brief Noise added to sensor data
    public: std::map<gz::sensors::SensorNoiseType, gz::sensors::NoisePtr> noises;

    /// \brief Event that is used to trigger callbacks when a new image
    /// is generated
    public: gz::common::EventT<
            void(const gz::msgs::Image &)> imageEvent;

    /// \brief Connection from depth camera with new depth data
    public: gz::common::ConnectionPtr depthConnection;

    /// \brief Connection from depth camera with new point cloud data
    public: gz::common::ConnectionPtr pointCloudConnection;

    /// \brief Connection to the Manager's scene change event.
    public: gz::common::ConnectionPtr sceneChangeConnection;

    /// \brief Just a mutex for thread safety
    public: std::mutex mutex;

    /// \brief True to save images
    public: bool saveImage = false;

    /// \brief path directory to where images are saved
    public: std::string saveImagePath = "./";

    /// \prefix of an image name
    public: std::string saveImagePrefix = "./";

    /// \brief counter used to set the image filename
    public: std::uint64_t saveImageCounter = 0;

    /// \brief SDF Sensor DOM object.
    public: sdf::Sensor sdfSensor;

    /// \brief The point cloud message.
    public: gz::msgs::PointCloudPacked pointMsg;

    /// \brief Helper class that can fill a msgs::PointCloudPacked
    /// image and depth data.
    public: gz::sensors::PointCloudUtil pointsUtil;

    /// \brief publisher to publish point cloud
    public: gz::transport::Node::Publisher pointPub;
  };

  /// \brief Example sensor that publishes the total distance travelled by a
  /// robot, with noise.
  class GZ_SENSORS_DEPTH_CAMERA_VISIBLE MultibeamSonarSensor
    : public gz::sensors::CameraSensor
  {
    /// \brief constructor
    public: MultibeamSonarSensor();

    /// \brief destructor
    public: virtual ~MultibeamSonarSensor();

    /// \brief Load the sensor based on data from an sdf::Sensor object.
    /// \param[in] _sdf SDF Sensor parameters.
    /// \return true if loading was successful
    public: virtual bool Load(const sdf::Sensor &_sdf) override;

    /// \brief Load the sensor with SDF parameters.
    /// \param[in] _sdf SDF Sensor parameters.
    /// \return true if loading was successful
    public: virtual bool Load(sdf::ElementPtr _sdf) override;

    /// \brief Initialize values in the sensor
    /// \return True on success
    public: virtual bool Init() override;

    /// \brief Update the sensor and generate data
    /// \param[in] _now The current time
    /// \return True if the update was successfull
    public: virtual bool Update(
      const std::chrono::steady_clock::duration &_now) override;

    /// \brief Get a pointer to the rendering depth camera
    /// \return Rendering depth camera
    public: virtual gz::rendering::DepthCameraPtr DepthCamera() const;

    /// \brief Depth data callback used to get the data from the sensor
    /// \param[in] _scan pointer to the data from the sensor
    /// \param[in] _width width of the depth image
    /// \param[in] _height height of the depth image
    /// \param[in] _channel bytes used for the depth data
    /// \param[in] _format string with the format
    public: void OnNewDepthFrame(const float *_scan,
                unsigned int _width, unsigned int _height,
                unsigned int _channel,
                const std::string &_format);

    /// \brief Point cloud data callback used to get the data from the sensor
    /// \param[in] _scan pointer to the data from the sensor
    /// \param[in] _width width of the point cloud image
    /// \param[in] _height height of the point cloud image
    /// \param[in] _channels bytes used for the point cloud data
    /// \param[in] _format string with the format
    public: void OnNewRgbPointCloud(const float *_scan,
                unsigned int _width, unsigned int _height,
                unsigned int _channels,
                const std::string &_format);

    /// \brief Set a callback to be called when image frame data is
    /// generated.
    /// \param[in] _callback This callback will be called every time the
    /// camera produces image data. The Update function will be blocked
    /// while the callbacks are executed.
    /// \remark Do not block inside of the callback.
    /// \return A connection pointer that must remain in scope. When the
    /// connection pointer falls out of scope, the connection is broken.
    public: gz::common::ConnectionPtr ConnectImageCallback(
                std::function<
                void(const gz::msgs::Image &)> _callback);

    /// \brief Set the rendering scene.
    /// \param[in] _scene Pointer to the scene
    public: virtual void SetScene(
                gz::rendering::ScenePtr _scene);

    /// \brief Get image width.
    /// \return width of the image
    public: virtual unsigned int ImageWidth() const;

    /// \brief Get image height.
    /// \return height of the image
    public: virtual unsigned int ImageHeight() const;

    /// \brief Get image width.
    /// \return width of the image
    public: virtual double FarClip() const;

    /// \brief Get image height.
    /// \return height of the image
    public: virtual double NearClip() const;

    // Documentation inherited.
    public: virtual bool HasConnections() const override;

    /// \brief Check if there are any depth subscribers
    /// \return True if there are subscribers, false otherwise
    public: virtual bool HasDepthConnections() const;

    /// \brief Check if there are any point subscribers
    /// \return True if there are subscribers, false otherwise
    public: virtual bool HasPointConnections() const;

    /// \brief Create a camera in a scene
    /// \return True on success.
    private: bool CreateCamera();

    /// \brief Callback that is triggered when the scene changes on
    /// the Manager.
    /// \param[in] _scene Pointer to the new scene.
    private: void OnSceneChange(gz::rendering::ScenePtr /*_scene*/)
            { }

    GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
    /// \brief Data pointer for private data
    /// \internal
    private: std::unique_ptr<MultibeamSonarSensorPrivate> dataPtr;
    GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
  };
  }
}

#endif