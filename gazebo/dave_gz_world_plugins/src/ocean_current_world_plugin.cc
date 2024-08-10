// Copyright (c) 2016 The UUV Simulator Authors.
// All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// \file ocean_current_world_plugin.cc

#include "dave_gz_world_plugins/ocean_current_world_plugin.hh"
#include <dave_gz_world_plugins_msgs/msgs/StratifiedCurrentVelocity.pb.h>

#include <gz/msgs/vector3d.pb.h>
#include <math.h>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <chrono>
#include <dave_gz_world_plugins/gauss_markov_process.hh>
#include <dave_gz_world_plugins/tidal_oscillation.hh>
#include <gz/math/Pose3.hh>
#include <gz/math/Vector3.hh>
#include <gz/math/Vector4.hh>
#include <gz/msgs/Utility.hh>
#include <gz/plugin/Register.hh>
#include <gz/sim/System.hh>
#include <gz/sim/World.hh>
#include <gz/transport/Node.hh>
#include <map>
#include <sdf/sdf.hh>
#include <string>
#include "gz/plugin/Register.hh"
#include "gz/sim/components/Model.hh"
#include "gz/sim/components/World.hh"
// #include <ament_index_cpp/get_package_share_directory.hpp> TODO (235-239)

// using namespace gz;
// using namespace sim;
// using namespace systems;
// using namespace dave_gz_world_plugins;

GZ_ADD_PLUGIN(
  dave_gz_world_plugins::UnderwaterCurrentPlugin, gz::sim::System,
  dave_gz_world_plugins::UnderwaterCurrentPlugin::ISystemConfigure,
  dave_gz_world_plugins::UnderwaterCurrentPlugin::ISystemPreUpdate,
  dave_gz_world_plugins::UnderwaterCurrentPlugin::ISystemPostUpdate)

namespace dave_gz_world_plugins
{
struct UnderwaterCurrentPlugin::PrivateData
{
  /// \brief Pointer to world
  gz::sim::World world{gz::sim::kNullEntity};

  /// \brief Pointer to sdf
  sdf::ElementPtr sdf;

  /// \brief True if the sea surface is present
  bool hasSurface;

  /// \brief Pointer to a node for communication
  std::shared_ptr<gz::transport::Node> gz_node;

  /// \brief Map of publishers
  // gz::transport::Node::Publisher publishers;
  std::map<std::string, gz::transport::Node::Publisher> publishers;

  /// \brief Vehicle Depth Subscriber
  gz::transport::Node subscriber;

  /// \brief Current velocity topic
  std::string currentVelocityTopic;

  /// \brief Stratified Ocean current topic
  std::string stratifiedCurrentVelocityTopic;

  /// \brief Vehicle depth topic
  std::string vehicleDepthTopic;

  /// \brief Ocean Current Database file path for csv file
  std::string databaseFilePath;

  /// \brief Tidal Oscillation Database file path for txt file
  std::string tidalFilePath;

  /// \brief Vector for read stratified current database values
  std::vector<gz::math::Vector3d> stratifiedDatabase;

  /// \brief Namespace for topics and services
  std::string ns;

  /// \brief Gauss-Markov process instance for the current velocity
  gz::GaussMarkovProcess currentVelModel;

  /// \brief Gauss-Markov process instance for horizontal angle model
  gz::GaussMarkovProcess currentHorzAngleModel;

  /// \brief Gauss-Markov process instance for vertical angle model
  gz::GaussMarkovProcess currentVertAngleModel;

  /// \brief Vector of Gauss-Markov process instances for stratified velocity
  std::vector<std::vector<gz::GaussMarkovProcess>> stratifiedCurrentModels;

  /// \brief Vector of dateGMT for tidal oscillation
  std::vector<std::array<int, 5>> dateGMT;

  /// \brief Vector of speedcmsec for tidal oscillation
  std::vector<double> speedcmsec;

  /// \brief Tidal current harmonic constituents
  bool tidalHarmonicFlag;

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
  int world_start_time_day;
  int world_start_time_month;
  int world_start_time_year;
  int world_start_time_hour;
  int world_start_time_minute;

