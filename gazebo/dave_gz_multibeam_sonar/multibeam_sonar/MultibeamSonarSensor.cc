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

#include <gz/msgs/image.pb.h>
#include <gz/msgs/pointcloud_packed.pb.h>

#include <mutex>

#include <gz/common/Console.hh>
#include <gz/common/Image.hh>
#include <gz/common/Profiler.hh>
#include <gz/common/SystemPaths.hh>

#include <gz/math/Angle.hh>
#include <gz/math/Helpers.hh>

#include <gz/msgs/Utility.hh>
#include <gz/rendering/Utils.hh>
#include <gz/transport/Node.hh>

#include <gz/sensors/DepthCameraSensor.hh>
#include <gz/sensors/Manager.hh>
#include <gz/sensors/SensorFactory.hh>
#include <gz/sensors/ImageGaussianNoiseModel.hh>
#include <gz/sensors/ImageNoise.hh>
#include <gz/sensors/RenderingEvents.hh>
#include <gz/sensors/Noise.hh>
#include <gz/sensors/Util.hh>

#include "MultibeamSonarSensor.hh"

// undefine near and far macros from windows.h
#ifdef _WIN32
  #undef near
  #undef far
#endif

using namespace custom;

//////////////////////////////////////////////////
bool MultibeamSonarSensorPrivate::ConvertDepthToImage(
    const float *_data,
    unsigned char *_imageBuffer,
    unsigned int _width, unsigned int _height)
{
  float maxDepth = 0;
  for (unsigned int i = 0; i < _height * _width; ++i)
  {
    if (_data[i] > maxDepth && !std::isinf(_data[i]))
    {
      maxDepth = _data[i];
    }
  }
  double factor = 255 / maxDepth;
  for (unsigned int j = 0; j < _height * _width; ++j)
  {
    unsigned char d = static_cast<unsigned char>(255 - (_data[j] * factor));
    _imageBuffer[j * 3] = d;
    _imageBuffer[j * 3 + 1] = d;
    _imageBuffer[j * 3 + 2] = d;
  }
  return true;
}

//////////////////////////////////////////////////
bool MultibeamSonarSensorPrivate::SaveImage(const float *_data,
    unsigned int _width, unsigned int _height,
    gz::common::Image::PixelFormatType /*_format*/)
{
  // Attempt to create the directory if it doesn't exist
  if (!gz::common::isDirectory(this->saveImagePath))
  {
    if (!gz::common::createDirectories(this->saveImagePath))
      return false;
  }

  if (_width == 0 || _height == 0)
    return false;

  gz::common::Image localImage;

  unsigned int depthSamples = _width * _height;
  unsigned int depthBufferSize = depthSamples * 3;

  unsigned char * imgDepthBuffer = new unsigned char[depthBufferSize];

  this->ConvertDepthToImage(_data, imgDepthBuffer, _width, _height);

  std::string filename = this->saveImagePrefix +
                         std::to_string(this->saveImageCounter) + ".png";
  ++this->saveImageCounter;

  localImage.SetFromData(imgDepthBuffer, _width, _height,
      gz::common::Image::RGB_INT8);
  localImage.SavePNG(
      gz::common::joinPaths(this->saveImagePath, filename));

  delete[] imgDepthBuffer;
  return true;
}

//////////////////////////////////////////////////
MultibeamSonarSensor::MultibeamSonarSensor()
  : gz::sensors::CameraSensor(), dataPtr(new MultibeamSonarSensorPrivate())
{
}

