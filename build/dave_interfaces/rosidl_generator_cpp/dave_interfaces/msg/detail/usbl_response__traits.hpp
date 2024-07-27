// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from dave_interfaces:msg/UsblResponse.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "dave_interfaces/msg/usbl_response.hpp"


#ifndef DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__TRAITS_HPP_
#define DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "dave_interfaces/msg/detail/usbl_response__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace dave_interfaces
{

namespace msg
{

inline void to_flow_style_yaml(
  const UsblResponse & msg,
  std::ostream & out)
{
  out << "{";
  // member: transceiver_id
  {
    out << "transceiver_id: ";
    rosidl_generator_traits::value_to_yaml(msg.transceiver_id, out);
    out << ", ";
  }

  // member: response_id
  {
    out << "response_id: ";
    rosidl_generator_traits::value_to_yaml(msg.response_id, out);
    out << ", ";
  }

  // member: data
  {
    out << "data: ";
    rosidl_generator_traits::value_to_yaml(msg.data, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const UsblResponse & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: transceiver_id
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "transceiver_id: ";
    rosidl_generator_traits::value_to_yaml(msg.transceiver_id, out);
    out << "\n";
  }

  // member: response_id
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "response_id: ";
    rosidl_generator_traits::value_to_yaml(msg.response_id, out);
    out << "\n";
  }

  // member: data
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "data: ";
    rosidl_generator_traits::value_to_yaml(msg.data, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const UsblResponse & msg, bool use_flow_style = false)
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
  const dave_interfaces::msg::UsblResponse & msg,
  std::ostream & out, size_t indentation = 0)
{
  dave_interfaces::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use dave_interfaces::msg::to_yaml() instead")]]
inline std::string to_yaml(const dave_interfaces::msg::UsblResponse & msg)
{
  return dave_interfaces::msg::to_yaml(msg);
}

template<>
inline const char * data_type<dave_interfaces::msg::UsblResponse>()
{
  return "dave_interfaces::msg::UsblResponse";
}

template<>
inline const char * name<dave_interfaces::msg::UsblResponse>()
{
  return "dave_interfaces/msg/UsblResponse";
}

template<>
struct has_fixed_size<dave_interfaces::msg::UsblResponse>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<dave_interfaces::msg::UsblResponse>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<dave_interfaces::msg::UsblResponse>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__TRAITS_HPP_