  /// \brief Tidal Oscillation flag
  bool tideFlag;

  /// \brief Tidal Oscillation interpolation model
  gz::TidalOscillation tide;

  /// \brief Last update time stamp
  std::chrono::steady_clock::duration lastUpdate{0};

  /// \brief Current linear velocity vector
  gz::math::Vector3d currentVelocity;

  /// \brief Vector of current depth-specific linear velocity vectors
  std::vector<gz::math::Vector4d> currentStratifiedVelocity;

  /// \brief File path for stratified current database
  std::string db_path;

  std::shared_ptr<rclcpp::Node> rosNode;
};

/////////////////////////////////////////////////
UnderwaterCurrentPlugin::UnderwaterCurrentPlugin() : dataPtr(std::make_unique<PrivateData>()) {}

/////////////////////////////////////////////////
UnderwaterCurrentPlugin::~UnderwaterCurrentPlugin() { this->dataPtr->rosNode.reset(); }

/////////////////////////////////////////////////
void UnderwaterCurrentPlugin::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr)
{
  GZ_ASSERT(_entity != NULL, "World pointer is invalid");
  GZ_ASSERT(_sdf != NULL, "SDF pointer is invalid");

  this->dataPtr->world = gz::sim::World(_ecm.EntityByComponents(gz::sim::components::World()));
  if (!this->dataPtr->world.Valid(_ecm))  // check
  {
    gzerr << "World entity not found" << std::endl;
    return;
  }
  this->dataPtr->sdf = std::const_pointer_cast<sdf::Element>(_sdf);

  // Read the namespace for topics and services
  this->dataPtr->ns = _sdf->Get<std::string>("namespace");

  gzmsg << "Loading underwater world..." << std::endl;
  // Initializing the transport node
  this->dataPtr->gz_node = std::make_shared<gz::transport::Node>();

  // this->dataPtr->gz_node->Init(this->dataPtr->world.Name(_ecm));  // check if correct

  LoadGlobalCurrentConfig();
  LoadStratifiedCurrentDatabase();
  if (this->dataPtr->sdf->HasElement("tidal_oscillation"))
  {
    LoadTidalOscillationDatabase();
  }

  // Connect the update event. This isn't needed it seems (check)
  //   this->dataPtr->updateConnection = event::Events::ConnectWorldUpdateBegin(
  //     boost::bind(&UnderwaterCurrentPlugin::Update,
  //     this, _1));

  gzmsg << "Underwater current plugin loaded!" << std::endl
        << "\tWARNING: Current velocity calculated in the ENU frame" << std::endl;
}

