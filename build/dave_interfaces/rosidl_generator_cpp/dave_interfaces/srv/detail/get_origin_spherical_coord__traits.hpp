// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from dave_interfaces:srv/GetOriginSphericalCoord.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "dave_interfaces/srv/get_origin_spherical_coord.hpp"


#ifndef DAVE_INTERFACES__SRV__DETAIL__GET_ORIGIN_SPHERICAL_COORD__TRAITS_HPP_
#define DAVE_INTERFACES__SRV__DETAIL__GET_ORIGIN_SPHERICAL_COORD__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "dave_interfaces/srv/detail/get_origin_spherical_coord__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace dave_interfaces
{

namespace srv
{

inline void to_flow_style_yaml(
  const GetOriginSphericalCoord_Request & msg,
  std::ostream & out)
{
  (void)msg;
  out << "null";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const GetOriginSphericalCoord_Request & msg,
  std::ostream & out, size_t indentation = 0)
{
  (void)msg;
  (void)indentation;
  out << "null\n";
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const GetOriginSphericalCoord_Request & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace srv

}  // namespace dave_interfaces

namespace rosidl_generator_traits
{

[[deprecated("use dave_interfaces::srv::to_block_style_yaml() instead")]]
inline void to_yaml(
  const dave_interfaces::srv::GetOriginSphericalCoord_Request & msg,
  std::ostream & out, size_t indentation = 0)
{
  dave_interfaces::srv::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use dave_interfaces::srv::to_yaml() instead")]]
inline std::string to_yaml(const dave_interfaces::srv::GetOriginSphericalCoord_Request & msg)
{
  return dave_interfaces::srv::to_yaml(msg);
}

template<>
inline const char * data_type<dave_interfaces::srv::GetOriginSphericalCoord_Request>()
{
  return "dave_interfaces::srv::GetOriginSphericalCoord_Request";
}

template<>
inline const char * name<dave_interfaces::srv::GetOriginSphericalCoord_Request>()
{
  return "dave_interfaces/srv/GetOriginSphericalCoord_Request";
}

template<>
struct has_fixed_size<dave_interfaces::srv::GetOriginSphericalCoord_Request>
  : std::integral_constant<bool, true> {};

template<>
struct has_bounded_size<dave_interfaces::srv::GetOriginSphericalCoord_Request>
  : std::integral_constant<bool, true> {};

template<>
struct is_message<dave_interfaces::srv::GetOriginSphericalCoord_Request>
  : std::true_type {};

}  // namespace rosidl_generator_traits

namespace dave_interfaces
{

namespace srv
{

inline void to_flow_style_yaml(
  const GetOriginSphericalCoord_Response & msg,
  std::ostream & out)
{
  out << "{";
  // member: latitude_deg
  {
    out << "latitude_deg: ";
    rosidl_generator_traits::value_to_yaml(msg.latitude_deg, out);
    out << ", ";
  }

  // member: longitude_deg
  {
    out << "longitude_deg: ";
    rosidl_generator_traits::value_to_yaml(msg.longitude_deg, out);
    out << ", ";
  }

  // member: altitude
  {
    out << "altitude: ";
    rosidl_generator_traits::value_to_yaml(msg.altitude, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const GetOriginSphericalCoord_Response & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: latitude_deg
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "latitude_deg: ";
    rosidl_generator_traits::value_to_yaml(msg.latitude_deg, out);
    out << "\n";
  }

  // member: longitude_deg
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "longitude_deg: ";
    rosidl_generator_traits::value_to_yaml(msg.longitude_deg, out);
    out << "\n";
  }

  // member: altitude
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "altitude: ";
    rosidl_generator_traits::value_to_yaml(msg.altitude, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const GetOriginSphericalCoord_Response & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace srv

}  // namespace dave_interfaces

namespace rosidl_generator_traits
{

[[deprecated("use dave_interfaces::srv::to_block_style_yaml() instead")]]
inline void to_yaml(
  const dave_interfaces::srv::GetOriginSphericalCoord_Response & msg,
  std::ostream & out, size_t indentation = 0)
{
  dave_interfaces::srv::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use dave_interfaces::srv::to_yaml() instead")]]
inline std::string to_yaml(const dave_interfaces::srv::GetOriginSphericalCoord_Response & msg)
{
  return dave_interfaces::srv::to_yaml(msg);
}

template<>
inline const char * data_type<dave_interfaces::srv::GetOriginSphericalCoord_Response>()
{
  return "dave_interfaces::srv::GetOriginSphericalCoord_Response";
}

template<>
inline const char * name<dave_interfaces::srv::GetOriginSphericalCoord_Response>()
{
  return "dave_interfaces/srv/GetOriginSphericalCoord_Response";
}

template<>
struct has_fixed_size<dave_interfaces::srv::GetOriginSphericalCoord_Response>
  : std::integral_constant<bool, true> {};

template<>
struct has_bounded_size<dave_interfaces::srv::GetOriginSphericalCoord_Response>
  : std::integral_constant<bool, true> {};

template<>
struct is_message<dave_interfaces::srv::GetOriginSphericalCoord_Response>
  : std::true_type {};

}  // namespace rosidl_generator_traits

// Include directives for member types
// Member 'info'
#include "service_msgs/msg/detail/service_event_info__traits.hpp"

namespace dave_interfaces
{

namespace srv
{

inline void to_flow_style_yaml(
  const GetOriginSphericalCoord_Event & msg,
  std::ostream & out)
{
  out << "{";
  // member: info
  {
    out << "info: ";
    to_flow_style_yaml(msg.info, out);
    out << ", ";
  }

  // member: request
  {
    if (msg.request.size() == 0) {
      out << "request: []";
    } else {
      out << "request: [";
      size_t pending_items = msg.request.size();
      for (auto item : msg.request) {
        to_flow_style_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: response
  {
    if (msg.response.size() == 0) {
      out << "response: []";
    } else {
      out << "response: [";
      size_t pending_items = msg.response.size();
      for (auto item : msg.response) {
        to_flow_style_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const GetOriginSphericalCoord_Event & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: info
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "info:\n";
    to_block_style_yaml(msg.info, out, indentation + 2);
  }

  // member: request
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.request.size() == 0) {
      out << "request: []\n";
    } else {
      out << "request:\n";
      for (auto item : msg.request) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "-\n";
        to_block_style_yaml(item, out, indentation + 2);
      }
    }
  }

  // member: response
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.response.size() == 0) {
      out << "response: []\n";
    } else {
      out << "response:\n";
      for (auto item : msg.response) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "-\n";
        to_block_style_yaml(item, out, indentation + 2);
      }
    }
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const GetOriginSphericalCoord_Event & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace srv

}  // namespace dave_interfaces

namespace rosidl_generator_traits
{

[[deprecated("use dave_interfaces::srv::to_block_style_yaml() instead")]]
inline void to_yaml(
  const dave_interfaces::srv::GetOriginSphericalCoord_Event & msg,
  std::ostream & out, size_t indentation = 0)
{
  dave_interfaces::srv::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use dave_interfaces::srv::to_yaml() instead")]]
inline std::string to_yaml(const dave_interfaces::srv::GetOriginSphericalCoord_Event & msg)
{
  return dave_interfaces::srv::to_yaml(msg);
}

template<>
inline const char * data_type<dave_interfaces::srv::GetOriginSphericalCoord_Event>()
{
  return "dave_interfaces::srv::GetOriginSphericalCoord_Event";
}

template<>
inline const char * name<dave_interfaces::srv::GetOriginSphericalCoord_Event>()
{
  return "dave_interfaces/srv/GetOriginSphericalCoord_Event";
}

template<>
struct has_fixed_size<dave_interfaces::srv::GetOriginSphericalCoord_Event>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<dave_interfaces::srv::GetOriginSphericalCoord_Event>
  : std::integral_constant<bool, has_bounded_size<dave_interfaces::srv::GetOriginSphericalCoord_Request>::value && has_bounded_size<dave_interfaces::srv::GetOriginSphericalCoord_Response>::value && has_bounded_size<service_msgs::msg::ServiceEventInfo>::value> {};

template<>
struct is_message<dave_interfaces::srv::GetOriginSphericalCoord_Event>
  : std::true_type {};

}  // namespace rosidl_generator_traits

namespace rosidl_generator_traits
{

template<>
inline const char * data_type<dave_interfaces::srv::GetOriginSphericalCoord>()
{
  return "dave_interfaces::srv::GetOriginSphericalCoord";
}

template<>
inline const char * name<dave_interfaces::srv::GetOriginSphericalCoord>()
{
  return "dave_interfaces/srv/GetOriginSphericalCoord";
}

template<>
struct has_fixed_size<dave_interfaces::srv::GetOriginSphericalCoord>
  : std::integral_constant<
    bool,
    has_fixed_size<dave_interfaces::srv::GetOriginSphericalCoord_Request>::value &&
    has_fixed_size<dave_interfaces::srv::GetOriginSphericalCoord_Response>::value
  >
{
};

template<>
struct has_bounded_size<dave_interfaces::srv::GetOriginSphericalCoord>
  : std::integral_constant<
    bool,
    has_bounded_size<dave_interfaces::srv::GetOriginSphericalCoord_Request>::value &&
    has_bounded_size<dave_interfaces::srv::GetOriginSphericalCoord_Response>::value
  >
{
};

template<>
struct is_service<dave_interfaces::srv::GetOriginSphericalCoord>
  : std::true_type
{
};

template<>
struct is_service_request<dave_interfaces::srv::GetOriginSphericalCoord_Request>
  : std::true_type
{
};

template<>
struct is_service_response<dave_interfaces::srv::GetOriginSphericalCoord_Response>
  : std::true_type
{
};

}  // namespace rosidl_generator_traits

#endif  // DAVE_INTERFACES__SRV__DETAIL__GET_ORIGIN_SPHERICAL_COORD__TRAITS_HPP_
