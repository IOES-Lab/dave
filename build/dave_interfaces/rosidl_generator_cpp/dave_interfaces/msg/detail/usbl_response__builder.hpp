// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from dave_interfaces:msg/UsblResponse.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "dave_interfaces/msg/usbl_response.hpp"


#ifndef DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__BUILDER_HPP_
#define DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "dave_interfaces/msg/detail/usbl_response__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace dave_interfaces
{

namespace msg
{

namespace builder
{

class Init_UsblResponse_data
{
public:
  explicit Init_UsblResponse_data(::dave_interfaces::msg::UsblResponse & msg)
  : msg_(msg)
  {}
  ::dave_interfaces::msg::UsblResponse data(::dave_interfaces::msg::UsblResponse::_data_type arg)
  {
    msg_.data = std::move(arg);
    return std::move(msg_);
  }

private:
  ::dave_interfaces::msg::UsblResponse msg_;
};

class Init_UsblResponse_response_id
{
public:
  explicit Init_UsblResponse_response_id(::dave_interfaces::msg::UsblResponse & msg)
  : msg_(msg)
  {}
  Init_UsblResponse_data response_id(::dave_interfaces::msg::UsblResponse::_response_id_type arg)
  {
    msg_.response_id = std::move(arg);
    return Init_UsblResponse_data(msg_);
  }

private:
  ::dave_interfaces::msg::UsblResponse msg_;
};

class Init_UsblResponse_transceiver_id
{
public:
  Init_UsblResponse_transceiver_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_UsblResponse_response_id transceiver_id(::dave_interfaces::msg::UsblResponse::_transceiver_id_type arg)
  {
    msg_.transceiver_id = std::move(arg);
    return Init_UsblResponse_response_id(msg_);
  }

private:
  ::dave_interfaces::msg::UsblResponse msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::dave_interfaces::msg::UsblResponse>()
{
  return dave_interfaces::msg::builder::Init_UsblResponse_transceiver_id();
}

}  // namespace dave_interfaces

#endif  // DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__BUILDER_HPP_