/////////////////////////////////////////////////
void UnderwaterCurrentPlugin::LoadTidalOscillationDatabase()
{
  this->dataPtr->tideFlag = true;
  this->dataPtr->tidalHarmonicFlag = false;

  sdf::ElementPtr tidalOscillationParams =
    this->dataPtr->sdf->GetElement("tidal_oscillation");  // include this xml/ (TODO)
  sdf::ElementPtr tidalHarmonicParams;

  // Read the tidal oscillation parameter from the SDF file // (TO DO)(TO UNDERSTAND)
  if (tidalOscillationParams->HasElement("databasefilePath"))
  {
    this->dataPtr->tidalFilePath = tidalOscillationParams->Get<std::string>("databasefilePath");
    gzmsg << "Tidal current database configuration found" << std::endl;
  }
  else
  {
    if (tidalOscillationParams->HasElement("harmonic_constituents"))
    {
      tidalHarmonicParams = tidalOscillationParams->GetElement("harmonic_constituents");
      gzmsg << "Tidal harmonic constituents " << "configuration found" << std::endl;
      this->dataPtr->tidalHarmonicFlag = true;
    }
    // else
    // {
    //   this->dataPtr->tidalFilePath =
    //     ament_index_cpp::get_package_share_directory("dave_worlds") +
    //     "/worlds/ACT1951_predictionMaxSlack_2021-02-24.csv";
    // }  TODO
  }

  // Read the tidal oscillation direction from the SDF file
  GZ_ASSERT(
    tidalOscillationParams->HasElement("mean_direction"), "Tidal mean direction not defined");
  if (tidalOscillationParams->HasElement("mean_direction"))
  {
    sdf::ElementPtr elem = tidalOscillationParams->GetElement("mean_direction");
    GZ_ASSERT(elem->HasElement("ebb"), "Tidal mean ebb direction not defined");
    this->dataPtr->ebbDirection = elem->Get<double>("ebb");
    this->dataPtr->floodDirection = elem->Get<double>("flood");
    GZ_ASSERT(elem->HasElement("flood"), "Tidal mean flood direction not defined");
  }

  // Read the world start time (GMT) from the SDF file
  GZ_ASSERT(
    tidalOscillationParams->HasElement("world_start_time_GMT"),
    "World start time (GMT) not defined");
  if (tidalOscillationParams->HasElement("world_start_time_GMT"))
  {
    sdf::ElementPtr elem = tidalOscillationParams->GetElement("world_start_time_GMT");
    GZ_ASSERT(elem->HasElement("day"), "World start time (day) not defined");
    this->dataPtr->world_start_time_day = elem->Get<double>("day");
    GZ_ASSERT(elem->HasElement("month"), "World start time (month) not defined");
    this->dataPtr->world_start_time_month = elem->Get<double>("month");
    GZ_ASSERT(elem->HasElement("year"), "World start time (year) not defined");
    this->dataPtr->world_start_time_year = elem->Get<double>("year");
    GZ_ASSERT(elem->HasElement("hour"), "World start time (hour) not defined");
    this->dataPtr->world_start_time_hour = elem->Get<double>("hour");
    if (elem->HasElement("minute"))
    {
      this->dataPtr->world_start_time_minute = elem->Get<double>("minute");
    }
    else
    {
      this->dataPtr->world_start_time_minute = 0;
    }
  }

  if (this->dataPtr->tidalHarmonicFlag)
  {
    // Read harmonic constituents
    GZ_ASSERT(tidalHarmonicParams->HasElement("M2"), "Harcomnic constituents M2 not found");
    sdf::ElementPtr M2Params = tidalHarmonicParams->GetElement("M2");
    this->dataPtr->M2_amp = M2Params->Get<double>("amp");
    this->dataPtr->M2_phase = M2Params->Get<double>("phase");
    this->dataPtr->M2_speed = M2Params->Get<double>("speed");
    GZ_ASSERT(tidalHarmonicParams->HasElement("S2"), "Harcomnic constituents S2 not found");
    sdf::ElementPtr S2Params = tidalHarmonicParams->GetElement("S2");
    this->dataPtr->S2_amp = S2Params->Get<double>("amp");
    this->dataPtr->S2_phase = S2Params->Get<double>("phase");
    this->dataPtr->S2_speed = S2Params->Get<double>("speed");
    GZ_ASSERT(tidalHarmonicParams->HasElement("N2"), "Harcomnic constituents N2 not found");
    sdf::ElementPtr N2Params = tidalHarmonicParams->GetElement("N2");
    this->dataPtr->N2_amp = N2Params->Get<double>("amp");
    this->dataPtr->N2_phase = N2Params->Get<double>("phase");
    this->dataPtr->N2_speed = N2Params->Get<double>("speed");
    gzmsg << "Tidal harmonic constituents loaded : " << std::endl;
    gzmsg << "M2 amp: " << this->dataPtr->M2_amp << " phase: " << this->dataPtr->M2_phase
          << " speed: " << this->dataPtr->M2_speed << std::endl;
    gzmsg << "S2 amp: " << this->dataPtr->S2_amp << " phase: " << this->dataPtr->S2_phase
          << " speed: " << this->dataPtr->S2_speed << std::endl;
    gzmsg << "N2 amp: " << this->dataPtr->N2_amp << " phase: " << this->dataPtr->N2_phase
          << " speed: " << this->dataPtr->N2_speed << std::endl;
  }
  else
  {
    // Read database
    std::ifstream csvFile;
    std::string line;
    csvFile.open(this->dataPtr->tidalFilePath);
    if (!csvFile)
    {
      gz::common::SystemPaths * paths = new gz::common::SystemPaths();
      this->dataPtr->tidalFilePath = paths->FindFile(this->dataPtr->tidalFilePath, true);
      csvFile.open(this->dataPtr->tidalFilePath);
    }
    GZ_ASSERT(csvFile, "Tidal Oscillation database file does not exist");

    gzmsg << "Tidal Oscillation  Database loaded : " << this->dataPtr->tidalFilePath << std::endl;

    // skip the first line
    getline(csvFile, line);
    while (getline(csvFile, line))
    {
      if (line.empty())  // skip empty lines:
      {
        continue;
      }
      std::istringstream iss(line);
      std::string lineStream;
      std::string::size_type sz;
      std::vector<std::string> row;
      std::array<int, 5> tmpDateArray;
      while (getline(iss, lineStream, ','))
      {
        row.push_back(lineStream);
      }
      if (strcmp(row[1].c_str(), " slack"))  // skip 'slack' category
      {
        tmpDateArray[0] = std::stoi(row[0].substr(0, 4));
        tmpDateArray[1] = std::stoi(row[0].substr(5, 7));
        tmpDateArray[2] = std::stoi(row[0].substr(8, 10));
        tmpDateArray[3] = std::stoi(row[0].substr(11, 13));
        tmpDateArray[4] = std::stoi(row[0].substr(14, 16));
        this->dataPtr->dateGMT.push_back(tmpDateArray);

        this->dataPtr->speedcmsec.push_back(stold(row[2], &sz));
      }
    }
    csvFile.close();

    // Eliminate data with same consecutive type
    std::vector<int> duplicated;
    for (int i = 0; i < this->dataPtr->dateGMT.size() - 1; i++)
    {
      // delete latter if same sign
      if (
        ((this->dataPtr->speedcmsec[i] > 0) - (this->dataPtr->speedcmsec[i] < 0)) ==
        ((this->dataPtr->speedcmsec[i + 1] > 0) - (this->dataPtr->speedcmsec[i + 1] < 0)))
      {
        duplicated.push_back(i + 1);
      }
    }
    int eraseCount = 0;
    for (int i = 0; i < duplicated.size(); i++)
    {
      this->dataPtr->dateGMT.erase(this->dataPtr->dateGMT.begin() + duplicated[i] - eraseCount);
      this->dataPtr->speedcmsec.erase(
        this->dataPtr->speedcmsec.begin() + duplicated[i] - eraseCount);
      eraseCount++;
    }
  }
}

