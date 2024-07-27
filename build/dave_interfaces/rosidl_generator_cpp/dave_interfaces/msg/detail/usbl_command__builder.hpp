// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from dave_interfaces:msg/UsblCommand.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "dave_interfaces/msg/usbl_command.hpp"


#ifndef DAVE_INTERFACES__MSG__DETAIL__USBL_COMMAND__BUILDER_HPP_
#define DAVE_INTERFACES__MSG__DETAIL__USBL_COMMAND__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "dave_interfaces/msg/detail/usbl_command__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace dave_interfaces
{

namespace msg
{

namespace builder
{

class Init_UsblCommand_data
{
public:
  explicit Init_UsblCommand_data(::dave_interfaces::msg::UsblCommand & msg)
  : msg_(msg)
  {}
  ::dave_interfaces::msg::UsblCommand data(::dave_interfaces::msg::UsblCommand::_data_type arg)
  {
    msg_.data = std::move(arg);
    return std::move(msg_);
  }

private:
  ::dave_interfaces::msg::UsblCommand msg_;
};

class Init_UsblCommand_command_id
{
public:
  explicit Init_UsblCommand_command_id(::dave_interfaces::msg::UsblCommand & msg)
  : msg_(msg)
  {}
  Init_UsblCommand_data command_id(::dave_interfaces::msg::UsblCommand::_command_id_type arg)
  {
    msg_.command_id = std::move(arg);
    return Init_UsblCommand_data(msg_);
  }

private:
  ::dave_interfaces::msg::UsblCommand msg_;
};

class Init_UsblCommand_transponder_id
{
public:
  Init_UsblCommand_transponder_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_UsblCommand_command_id transponder_id(::dave_interfaces::msg::UsblCommand::_transponder_id_type arg)
  {
    msg_.transponder_id = std::move(arg);
    return Init_UsblCommand_command_id(msg_);
  }

private:
  ::dave_interfaces::msg::UsblCommand msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::dave_interfaces::msg::UsblCommand>()
{
  return dave_interfaces::msg::builder::Init_UsblCommand_transponder_id();
}

}  // namespace dave_interfaces

#endif  // DAVE_INTERFACES__MSG__DETAIL__USBL_COMMAND__BUILDER_HPP_
