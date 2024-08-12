#ifndef DAVE_GZ_WORLD_PLUGINS__TIDAL_OSCILLATION_HH_
#define DAVE_GZ_WORLD_PLUGINS__TIDAL_OSCILLATION_HH_

#include <cstdlib>
#include <ctime>
#include <gz/sim/System.hh>
#include <random>
#include <string>
#include <utility>
#include <vector>

// #include <boost/math/interpolators/barycentric_rational.hpp>

namespace dave_gz_world_plugins
{
/// \brief Interpolation of NOAA data for Tidal Oscillation feature
class TidalOscillation
{
  /// \brief Class constructor
public:
  TidalOscillation();

  /// \brief Resets the process parameters
public:
  void Reset();

  /// \brief Prepare the data for interpolation
public:
  void Initiate(bool _harmonicConstituents);

  /// \brief Translate datetime string to datenum
public:
  double TranslateDate(std::array<int, 5> _datetime);

  /// \brief Input Datenum data
public:
  std::vector<std::array<int, 5>> dateGMT;

  /// \brief Input Tidal data
public:
  std::vector<double> speedcmsec;

  /// \brief Input Datenum data
public:
  std::vector<double> datenum;

  /// \brief Bool for method type
public:
  bool harmonicConstituent;

  /// \brief Tidal current harmonic constituents
public:
  double M2_amp;

public:
  double M2_phase;

public:
  double M2_speed;

public:
  double S2_amp;

public:
  double S2_phase;

public:
  double S2_speed;

public:
  double N2_amp;

public:
  double N2_phase;

public:
  double N2_speed;

  /// \brief Input Tidal direction
public:
  double ebbDirection;

public:
  double floodDirection;

  /// \brief Input world start time
public:
  std::array<int, 5> worldStartTime;

public:
  double worldStartTime_num;

  /// \brief Update function for a new time stamp
  /// \param _time Current time stamp
public:
  std::pair<double, double> Update(double _time, double _currentDepthRatio);

  /// \brief save current state (Flood: true, Ebb: false)
public:
  bool currentType;
};
}  // namespace dave_gz_world_plugins

#endif  // DAVE_GZ_WORLD_PLUGINS__TIDAL_OSCILLATION_HH_S