/////////////////////////////////////////////////
void UnderwaterCurrentPlugin::LoadStratifiedCurrentDatabase()
{
  GZ_ASSERT(
    this->dataPtr->sdf->HasElement("transient_current"),
    "Transient current configuration not available");
  sdf::ElementPtr transientCurrentParams = this->dataPtr->sdf->GetElement("transient_current");

  if (transientCurrentParams->HasElement("topic_stratified"))
  {
    this->dataPtr->stratifiedCurrentVelocityTopic =
      transientCurrentParams->Get<std::string>("topic_stratified");
  }
  else
  {
    this->dataPtr->stratifiedCurrentVelocityTopic = "stratified_current_velocity";
  }

  GZ_ASSERT(
    !this->dataPtr->stratifiedCurrentVelocityTopic.empty(),
    "Empty stratified ocean current velocity topic");

  // Read the depth dependent ocean current file path from the SDF file
  if (transientCurrentParams->HasElement("databasefilePath"))
  {
    this->dataPtr->databaseFilePath = transientCurrentParams->Get<std::string>("databasefilePath");
  }
  else
  {
    this->dataPtr->databaseFilePath = "transientOceanCurrentDatabase.csv";
  }

  GZ_ASSERT(
    !this->dataPtr->databaseFilePath.empty(), "Empty stratified ocean current database file path");

  gzmsg << this->dataPtr->databaseFilePath << std::endl;

  // #if GAZEBO_MAJOR_VERSION >= 8
  //   this->dataPtr->lastUpdate = this->dataPtr->world->SimTime();
  // #else
  //   this->dataPtr->lastUpdate = this->dataPtr->world->GetSimTime();
  // #endif

  // Read database
  std::ifstream csvFile;
  std::string line;
  csvFile.open(this->dataPtr->databaseFilePath);
  if (!csvFile)
  {
    gz::common::SystemPaths * paths = new gz::common::SystemPaths();
    this->dataPtr->databaseFilePath = paths->FindFile(this->dataPtr->databaseFilePath, true);
    csvFile.open(this->dataPtr->databaseFilePath);
  }
  GZ_ASSERT(csvFile, "Stratified Ocean database file does not exist");

  gzmsg << "Statified Ocean Current Database loaded : " << this->dataPtr->databaseFilePath
        << std::endl;

  // skip the 3 lines
  getline(csvFile, line);
  getline(csvFile, line);
  getline(csvFile, line);
  while (getline(csvFile, line))
  {
    if (line.empty())  // skip empty lines:
    {
      continue;
    }
    std::istringstream iss(line);
    std::string lineStream;
    std::string::size_type sz;
    std::vector<long double> row;
    while (getline(iss, lineStream, ','))
    {
      row.push_back(stold(lineStream, &sz));  // convert to double
    }
    gz::math::Vector3d read;
    read.X() = row[0];
    read.Y() = row[1];
    read.Z() = row[2];
    this->dataPtr->stratifiedDatabase.push_back(read);

    // Create Gauss-Markov processes for the stratified currents
    // Means are the database-specified magnitudes & angles, and
    // the other values come from the constant current models
    // TODO: Vertical angle currently set to 0 (not in database)
    gz::GaussMarkovProcess magnitudeModel;
    magnitudeModel.mean = hypot(row[1], row[0]);
    magnitudeModel.var = magnitudeModel.mean;
    magnitudeModel.max = this->dataPtr->currentVelModel.max;
    magnitudeModel.min = 0.0;
    magnitudeModel.mu = this->dataPtr->currentVelModel.mu;
    magnitudeModel.noiseAmp = this->dataPtr->currentVelModel.noiseAmp;
    // magnitudeModel.lastUpdate = this->dataPtr->lastUpdate;
    magnitudeModel.lastUpdate = std::chrono::duration<double>(this->dataPtr->lastUpdate).count();

    gz::GaussMarkovProcess hAngleModel;
    hAngleModel.mean = atan2(row[1], row[0]);
    hAngleModel.var = hAngleModel.mean;
    hAngleModel.max = M_PI;
    hAngleModel.min = -M_PI;
    hAngleModel.mu = this->dataPtr->currentHorzAngleModel.mu;
    hAngleModel.noiseAmp = this->dataPtr->currentHorzAngleModel.noiseAmp;
    hAngleModel.lastUpdate = std::chrono::duration<double>(this->dataPtr->lastUpdate).count();

    gz::GaussMarkovProcess vAngleModel;
    vAngleModel.mean = 0.0;
    vAngleModel.var = vAngleModel.mean;
    vAngleModel.max = M_PI / 2.0;
    vAngleModel.min = -M_PI / 2.0;
    vAngleModel.mu = this->dataPtr->currentVertAngleModel.mu;
    vAngleModel.noiseAmp = this->dataPtr->currentVertAngleModel.noiseAmp;
    vAngleModel.lastUpdate = std::chrono::duration<double>(this->dataPtr->lastUpdate).count();

    std::vector<gz::GaussMarkovProcess> depthModels;
    depthModels.push_back(magnitudeModel);
    depthModels.push_back(hAngleModel);
    depthModels.push_back(vAngleModel);
    this->dataPtr->stratifiedCurrentModels.push_back(depthModels);
  }
  csvFile.close();

  this->dataPtr->publishers[this->dataPtr->stratifiedCurrentVelocityTopic] =
    this->dataPtr->gz_node->Advertise<dave_gz_world_plugins_msgs::msgs::StratifiedCurrentVelocity>(
      this->dataPtr->ns + "/" + this->dataPtr->stratifiedCurrentVelocityTopic);

  gzmsg << "Stratified current velocity topic name: "
        << this->dataPtr->ns + "/" + this->dataPtr->stratifiedCurrentVelocityTopic << std::endl;
}

