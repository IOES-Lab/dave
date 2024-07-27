// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from dave_interfaces:msg/Location.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "dave_interfaces/msg/location.hpp"


#ifndef DAVE_INTERFACES__MSG__DETAIL__LOCATION__BUILDER_HPP_
#define DAVE_INTERFACES__MSG__DETAIL__LOCATION__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "dave_interfaces/msg/detail/location__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace dave_interfaces
{

namespace msg
{

namespace builder
{

class Init_Location_z
{
public:
  explicit Init_Location_z(::dave_interfaces::msg::Location & msg)
  : msg_(msg)
  {}
  ::dave_interfaces::msg::Location z(::dave_interfaces::msg::Location::_z_type arg)
  {
    msg_.z = std::move(arg);
    return std::move(msg_);
  }

private:
  ::dave_interfaces::msg::Location msg_;
};

class Init_Location_y
{
public:
  explicit Init_Location_y(::dave_interfaces::msg::Location & msg)
  : msg_(msg)
  {}
  Init_Location_z y(::dave_interfaces::msg::Location::_y_type arg)
  {
    msg_.y = std::move(arg);
    return Init_Location_z(msg_);
  }

private:
  ::dave_interfaces::msg::Location msg_;
};

class Init_Location_x
{
public:
  explicit Init_Location_x(::dave_interfaces::msg::Location & msg)
  : msg_(msg)
  {}
  Init_Location_y x(::dave_interfaces::msg::Location::_x_type arg)
  {
    msg_.x = std::move(arg);
    return Init_Location_y(msg_);
  }

private:
  ::dave_interfaces::msg::Location msg_;
};

class Init_Location_transponder_id
{
public:
  Init_Location_transponder_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Location_x transponder_id(::dave_interfaces::msg::Location::_transponder_id_type arg)
  {
    msg_.transponder_id = std::move(arg);
    return Init_Location_x(msg_);
  }

private:
  ::dave_interfaces::msg::Location msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::dave_interfaces::msg::Location>()
{
  return dave_interfaces::msg::builder::Init_Location_transponder_id();
}

}  // namespace dave_interfaces

#endif  // DAVE_INTERFACES__MSG__DETAIL__LOCATION__BUILDER_HPP_