//////////////////////////////////////////////////
MultibeamSonarSensor::~MultibeamSonarSensor()
{
  this->dataPtr->depthConnection.reset();
  this->dataPtr->pointCloudConnection.reset();
  if (this->dataPtr->depthBuffer)
    delete [] this->dataPtr->depthBuffer;
  if (this->dataPtr->pointCloudBuffer)
    delete [] this->dataPtr->pointCloudBuffer;
  if (this->dataPtr->xyzBuffer)
    delete [] this->dataPtr->xyzBuffer;
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Init()
{
  return this->CameraSensor::Init();
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Load(sdf::ElementPtr _sdf)
{
  sdf::Sensor sdfSensor;
  sdfSensor.Load(_sdf);
  return this->Load(sdfSensor);
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Load(const sdf::Sensor &_sdf)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->mutex);

  if (!Sensor::Load(_sdf))
  {
    return false;
  }

  if (_sdf.CameraSensor() == nullptr)
  {
    gzerr << "_sdf.CameraSensor() == nullptr" << std::endl;
  }

  // Check if this is the right type
  if (_sdf.Type() != sdf::SensorType::CUSTOM)
  {
    gzerr << "Attempting to a load a custom sensor, but received "
      << "a " << _sdf.TypeStr() << std::endl;
  }

  auto type = gz::sensors::customType(_sdf);
  if ("multibeam_sonar" != type)
  {
    gzerr << "Trying to load [multibeam_sonar] sensor, but got type ["
           << type << "] instead." << std::endl;
    return false;
  }

  // Additional debug information
  sdf::ElementPtr cameraSensorSDFElementPtr = _sdf.Element();
  sdf::Sensor depthCameraSDF;
  if (!_sdf.Element()->HasElement("camera"))
  {
    gzerr << "The <camera> element is not found in the SDF." << std::endl;
  }
  else
  {
    gzerr << "The <camera> element is found in the SDF." << std::endl;
    // Create an sdf::Camera object and populate it with the camera element
    sdf::ElementPtr cameraElem = _sdf.Element()->GetElement("camera");
    // Ensure the camera element is valid before loading
    if (cameraElem == nullptr)
    {
        gzerr << "Camera element is null." << std::endl;
        return false;
    }

    // Now create a CameraSensor using the sdf::Camera object

    gzerr << "---------------------1-------------------------" << std::endl;
    // auto cameraSensor = std::make_shared<gz::sensors::Sensor>();
    sdf::ElementPtr cameraSensorSDFElementPtr = _sdf.Element();
    gzerr << "----------------------2------------------------" << std::endl;
    cameraSensorSDFElementPtr->GetAttribute("type")->SetFromString("depth_camera");
    // auto depthCamera = std::dynamic_pointer_cast<gz::sensors::Sensor>(cameraSensorSDFElementPtr);
    // Initialize the sdf::Sensor object
    sdf::Errors errors = depthCameraSDF.Load(cameraSensorSDFElementPtr);
    if (!errors.empty())
    {
        for (const auto &err : errors)
        {
            gzerr << "Error: " << err.Message() << std::endl;
        }
        gzerr << "Failed to load CameraSensor." << std::endl;
        return false;
    }
    gzerr << "----------------------3------------------------" << std::endl;

    if (depthCameraSDF.CameraSensor() == nullptr)
    {
      gzerr << "depthCameraSDF.CameraSensor() == nullptr" << std::endl;
    }

    // cameraSensor->GetAttribute("type")->SetFromString("depth_camera");
    // auto depthCamera = std::dynamic_pointer_cast<gz::sensors::CameraSensor>(cameraSensor);
    gzerr << "----------------------4------------------------" << std::endl;
  }

  gzerr << "----------------------4------------------------" << std::endl;
  
  this->dataPtr->sdfSensor = depthCameraSDF;

  if (this->Topic().empty())
    this->SetTopic("/camera/depth");

  this->dataPtr->pub =
      this->dataPtr->node.Advertise<gz::msgs::Image>(
          this->Topic());
  if (!this->dataPtr->pub)
  {
    gzerr << "Unable to create publisher on topic["
      << this->Topic() << "].\n";
    return false;
  }

  gzdbg << "Depth images for [" << this->Name() << "] advertised on ["
         << this->Topic() << "]" << std::endl;

    gzerr << "----------------------5------------------------" << std::endl;
  if (depthCameraSDF.CameraSensor()->Triggered())
  {
    std::string triggerTopic = depthCameraSDF.CameraSensor()->TriggerTopic();
    if (triggerTopic.empty())
    {
      triggerTopic = gz::transport::TopicUtils::AsValidTopic(this->Topic() +
                                                         "/trigger");
    }
    this->SetTriggered(true, triggerTopic);
  }

  gzerr << "----------------------6------------------------" << std::endl;
  if (!this->AdvertiseInfo())
    return false;

  // Create the point cloud publisher
  this->dataPtr->pointPub =
      this->dataPtr->node.Advertise<gz::msgs::PointCloudPacked>(
          this->Topic() + "/points");
  if (!this->dataPtr->pointPub)
  {
    gzerr << "Unable to create publisher on topic["
      << this->Topic() + "/points" << "].\n";
    return false;
  }

  gzerr << "----------------------7------------------------" << std::endl;
  gzdbg << "Points for [" << this->Name() << "] advertised on ["
         << this->Topic() << "/points]" << std::endl;


  gzerr << this->Scene() << std::endl;
  gzerr << this->RenderingCamera() << std::endl;
  gzerr << gz::sensors::RenderingSensor::Scene() << std::endl;


  if (this->Scene())
  // if (this->dataPtr->scene)
  {
    gzerr << "----------------------77777777------------------------" << std::endl;
    this->CreateCamera();
  }

    gzerr << "----------------------8------------------------" << std::endl;
  this->dataPtr->sceneChangeConnection =
      gz::sensors::RenderingEvents::ConnectSceneChangeCallback(
      std::bind(&MultibeamSonarSensor::SetScene, this, std::placeholders::_1));

  this->dataPtr->initialized = true;
    gzerr << "----------------------9------------------------" << std::endl;

  return true;
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::CreateCamera()
{
    gzerr << "----------------------7---------1---------------" << std::endl;
  sdf::Camera *cameraSdf = this->dataPtr->sdfSensor.CameraSensor();

    gzerr << "----------------------7---------2---------------" << std::endl;
  if (!cameraSdf)
  {
    gzerr << "Unable to access camera SDF element\n";
    return false;
  }

    gzerr << "----------------------7---------3---------------" << std::endl;
  int width = cameraSdf->ImageWidth();
  int height = cameraSdf->ImageHeight();

  double far = cameraSdf->FarClip();
  double near = cameraSdf->NearClip();

    gzerr << "----------------------7-----------4-------------" << std::endl;
  this->dataPtr->depthCamera = this->Scene()->CreateDepthCamera(this->Name());
  this->dataPtr->depthCamera->SetImageWidth(width);
  this->dataPtr->depthCamera->SetImageHeight(height);
  this->dataPtr->depthCamera->SetNearClipPlane(near);
  this->dataPtr->depthCamera->SetFarClipPlane(far);
  this->dataPtr->depthCamera->SetVisibilityMask(
      cameraSdf->VisibilityMask());
  this->dataPtr->depthCamera->SetLocalPose(this->Pose());
  this->AddSensor(this->dataPtr->depthCamera);
    gzerr << "----------------------7--------5----------------" << std::endl;

  const std::map<gz::sensors::SensorNoiseType, sdf::Noise> noises = {
    {gz::sensors::CAMERA_NOISE, cameraSdf->ImageNoise()},
  };
    gzerr << "----------------------7---------6---------------" << std::endl;

  for (const auto & [noiseType, noiseSdf] : noises)
  {
    // Add gaussian noise to camera sensor
    if (noiseSdf.Type() == sdf::NoiseType::GAUSSIAN)
    {
      // Skip applying noise if mean and stddev are 0 - this avoids
      // doing an extra render pass in gz-rendering
      // Note ImageGaussianNoiseModel only uses mean and stddev and does not
      // use bias parameters.
      if (!gz::math::equal(noiseSdf.Mean(), 0.0) ||
          !gz::math::equal(noiseSdf.StdDev(), 0.0))
      {
        this->dataPtr->noises[noiseType] =
          gz::sensors::ImageNoiseFactory::NewNoiseModel(noiseSdf, "depth");

        std::dynamic_pointer_cast<gz::sensors::ImageGaussianNoiseModel>(
             this->dataPtr->noises[noiseType])->SetCamera(
               this->dataPtr->depthCamera);
      }
    }
    else if (noiseSdf.Type() != sdf::NoiseType::NONE)
    {
      gzwarn << "The depth camera sensor only supports Gaussian noise. "
       << "The supplied noise type[" << static_cast<int>(noiseSdf.Type())
       << "] is not supported." << std::endl;
    }
  }

    gzerr << "----------------------7----------7--------------" << std::endl;
  // Near clip plane not set because we need to be able to detect occlusion
  // from objects before near clip plane
  this->dataPtr->near = near;

  // \todo(nkoeng) these parameters via sdf
  this->dataPtr->depthCamera->SetAntiAliasing(2);

  gz::math::Angle angle = cameraSdf->HorizontalFov();
  if (angle < 0.01 || angle > GZ_PI*2)
  {
    gzerr << "Invalid horizontal field of view [" << angle << "]\n";

    return false;
  }
  this->dataPtr->depthCamera->SetAspectRatio(static_cast<double>(width)/height);
  this->dataPtr->depthCamera->SetHFOV(angle);

  // Create depth texture when the camera is reconfigured from default values
  this->dataPtr->depthCamera->CreateDepthTexture();

  // \todo(nkoenig) Port Distortion class
  // This->dataPtr->distortion.reset(new Distortion());
  // This->dataPtr->distortion->Load(this->sdf->GetElement("distortion"));

  this->Scene()->RootVisual()->AddChild(this->dataPtr->depthCamera);

  this->UpdateLensIntrinsicsAndProjection(this->dataPtr->depthCamera,
      *cameraSdf);

  // Create the directory to store frames
  if (cameraSdf->SaveFrames())
  {
    this->dataPtr->saveImagePath = cameraSdf->SaveFramesPath();
    this->dataPtr->saveImagePrefix = this->Name() + "_";
    this->dataPtr->saveImage = true;
  }
    gzerr << "----------------------7----------8--------------" << std::endl;

  this->PopulateInfo(cameraSdf);

  this->dataPtr->depthConnection =
      this->dataPtr->depthCamera->ConnectNewDepthFrame(
      std::bind(&MultibeamSonarSensor::OnNewDepthFrame, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
        std::placeholders::_4, std::placeholders::_5));

  // Initialize the point message.
  // \todo(anyone) The true value in the following function call forces
  // the xyz and rgb fields to be aligned to memory boundaries. This is need
  // by ROS1: https://github.com/ros/common_msgs/pull/77. Ideally, memory
  // alignment should be configured.
  gz::msgs::InitPointCloudPacked(
        this->dataPtr->pointMsg,
        this->OpticalFrameId(),
        true,
        {{"xyz", gz::msgs::PointCloudPacked::Field::FLOAT32},
         {"rgb", gz::msgs::PointCloudPacked::Field::FLOAT32}});

  // Set the values of the point message based on the camera information.
  this->dataPtr->pointMsg.set_width(this->ImageWidth());
  this->dataPtr->pointMsg.set_height(this->ImageHeight());
  this->dataPtr->pointMsg.set_row_step(
      this->dataPtr->pointMsg.point_step() * this->ImageWidth());

  return true;
}

/////////////////////////////////////////////////
void MultibeamSonarSensor::OnNewDepthFrame(const float *_scan,
                    unsigned int _width, unsigned int _height,
                    unsigned int /*_channels*/,
                    const std::string &_format)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->mutex);

  unsigned int depthSamples = _width * _height;
  unsigned int depthBufferSize = depthSamples * sizeof(float);

  gz::common::Image::PixelFormatType format =
    gz::common::Image::ConvertPixelFormat(_format);

  if (!this->dataPtr->depthBuffer)
    this->dataPtr->depthBuffer = new float[depthSamples];

  memcpy(this->dataPtr->depthBuffer, _scan, depthBufferSize);

  // Save image
  if (this->dataPtr->saveImage)
  {
    this->dataPtr->SaveImage(_scan, _width, _height,
        format);
  }
}