/////////////////////////////////////////////////
void UnderwaterCurrentPlugin::LoadGlobalCurrentConfig()
{
  // NOTE: The plugin currently requires stratified current, so the
  //       global current set up in this method is potentially
  //       inconsistent or redundant.
  //       Consider setting it up as one or the other, but not both?

  // Retrieve the velocity configuration, if existent
  GZ_ASSERT(
    this->dataPtr->sdf->HasElement("constant_current"), "Current configuration not available");
  sdf::ElementPtr currentVelocityParams = this->dataPtr->sdf->GetElement("constant_current");

  // Read the topic names from the SDF file
  if (currentVelocityParams->HasElement("topic"))
  {
    this->dataPtr->currentVelocityTopic = currentVelocityParams->Get<std::string>("topic");
  }
  else
  {
    this->dataPtr->currentVelocityTopic = "current_velocity";
  }

  GZ_ASSERT(!this->dataPtr->currentVelocityTopic.empty(), "Empty ocean current velocity topic");

  if (currentVelocityParams->HasElement("velocity"))
  {
    sdf::ElementPtr elem = currentVelocityParams->GetElement("velocity");
    if (elem->HasElement("mean"))
    {
      this->dataPtr->currentVelModel.mean = elem->Get<double>("mean");
    }
    if (elem->HasElement("min"))
    {
      this->dataPtr->currentVelModel.min = elem->Get<double>("min");
    }
    if (elem->HasElement("max"))
    {
      this->dataPtr->currentVelModel.max = elem->Get<double>("max");
    }
    if (elem->HasElement("mu"))
    {
      this->dataPtr->currentVelModel.mu = elem->Get<double>("mu");
    }
    if (elem->HasElement("noiseAmp"))
    {
      this->dataPtr->currentVelModel.noiseAmp = elem->Get<double>("noiseAmp");
    }

    GZ_ASSERT(
      this->dataPtr->currentVelModel.min < this->dataPtr->currentVelModel.max,
      "Invalid current velocity limits");
    GZ_ASSERT(
      this->dataPtr->currentVelModel.mean >= this->dataPtr->currentVelModel.min,
      "Mean velocity must be greater than minimum");
    GZ_ASSERT(
      this->dataPtr->currentVelModel.mean <= this->dataPtr->currentVelModel.max,
      "Mean velocity must be smaller than maximum");
    GZ_ASSERT(
      this->dataPtr->currentVelModel.mu >= 0 && this->dataPtr->currentVelModel.mu < 1,
      "Invalid process constant");
    GZ_ASSERT(
      this->dataPtr->currentVelModel.noiseAmp < 1 && this->dataPtr->currentVelModel.noiseAmp >= 0,
      "Noise amplitude has to be smaller than 1");
  }

  this->dataPtr->currentVelModel.var = this->dataPtr->currentVelModel.mean;
  gzmsg << "Current velocity [m/s] Gauss-Markov process model:" << std::endl;
  this->dataPtr->currentVelModel.Print();

  if (currentVelocityParams->HasElement("horizontal_angle"))
  {
    sdf::ElementPtr elem = currentVelocityParams->GetElement("horizontal_angle");

    if (elem->HasElement("mean"))
    {
      this->dataPtr->currentHorzAngleModel.mean = elem->Get<double>("mean");
    }
    if (elem->HasElement("min"))
    {
      this->dataPtr->currentHorzAngleModel.min = elem->Get<double>("min");
    }
    if (elem->HasElement("max"))
    {
      this->dataPtr->currentHorzAngleModel.max = elem->Get<double>("max");
    }
    if (elem->HasElement("mu"))
    {
      this->dataPtr->currentHorzAngleModel.mu = elem->Get<double>("mu");
    }
    if (elem->HasElement("noiseAmp"))
    {
      this->dataPtr->currentHorzAngleModel.noiseAmp = elem->Get<double>("noiseAmp");
    }

    GZ_ASSERT(
      this->dataPtr->currentHorzAngleModel.min < this->dataPtr->currentHorzAngleModel.max,
      "Invalid current horizontal angle limits");
    GZ_ASSERT(
      this->dataPtr->currentHorzAngleModel.mean >= this->dataPtr->currentHorzAngleModel.min,
      "Mean horizontal angle must be greater than minimum");
    GZ_ASSERT(
      this->dataPtr->currentHorzAngleModel.mean <= this->dataPtr->currentHorzAngleModel.max,
      "Mean horizontal angle must be smaller than maximum");
    GZ_ASSERT(
      this->dataPtr->currentHorzAngleModel.mu >= 0 && this->dataPtr->currentHorzAngleModel.mu < 1,
      "Invalid process constant");
    GZ_ASSERT(
      this->dataPtr->currentHorzAngleModel.noiseAmp < 1 &&
        this->dataPtr->currentHorzAngleModel.noiseAmp >= 0,
      "Noise amplitude for horizontal angle has to be between 0 and 1");
  }

  this->dataPtr->currentHorzAngleModel.var = this->dataPtr->currentHorzAngleModel.mean;
  gzmsg << "Current velocity horizontal angle [rad] Gauss-Markov process model:" << std::endl;
  this->dataPtr->currentHorzAngleModel.Print();

  if (currentVelocityParams->HasElement("vertical_angle"))
  {
    sdf::ElementPtr elem = currentVelocityParams->GetElement("vertical_angle");

    if (elem->HasElement("mean"))
    {
      this->dataPtr->currentVertAngleModel.mean = elem->Get<double>("mean");
    }
    if (elem->HasElement("min"))
    {
      this->dataPtr->currentVertAngleModel.min = elem->Get<double>("min");
    }
    if (elem->HasElement("max"))
    {
      this->dataPtr->currentVertAngleModel.max = elem->Get<double>("max");
    }
    if (elem->HasElement("mu"))
    {
      this->dataPtr->currentVertAngleModel.mu = elem->Get<double>("mu");
    }
    if (elem->HasElement("noiseAmp"))
    {
      this->dataPtr->currentVertAngleModel.noiseAmp = elem->Get<double>("noiseAmp");
    }

    GZ_ASSERT(
      this->dataPtr->currentVertAngleModel.min < this->dataPtr->currentVertAngleModel.max,
      "Invalid current vertical angle limits");
    GZ_ASSERT(
      this->dataPtr->currentVertAngleModel.mean >= this->dataPtr->currentVertAngleModel.min,
      "Mean vertical angle must be greater than minimum");
    GZ_ASSERT(
      this->dataPtr->currentVertAngleModel.mean <= this->dataPtr->currentVertAngleModel.max,
      "Mean vertical angle must be smaller than maximum");
    GZ_ASSERT(
      this->dataPtr->currentVertAngleModel.mu >= 0 && this->dataPtr->currentVertAngleModel.mu < 1,
      "Invalid process constant");
    GZ_ASSERT(
      this->dataPtr->currentVertAngleModel.noiseAmp < 1 &&
        this->dataPtr->currentVertAngleModel.noiseAmp >= 0,
      "Noise amplitude for vertical angle has to be between 0 and 1");
  }

  this->dataPtr->currentVertAngleModel.var = this->dataPtr->currentVertAngleModel.mean;
  gzmsg << "Current velocity vertical angle [rad] Gauss-Markov process model:" << std::endl;
  this->dataPtr->currentVertAngleModel.Print();

  this->dataPtr->currentVelModel.lastUpdate =
    std::chrono::duration<double>(this->dataPtr->lastUpdate).count();
  this->dataPtr->currentHorzAngleModel.lastUpdate =
    std::chrono::duration<double>(this->dataPtr->lastUpdate).count();
  this->dataPtr->currentVertAngleModel.lastUpdate =
    std::chrono::duration<double>(this->dataPtr->lastUpdate).count();

  // Advertise the current velocity & stratified current velocity topics
  this->dataPtr->publishers[this->dataPtr->currentVelocityTopic] =
    this->dataPtr->gz_node->Advertise<gz::msgs::Vector3d>(
      this->dataPtr->ns + "/" + this->dataPtr->currentVelocityTopic);
  gzmsg << "Current velocity topic name: "
        << this->dataPtr->ns + "/" + this->dataPtr->currentVelocityTopic << std::endl;
}

