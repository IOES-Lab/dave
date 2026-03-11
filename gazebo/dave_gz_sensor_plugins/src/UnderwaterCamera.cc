/*
 * Copyright (C) 2024 Rakesh Vivekanandan
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

#include "dave_gz_sensor_plugins/UnderwaterCamera.hh"
#include <gz/common/Console.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/Util.hh>
#include <gz/sim/components/Camera.hh>
#include <gz/sim/components/ParentEntity.hh>
#include <gz/sim/components/RgbdCamera.hh>
#include <gz/sim/components/Sensor.hh>
#include <gz/transport/Node.hh>
#include <sdf/Camera.hh>
#include "gz/plugin/Register.hh"
#include "gz/sim/EntityComponentManager.hh"
#include "gz/sim/components/Link.hh"
#include "gz/sim/components/Model.hh"
#include "gz/sim/components/Name.hh"
#include "gz/sim/components/World.hh"

GZ_ADD_PLUGIN(
  dave_gz_sensor_plugins::UnderwaterCamera, gz::sim::System,
  dave_gz_sensor_plugins::UnderwaterCamera::ISystemConfigure,
  dave_gz_sensor_plugins::UnderwaterCamera::ISystemPostUpdate)

namespace dave_gz_sensor_plugins
{

struct UnderwaterCamera::PrivateData
{
  // Add any private data members here.
  gz::sim::Model model;
  std::string linkName;

  std::mutex mutex_;
  gz::transport::Node gz_node;
  std::string topic;
  std::string image_topic;
  std::string depth_image_topic;
  std::string simulated_image_topic;
  std::string camera_info_topic;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub;

  /// \brief Width of the image.
  unsigned int width;

  /// \brief Height of the image.
  unsigned int height;

  /// \brief Camera intrinsics.
  double fx;
  double fy;
  double cx;
  double cy;

  /// \brief Temporarily store pointer to previous depth image.
  gz::msgs::Image lastDepth;

  /// \brief Latest simulated image.
  gz::msgs::Image lastImage;

  /// \brief Depth to range lookup table (LUT)
  float * depth2rangeLUT;

  /// \brief Attenuation constants per channel (RGB)
  float attenuation[3];

  /// \brief Background constants per channel (RGB)
  unsigned char background[3];

  bool firstImage = true;

  float min_range;
  float max_range;
};

UnderwaterCamera::UnderwaterCamera() : dataPtr(std::make_unique<PrivateData>()) {}

UnderwaterCamera::~UnderwaterCamera()
{
  if (this->dataPtr->depth2rangeLUT)
  {
    delete[] this->dataPtr->depth2rangeLUT;
  }
}

void UnderwaterCamera::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventManager)
{
  gzdbg << "dave_gz_sensor_plugins::UnderwaterCamera::Configure on entity: " << _entity
        << std::endl;

  auto rgbdCamera = _ecm.Component<gz::sim::components::RgbdCamera>(_entity);
  if (!rgbdCamera)
  {
    gzerr << "UnderwaterCamera plugin should be attached to a rgbd_camera sesnsor. "
          << "Failed to initialize." << std::endl;
    return;
  }

  // get world entity
  auto worldEntity = _ecm.EntityByComponents(gz::sim::components::World());
  auto worldName = _ecm.Component<gz::sim::components::Name>(worldEntity)->Data();

  auto sensorSdf = rgbdCamera->Data();
  this->dataPtr->topic = sensorSdf.Topic();

  if (sensorSdf.CameraSensor() == nullptr)
  {
    gzerr << "Attempting to a load an RGBD Camera sensor, but received "
          << "a null sensor." << std::endl;
    return;
  }

  if (!rclcpp::ok())
  {
    rclcpp::init(0, nullptr);
  }

  std::string rosNodeName = sensorSdf.Name() + "_node";
  this->ros_node_ = std::make_shared<rclcpp::Node>(rosNodeName);

  if (this->dataPtr->topic.empty())
  {
    auto scoped = gz::sim::scopedName(_entity, _ecm);
    this->dataPtr->topic = "/world/" + worldName + "/" + scoped;
  }

  this->dataPtr->image_topic = this->dataPtr->topic + "/image";
  this->dataPtr->depth_image_topic = this->dataPtr->topic + "/depth_image";
  this->dataPtr->simulated_image_topic = this->dataPtr->topic + "/simulated_image";

  sdf::Camera * cameraSdf = sensorSdf.CameraSensor();

  // get camera intrinsics
  this->dataPtr->width = cameraSdf->ImageWidth();
  this->dataPtr->height = cameraSdf->ImageHeight();

  if (this->dataPtr->width == 0 || this->dataPtr->height == 0)
  {
    gzerr << "Camera image size is zero" << std::endl;
    return;
  }

  this->dataPtr->min_range = cameraSdf->NearClip();
  this->dataPtr->max_range = cameraSdf->FarClip();

  // Check if min_range and max_range are valid
  if (this->dataPtr->min_range == 0 || this->dataPtr->max_range == 0)
  {
    gzerr << "Invalid min_range or max_range" << std::endl;
    return;
  }

  this->dataPtr->fx = cameraSdf->LensIntrinsicsFx();
  this->dataPtr->fy = cameraSdf->LensIntrinsicsFy();
  this->dataPtr->cx = cameraSdf->LensIntrinsicsCx();
  this->dataPtr->cy = cameraSdf->LensIntrinsicsCy();

  // Check if fx and fy are non-zero to avoid division by zero
  if (this->dataPtr->fx == 0 || this->dataPtr->fy == 0)
  {
    gzerr << "Camera intrinsics have zero focal length (fx or fy)" << std::endl;
    return;
  }

  // Check if cx, cy are zero. If so, set them to the center of the image
  if (this->dataPtr->cx == 0)
  {
    this->dataPtr->cx = this->dataPtr->width / 2;
  }
  if (this->dataPtr->cy == 0)
  {
    this->dataPtr->cy = this->dataPtr->height / 2;
  }

  gzmsg << "Camera intrinsics: fx=" << this->dataPtr->fx << ", fy=" << this->dataPtr->fy
        << ", cx=" << this->dataPtr->cx << ", cy=" << this->dataPtr->cy << std::endl;

  gzmsg << "Image size: width=" << this->dataPtr->width << ", height=" << this->dataPtr->height
        << std::endl;

  gzmsg << "Min range: " << this->dataPtr->min_range << ", Max range: " << this->dataPtr->max_range
        << std::endl;

  // Free previous LUT memory if it was already allocated
  if (this->dataPtr->depth2rangeLUT)
  {
    delete[] this->dataPtr->depth2rangeLUT;
  }

  // Allocate memory for the new LUT
  this->dataPtr->depth2rangeLUT = new float[this->dataPtr->width * this->dataPtr->height];
  float * lutPtr = this->dataPtr->depth2rangeLUT;

  // Fill depth2range LUT
  for (int v = 0; v < this->dataPtr->height; v++)
  {
    double y_z = (v - this->dataPtr->cy) / this->dataPtr->fy;
    for (int u = 0; u < this->dataPtr->width; u++)
    {
      double x_z = (u - this->dataPtr->cx) / this->dataPtr->fx;
      // Precompute the per-pixel factor in the following formula:
      // range = || (x, y, z) ||_2
      // range = || z * (x/z, y/z, 1.0) ||_2
      // range = z * || (x/z, y/z, 1.0) ||_2
      *(lutPtr++) = sqrt(1.0 + x_z * x_z + y_z * y_z);
    }
  }

  if (!_sdf->HasElement("attenuationR"))
  {
    this->dataPtr->attenuation[0] = 1.f / 30.f;
  }
  else
  {
    this->dataPtr->attenuation[0] = _sdf->Get<float>("attenuationR");
  }

  if (!_sdf->HasElement("attenuationG"))
  {
    this->dataPtr->attenuation[1] = 1.f / 30.f;
  }
  else
  {
    this->dataPtr->attenuation[1] = _sdf->Get<float>("attenuationG");
  }

  if (!_sdf->HasElement("attenuationB"))
  {
    this->dataPtr->attenuation[2] = 1.f / 30.f;
  }
  else
  {
    this->dataPtr->attenuation[2] = _sdf->Get<float>("attenuationB");
  }

  if (!_sdf->HasElement("backgroundR"))
  {
    this->dataPtr->background[0] = (unsigned char)0;
  }
  else
  {
    this->dataPtr->background[0] = (unsigned char)_sdf->Get<int>("backgroundR");
  }

  if (!_sdf->HasElement("backgroundG"))
  {
    this->dataPtr->background[1] = (unsigned char)0;
  }
  else
  {
    this->dataPtr->background[1] = (unsigned char)_sdf->Get<int>("backgroundG");
  }

  if (!_sdf->HasElement("backgroundB"))
  {
    this->dataPtr->background[2] = (unsigned char)0;
  }
  else
  {
    this->dataPtr->background[2] = (unsigned char)_sdf->Get<int>("backgroundB");
  }

  // Gazebo camera subscriber
  std::function<void(const gz::msgs::Image &)> camera_callback =
    std::bind(&UnderwaterCamera::CameraCallback, this, std::placeholders::_1);

  this->dataPtr->gz_node.Subscribe(this->dataPtr->image_topic, camera_callback);

  // Gazebo depth image subscriber
  std::function<void(const gz::msgs::Image &)> depth_callback =
    std::bind(&UnderwaterCamera::DepthImageCallback, this, std::placeholders::_1);

  this->dataPtr->gz_node.Subscribe(this->dataPtr->depth_image_topic, depth_callback);

  // ROS2 publisher
  this->dataPtr->image_pub = this->ros_node_->create_publisher<sensor_msgs::msg::Image>(
    this->dataPtr->simulated_image_topic, 1);
}

cv::Mat UnderwaterCamera::ConvertGazeboToOpenCV(const gz::msgs::Image & gz_image)
{
  int cv_type;
  switch (gz_image.pixel_format_type())
  {
    case gz::msgs::PixelFormatType::RGB_INT8:
      cv_type = CV_8UC3;
      break;
    case gz::msgs::PixelFormatType::RGBA_INT8:
      cv_type = CV_8UC4;
      break;
    case gz::msgs::PixelFormatType::BGR_INT8:
      cv_type = CV_8UC3;
      break;
    case gz::msgs::PixelFormatType::L_INT8:  // MONO8
      cv_type = CV_8UC1;
      break;
    case gz::msgs::PixelFormatType::R_FLOAT32:  // DEPTH32F
      cv_type = CV_32FC1;
      break;
    default:
      throw std::runtime_error("Unsupported pixel format");
  }

  // Create OpenCV Mat header that uses the same memory as the Gazebo image data
  cv::Mat cv_image(
    gz_image.height(), gz_image.width(), cv_type,
    const_cast<void *>(reinterpret_cast<const void *>(gz_image.data().data())));

  // Optionally convert color space if needed (e.g., RGB to BGR)
  if (gz_image.pixel_format_type() == gz::msgs::PixelFormatType::RGB_INT8)
  {
    cv::cvtColor(cv_image, cv_image, cv::COLOR_RGB2BGR);
  }
  else if (gz_image.pixel_format_type() == gz::msgs::PixelFormatType::RGBA_INT8)
  {
    cv::cvtColor(cv_image, cv_image, cv::COLOR_RGBA2BGRA);
  }

  return cv_image;
}

void UnderwaterCamera::CameraCallback(const gz::msgs::Image & msg)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->mutex_);

  if (!this->dataPtr->depth2rangeLUT)
  {
    gzerr << "Depth2range LUT not initialized" << std::endl;
    return;
  }
  else
  {
    gzmsg << "dave_gz_sensor_plugins::UnderwaterCamera::CameraCallback" << std::endl;

    if (this->dataPtr->firstImage)
    {
      this->dataPtr->lastImage = msg;
      this->dataPtr->firstImage = false;
    }
    else
    {
      // Convert Gazebo image to OpenCV image
      cv::Mat image = this->ConvertGazeboToOpenCV(msg);

      // Convert depth image to OpenCV image using the ConvertGazeboToOpenCV function
      cv::Mat depth_image = this->ConvertGazeboToOpenCV(this->dataPtr->lastDepth);

      // Create output image
      cv::Mat output_image = this->ConvertGazeboToOpenCV(this->dataPtr->lastImage);

      // Simulate underwater
      cv::Mat simulated_image = this->SimulateUnderwater(image, depth_image, output_image);

      // Publish simulated image
      sensor_msgs::msg::Image ros_image;
      ros_image.header.stamp = this->ros_node_->now();
      ros_image.height = msg.height();
      ros_image.width = msg.width();
      ros_image.encoding = "bgr8";
      ros_image.is_bigendian = false;
      ros_image.step = msg.width() * 3;
      ros_image.data = std::vector<unsigned char>(
        simulated_image.data,
        simulated_image.data + simulated_image.total() * simulated_image.elemSize());

      this->dataPtr->image_pub->publish(ros_image);

      // Store the current image for the next iteration
      this->dataPtr->lastImage = msg;
    }
  }
}

void UnderwaterCamera::DepthImageCallback(const gz::msgs::Image & msg)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->mutex_);

  gzmsg << "dave_gz_sensor_plugins::UnderwaterCamera::DepthImageCallback" << std::endl;

  this->dataPtr->lastDepth = msg;
}

cv::Mat UnderwaterCamera::SimulateUnderwater(
  const cv::Mat & _inputImage, const cv::Mat & _inputDepth, cv::Mat & _outputImage)
{
  const float * lutPtr = this->dataPtr->depth2rangeLUT;
  for (unsigned int row = 0; row < this->dataPtr->height; row++)
  {
    const cv::Vec3b * inrow = _inputImage.ptr<cv::Vec3b>(row);
    const float * depthrow = _inputDepth.ptr<float>(row);
    cv::Vec3b * outrow = _outputImage.ptr<cv::Vec3b>(row);

    for (int col = 0; col < this->dataPtr->width; col++)
    {
      // Convert depth to range using the depth2range LUT
      float r = *(lutPtr++) * depthrow[col];

      const cv::Vec3b & in = inrow[col];
      cv::Vec3b & out = outrow[col];

      if (r < this->dataPtr->min_range)
      {
        r = this->dataPtr->min_range;
      }
      else if (r > this->dataPtr->max_range)
      {
        r = this->dataPtr->max_range;
      }

      for (int c = 0; c < 3; c++)
      {
        // Simplifying assumption: intensity ~ irradiance.
        // This is not really the case but a good enough approximation
        // for now (it would be better to use a proper Radiometric
        // Response Function).
        float e = std::exp(-r * this->dataPtr->attenuation[c]);
        out[c] = e * in[c] + (1.0f - e) * this->dataPtr->background[c];
      }
    }
  }
  return _outputImage;
}

void UnderwaterCamera::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  if (!_info.paused)
  {
    rclcpp::spin_some(this->ros_node_);

    if (_info.iterations % 1000 == 0)
    {
      gzmsg << "dave_gz_sensor_plugins::UnderwaterCamera::PostUpdate" << std::endl;
    }
  }
}

}  // namespace dave_gz_sensor_plugins