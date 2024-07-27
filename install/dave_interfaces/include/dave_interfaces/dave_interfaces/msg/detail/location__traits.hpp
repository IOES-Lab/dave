// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from dave_interfaces:msg/Location.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "dave_interfaces/msg/location.hpp"


#ifndef DAVE_INTERFACES__MSG__DETAIL__LOCATION__TRAITS_HPP_
#define DAVE_INTERFACES__MSG__DETAIL__LOCATION__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "dave_interfaces/msg/detail/location__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace dave_interfaces
{

namespace msg
{

inline void to_flow_style_yaml(
  const Location & msg,
  std::ostream & out)
{
  out << "{";
  // member: transponder_id
  {
    out << "transponder_id: ";
    rosidl_generator_traits::value_to_yaml(msg.transponder_id, out);
    out << ", ";
  }

  // member: x
  {
    out << "x: ";
    rosidl_generator_traits::value_to_yaml(msg.x, out);
    out << ", ";
  }

  // member: y
  {
    out << "y: ";
    rosidl_generator_traits::value_to_yaml(msg.y, out);
    out << ", ";
  }

  // member: z
  {
    out << "z: ";
    rosidl_generator_traits::value_to_yaml(msg.z, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const Location & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: transponder_id
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "transponder_id: ";
    rosidl_generator_traits::value_to_yaml(msg.transponder_id, out);
    out << "\n";
  }

  // member: x
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "x: ";
    rosidl_generator_traits::value_to_yaml(msg.x, out);
    out << "\n";
  }

  // member: y
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "y: ";
    rosidl_generator_traits::value_to_yaml(msg.y, out);
    out << "\n";
  }

  // member: z
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "z: ";
    rosidl_generator_traits::value_to_yaml(msg.z, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const Location & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace dave_interfaces

namespace rosidl_generator_traits
{

[[deprecated("use dave_interfaces::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const dave_interfaces::msg::Location & msg,
  std::ostream & out, size_t indentation = 0)
{
  dave_interfaces::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use dave_interfaces::msg::to_yaml() instead")]]
inline std::string to_yaml(const dave_interfaces::msg::Location & msg)
{
  return dave_interfaces::msg::to_yaml(msg);
}

template<>
inline const char * data_type<dave_interfaces::msg::Location>()
{
  return "dave_interfaces::msg::Location";
}

template<>
inline const char * name<dave_interfaces::msg::Location>()
{
  return "dave_interfaces/msg/Location";
}

template<>
struct has_fixed_size<dave_interfaces::msg::Location>
  : std::integral_constant<bool, true> {};

template<>
struct has_bounded_size<dave_interfaces::msg::Location>
  : std::integral_constant<bool, true> {};

template<>
struct is_message<dave_interfaces::msg::Location>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // DAVE_INTERFACES__MSG__DETAIL__LOCATION__TRAITS_HPP_
