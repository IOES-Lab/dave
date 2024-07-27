// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from dave_interfaces:srv/TransformToSphericalCoord.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "dave_interfaces/srv/transform_to_spherical_coord.hpp"


#ifndef DAVE_INTERFACES__SRV__DETAIL__TRANSFORM_TO_SPHERICAL_COORD__BUILDER_HPP_
#define DAVE_INTERFACES__SRV__DETAIL__TRANSFORM_TO_SPHERICAL_COORD__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "dave_interfaces/srv/detail/transform_to_spherical_coord__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace dave_interfaces
{

namespace srv
{

namespace builder
{

class Init_TransformToSphericalCoord_Request_input
{
public:
  Init_TransformToSphericalCoord_Request_input()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::dave_interfaces::srv::TransformToSphericalCoord_Request input(::dave_interfaces::srv::TransformToSphericalCoord_Request::_input_type arg)
  {
    msg_.input = std::move(arg);
    return std::move(msg_);
  }

private:
  ::dave_interfaces::srv::TransformToSphericalCoord_Request msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::dave_interfaces::srv::TransformToSphericalCoord_Request>()
{
  return dave_interfaces::srv::builder::Init_TransformToSphericalCoord_Request_input();
}

}  // namespace dave_interfaces


namespace dave_interfaces
{

namespace srv
{

namespace builder
{

class Init_TransformToSphericalCoord_Response_altitude
{
public:
  explicit Init_TransformToSphericalCoord_Response_altitude(::dave_interfaces::srv::TransformToSphericalCoord_Response & msg)
  : msg_(msg)
  {}
  ::dave_interfaces::srv::TransformToSphericalCoord_Response altitude(::dave_interfaces::srv::TransformToSphericalCoord_Response::_altitude_type arg)
  {
    msg_.altitude = std::move(arg);
    return std::move(msg_);
  }

private:
  ::dave_interfaces::srv::TransformToSphericalCoord_Response msg_;
};

class Init_TransformToSphericalCoord_Response_longitude_deg
{
public:
  explicit Init_TransformToSphericalCoord_Response_longitude_deg(::dave_interfaces::srv::TransformToSphericalCoord_Response & msg)
  : msg_(msg)
  {}
  Init_TransformToSphericalCoord_Response_altitude longitude_deg(::dave_interfaces::srv::TransformToSphericalCoord_Response::_longitude_deg_type arg)
  {
    msg_.longitude_deg = std::move(arg);
    return Init_TransformToSphericalCoord_Response_altitude(msg_);
  }

private:
  ::dave_interfaces::srv::TransformToSphericalCoord_Response msg_;
};

class Init_TransformToSphericalCoord_Response_latitude_deg
{
public:
  Init_TransformToSphericalCoord_Response_latitude_deg()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_TransformToSphericalCoord_Response_longitude_deg latitude_deg(::dave_interfaces::srv::TransformToSphericalCoord_Response::_latitude_deg_type arg)
  {
    msg_.latitude_deg = std::move(arg);
    return Init_TransformToSphericalCoord_Response_longitude_deg(msg_);
  }

private:
  ::dave_interfaces::srv::TransformToSphericalCoord_Response msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::dave_interfaces::srv::TransformToSphericalCoord_Response>()
{
  return dave_interfaces::srv::builder::Init_TransformToSphericalCoord_Response_latitude_deg();
}

}  // namespace dave_interfaces


namespace dave_interfaces
{

namespace srv
{

namespace builder
{

class Init_TransformToSphericalCoord_Event_response
{
public:
  explicit Init_TransformToSphericalCoord_Event_response(::dave_interfaces::srv::TransformToSphericalCoord_Event & msg)
  : msg_(msg)
  {}
  ::dave_interfaces::srv::TransformToSphericalCoord_Event response(::dave_interfaces::srv::TransformToSphericalCoord_Event::_response_type arg)
  {
    msg_.response = std::move(arg);
    return std::move(msg_);
  }

private:
  ::dave_interfaces::srv::TransformToSphericalCoord_Event msg_;
};

class Init_TransformToSphericalCoord_Event_request
{
public:
  explicit Init_TransformToSphericalCoord_Event_request(::dave_interfaces::srv::TransformToSphericalCoord_Event & msg)
  : msg_(msg)
  {}
  Init_TransformToSphericalCoord_Event_response request(::dave_interfaces::srv::TransformToSphericalCoord_Event::_request_type arg)
  {
    msg_.request = std::move(arg);
    return Init_TransformToSphericalCoord_Event_response(msg_);
  }

private:
  ::dave_interfaces::srv::TransformToSphericalCoord_Event msg_;
};

class Init_TransformToSphericalCoord_Event_info
{
public:
  Init_TransformToSphericalCoord_Event_info()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_TransformToSphericalCoord_Event_request info(::dave_interfaces::srv::TransformToSphericalCoord_Event::_info_type arg)
  {
    msg_.info = std::move(arg);
    return Init_TransformToSphericalCoord_Event_request(msg_);
  }

private:
  ::dave_interfaces::srv::TransformToSphericalCoord_Event msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::dave_interfaces::srv::TransformToSphericalCoord_Event>()
{
  return dave_interfaces::srv::builder::Init_TransformToSphericalCoord_Event_info();
}

}  // namespace dave_interfaces

#endif  // DAVE_INTERFACES__SRV__DETAIL__TRANSFORM_TO_SPHERICAL_COORD__BUILDER_HPP_