/////////////////////////////////////////////////
void UnderwaterCurrentPlugin::PublishCurrentVelocity()
{
  gz::msgs::Vector3d currentVel;
  gz::msgs::Set(
    &currentVel, gz::math::Vector3d(
                   this->dataPtr->currentVelocity.X(), this->dataPtr->currentVelocity.Y(),
                   this->dataPtr->currentVelocity.Z()));
  this->dataPtr->publishers[this->dataPtr->currentVelocityTopic].Publish(currentVel);
}

/////////////////////////////////////////////////
void UnderwaterCurrentPlugin::PublishStratifiedCurrentVelocity()
{
  dave_gz_world_plugins_msgs::msgs::StratifiedCurrentVelocity currentVel;  // check
  for (std::vector<gz::math::Vector4d>::iterator it =
         this->dataPtr->currentStratifiedVelocity.begin();
       it != this->dataPtr->currentStratifiedVelocity.end();
       ++it)  // currentStratifiedVelocity values defined where ? (TODO)
  {
    gz::msgs::Set(currentVel.add_velocity(), gz::math::Vector3d(it->X(), it->Y(), it->Z()));
    currentVel.add_depth(it->W());
  }
  if (currentVel.velocity_size() == 0)
  {
    return;
  }
  this->dataPtr->publishers[this->dataPtr->stratifiedCurrentVelocityTopic].Publish(currentVel);
}

