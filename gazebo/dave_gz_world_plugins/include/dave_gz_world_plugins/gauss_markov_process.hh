#ifndef DAVE_GZ_WORLD_PLUGINS__GAUSS_MARKOV_PROCESS_HH_
#define DAVE_GZ_WORLD_PLUGINS__GAUSS_MARKOV_PROCESS_HH_

#include <cstdlib>
#include <ctime>
#include <gz/plugin/Register.hh>
#include <gz/sim/System.hh>
#include <memory>
#include <random>
#include <rclcpp/rclcpp.hpp>

namespace dave_gz_world_plugins
{
/// \brief Implementation of a Gauss-Markov process to model the current
/// velocity and direction according to [1]
/// [1] Fossen, Thor I. Handbook of marine craft hydrodynamics and motion
/// control. John Wiley & Sons, 2011.
class GaussMarkovProcess
{
  /// \brief Class constructor
public:
  GaussMarkovProcess();

  /// \brief Resets the process parameters
public:
  void Reset();
  /// \brief Sets all the necessary parameters for the computation
  /// \param _mean Mean value
  /// \param _min Minimum limit
  /// \param _max Maximum limit
  /// \param _mu Process constant
  /// \param _noise Amplitude for the Gaussian white noise
  /// \return True, if all parameters were valid
public:
  bool SetModel(double _mean, double _min, double _max, double _mu = 0, double _noise = 0);

  /// \brief Set mean process value
  /// \param _mean New mean value
  /// \return True, if value inside the limit range
public:
  bool SetMean(double _mean);

  /// \brief Process variable
public:
  double var;

  /// \brief Mean process value
public:
  double mean;

  /// \brief Minimum limit for the process variable
public:
  double min;

  /// \brief Maximum limit for the process variable
public:
  double max;

  /// \brief Process constant, if zero, process becomes a random walk
public:
  double mu;

  /// \brief Gaussian white noise amplitude
public:
  double noiseAmp;

  /// \brief Timestamp for the last update
public:
  double lastUpdate;

  /// \brief State storage for the random number generator
private:
  unsigned int randSeed;

  /// \brief Update function for a new time stamp
  /// \param _time Current time stamp
public:
  double Update(double _time);

  /// \brief Print current model parameters
public:
  void Print();
};
}  // namespace dave_gz_world_plugins

#endif  // DAVE_GZ_WORLD_PLUGINS__GAUSS_MARKOV_PROCESS_HH_
