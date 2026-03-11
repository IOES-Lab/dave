#ifndef GZ_SENSORS_AXISALIGNEDPATCH2_HH_
#define GZ_SENSORS_AXISALIGNEDPATCH2_HH_

#include <algorithm>
#include <gz/math/Vector2.hh>

namespace gz::sensors
{

/// \brief Axis-aligned patch on a plane, using image frame conventions.
template <typename T>
class AxisAlignedPatch2
{
public:
  AxisAlignedPatch2() = default;

  AxisAlignedPatch2(
    const gz::math::Vector2<T> & _topLeft, const gz::math::Vector2<T> & _bottomRight)
  : topLeft(_topLeft), bottomRight(_bottomRight)
  {
  }

  /// Scalar converting copy constructor
  template <typename U>
  AxisAlignedPatch2(const AxisAlignedPatch2<U> & _other)
  {
    this->topLeft.X(static_cast<T>(_other.XMax()));
    this->topLeft.Y(static_cast<T>(_other.YMax()));
    this->bottomRight.X(static_cast<T>(_other.XMin()));
    this->bottomRight.Y(static_cast<T>(_other.YMin()));
  }

  T XMax() const { return this->topLeft.X(); }
  T XMin() const { return this->bottomRight.X(); }
  T XSize() const { return this->XMax() - this->XMin(); }

  T YMax() const { return this->topLeft.Y(); }
  T YMin() const { return this->bottomRight.Y(); }
  T YSize() const { return this->YMax() - this->YMin(); }

  /// Merge patch with `_other`. Returns *this
  AxisAlignedPatch2<T> & Merge(const AxisAlignedPatch2<T> & _other)
  {
    this->topLeft.Set(
      std::max(this->topLeft.X(), _other.topLeft.X()),
      std::max(this->topLeft.Y(), _other.topLeft.Y()));
    this->bottomRight.Set(
      std::min(this->bottomRight.X(), _other.bottomRight.X()),
      std::min(this->bottomRight.Y(), _other.bottomRight.Y()));
    return *this;
  }

  AxisAlignedPatch2<T> Flip() const { return {-this->bottomRight, -this->topLeft}; }

  const AxisAlignedPatch2<T> operator*(gz::math::Vector2<T> _vector) const
  {
    return {this->topLeft * _vector, this->bottomRight * _vector};
  }

  const AxisAlignedPatch2<T> operator/(gz::math::Vector2<T> _vector) const
  {
    return {this->topLeft / _vector, this->bottomRight / _vector};
  }

  const AxisAlignedPatch2<T> operator+(gz::math::Vector2<T> _vector) const
  {
    return {this->topLeft + _vector, this->bottomRight + _vector};
  }

  const AxisAlignedPatch2<T> operator-(gz::math::Vector2<T> _vector) const
  {
    return {this->topLeft - _vector, this->bottomRight - _vector};
  }

private:
  gz::math::Vector2<T> topLeft;
  gz::math::Vector2<T> bottomRight;
};

// Handy typedefs
using AxisAlignedPatch2d = AxisAlignedPatch2<double>;
using AxisAlignedPatch2i = AxisAlignedPatch2<int>;

}  // namespace gz::sensors

#endif  // GZ_SENSORS_AXISALIGNEDPATCH2_HH_