/////////////////////////////////////////////////
void MultibeamSonarSensor::OnNewRgbPointCloud(const float *_scan,
                    unsigned int _width, unsigned int _height,
                    unsigned int _channels,
                    const std::string &/*_format*/)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->mutex);

  unsigned int pointCloudSamples = _width * _height;
  unsigned int pointCloudBufferSize = pointCloudSamples * _channels *
      sizeof(float);

  if (!this->dataPtr->pointCloudBuffer)
    this->dataPtr->pointCloudBuffer = new float[pointCloudSamples * _channels];

  memcpy(this->dataPtr->pointCloudBuffer, _scan, pointCloudBufferSize);
}

/////////////////////////////////////////////////
gz::rendering::DepthCameraPtr MultibeamSonarSensor::DepthCamera() const
{
  return this->dataPtr->depthCamera;
}

/////////////////////////////////////////////////
gz::common::ConnectionPtr MultibeamSonarSensor::ConnectImageCallback(
    std::function<void(const gz::msgs::Image &)> _callback)
{
  return this->dataPtr->imageEvent.Connect(_callback);
}

/////////////////////////////////////////////////
void MultibeamSonarSensor::SetScene(gz::rendering::ScenePtr _scene)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->mutex);
  // APIs make it possible for the scene pointer to change
  if (this->Scene() != _scene)
  {
    // TODO(anyone) Remove camera from scene
    this->dataPtr->depthCamera = nullptr;
    gz::sensors::RenderingSensor::SetScene(_scene);

    if (this->dataPtr->initialized)
      this->CreateCamera();
  }
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Update(
  const std::chrono::steady_clock::duration &_now)
{
  GZ_PROFILE("MultibeamSonarSensor::Update");
  if (!this->dataPtr->initialized)
  {
    gzerr << "Not initialized, update ignored.\n";
    return false;
  }

  // if (!this->dataPtr->depthCamera)
  // {
  //   gzerr << "Camera doesn't exist.\n";
  //   return false;
  // }

  if (this->HasInfoConnections())
  {
    // publish the camera info message
    this->PublishInfo(_now);
  }

  if (!this->HasDepthConnections() && !this->HasPointConnections())
  {
    return false;
  }

  if (this->HasPointConnections() && !this->dataPtr->pointCloudConnection)
  {
    this->dataPtr->pointCloudConnection =
        this->dataPtr->depthCamera->ConnectNewRgbPointCloud(
        std::bind(&MultibeamSonarSensor::OnNewRgbPointCloud, this,
          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
          std::placeholders::_4, std::placeholders::_5));
  }
  else if (!this->HasPointConnections() && this->dataPtr->pointCloudConnection)
  {
    this->dataPtr->pointCloudConnection.reset();
  }

  // generate sensor data
  this->Render();

  unsigned int width = this->dataPtr->depthCamera->ImageWidth();
  unsigned int height = this->dataPtr->depthCamera->ImageHeight();

  auto msgsFormat = gz::msgs::PixelFormatType::R_FLOAT32;

  // create message
  gz::msgs::Image msg;
  msg.set_width(width);
  msg.set_height(height);
  msg.set_step(width * gz::rendering::PixelUtil::BytesPerPixel(
               gz::rendering::PF_FLOAT32_R));
  msg.set_pixel_format_type(msgsFormat);
  *msg.mutable_header()->mutable_stamp() = gz::msgs::Convert(_now);

  auto* frame = msg.mutable_header()->add_data();
  frame->set_key("frame_id");
  frame->add_value(this->OpticalFrameId());

  std::lock_guard<std::mutex> lock(this->dataPtr->mutex);
  msg.set_data(this->dataPtr->depthBuffer,
      gz::rendering::PixelUtil::MemorySize(gz::rendering::PF_FLOAT32_R,
      width, height));

  this->AddSequence(msg.mutable_header(), "default");
  this->dataPtr->pub.Publish(msg);


  if (this->dataPtr->imageEvent.ConnectionCount() > 0u)
  {
    // Trigger callbacks.
    try
    {
      this->dataPtr->imageEvent(msg);
    }
    catch(...)
    {
      gzerr << "Exception thrown in an image callback.\n";
    }
  }

  if (this->HasPointConnections() &&
      this->dataPtr->pointCloudBuffer)
  {
    // Set the time stamp
    *this->dataPtr->pointMsg.mutable_header()->mutable_stamp() =
      gz::msgs::Convert(_now);
    this->dataPtr->pointMsg.set_is_dense(true);

    if (!this->dataPtr->xyzBuffer)
      this->dataPtr->xyzBuffer = new float[width*height*3];

    if (this->dataPtr->image.Width() != width
        || this->dataPtr->image.Height() != height)
    {
      this->dataPtr->image =
          gz::rendering::Image(width, height, gz::rendering::PF_R8G8B8);
    }

    // extract image data from point cloud data
    this->dataPtr->pointsUtil.XYZFromPointCloud(
        this->dataPtr->xyzBuffer,
        this->dataPtr->pointCloudBuffer,
        width, height);

    // convert depth to grayscale rgb image
    this->dataPtr->ConvertDepthToImage(this->dataPtr->depthBuffer,
        this->dataPtr->image.Data<unsigned char>(), width, height);

    // fill the point cloud msg with data from xyz and rgb buffer
    this->dataPtr->pointsUtil.FillMsg(this->dataPtr->pointMsg,
        this->dataPtr->xyzBuffer,
        this->dataPtr->image.Data<unsigned char>());

    this->AddSequence(this->dataPtr->pointMsg.mutable_header(), "pointMsg");
    this->dataPtr->pointPub.Publish(this->dataPtr->pointMsg);
  }
  return true;
}


//////////////////////////////////////////////////
unsigned int MultibeamSonarSensor::ImageWidth() const
{
  return this->dataPtr->depthCamera->ImageWidth();
}

//////////////////////////////////////////////////
unsigned int MultibeamSonarSensor::ImageHeight() const
{
  return this->dataPtr->depthCamera->ImageHeight();
}

//////////////////////////////////////////////////
double MultibeamSonarSensor::FarClip() const
{
  return this->dataPtr->depthCamera->FarClipPlane();
}

//////////////////////////////////////////////////
double MultibeamSonarSensor::NearClip() const
{
  return this->dataPtr->near;
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::HasConnections() const
{
  return this->HasDepthConnections() || this->HasPointConnections() ||
      this->HasInfoConnections();
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::HasDepthConnections() const
{
  return (this->dataPtr->pub && this->dataPtr->pub.HasConnections())
         || this->dataPtr->imageEvent.ConnectionCount() > 0u;
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::HasPointConnections() const
{
  return this->dataPtr->pointPub && this->dataPtr->pointPub.HasConnections();
}