/////////////////////////////////////////////////
void UnderwaterCurrentPlugin::PreUpdate(
  const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm)
{
}

/////////////////////////////////////////////////
void UnderwaterCurrentPlugin::Update(
  const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm)
{
  // Update the time
  auto time = std::chrono::duration<double>(_info.simTime).count();

  // Calculate the flow velocity and the direction using the Gauss-Markov
  // model

  // Update current velocity
  double currentVelMag = this->dataPtr->currentVelModel.Update(time);
  // Update current horizontal direction around z axis of flow frame
  double horzAngle = this->dataPtr->currentHorzAngleModel.Update(time);

  // Update current horizontal direction around z axis of flow frame
  double vertAngle = this->dataPtr->currentVertAngleModel.Update(time);

  // Generating the current velocity vector as in the North-East-Down frame
  this->dataPtr->currentVelocity = gz::math::Vector3d(
    currentVelMag * cos(horzAngle) * cos(vertAngle),
    currentVelMag * sin(horzAngle) * cos(vertAngle), currentVelMag * sin(vertAngle));

  // Generate the depth-specific velocities
  this->dataPtr->currentStratifiedVelocity.clear();
  for (int i = 0; i < this->dataPtr->stratifiedDatabase.size(); i++)
  {
    double depth = this->dataPtr->stratifiedDatabase[i].Z();
    currentVelMag = this->dataPtr->stratifiedCurrentModels[i][0].Update(time);
    horzAngle = this->dataPtr->stratifiedCurrentModels[i][1].Update(time);
    vertAngle = this->dataPtr->stratifiedCurrentModels[i][2].Update(time);
    gz::math::Vector4d depthVel(
      currentVelMag * cos(horzAngle) * cos(vertAngle),
      currentVelMag * sin(horzAngle) * cos(vertAngle), currentVelMag * sin(vertAngle), depth);
    this->dataPtr->currentStratifiedVelocity.push_back(depthVel);
  }
}

/////////////////////////////////////////////////
void UnderwaterCurrentPlugin::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  // Update time stamp
  this->dataPtr->lastUpdate = _info.simTime;
  PublishCurrentVelocity();
  PublishStratifiedCurrentVelocity();
}
}  // namespace dave_gz_world_plugins