// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from dave_interfaces:msg/UsblResponse.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "dave_interfaces/msg/usbl_response.hpp"


#ifndef DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__STRUCT_HPP_
#define DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__dave_interfaces__msg__UsblResponse __attribute__((deprecated))
#else
# define DEPRECATED__dave_interfaces__msg__UsblResponse __declspec(deprecated)
#endif

namespace dave_interfaces
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct UsblResponse_
{
  using Type = UsblResponse_<ContainerAllocator>;

  explicit UsblResponse_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->transceiver_id = 0l;
      this->response_id = 0l;
      this->data = "";
    }
  }

  explicit UsblResponse_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : data(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->transceiver_id = 0l;
      this->response_id = 0l;
      this->data = "";
    }
  }

  // field types and members
  using _transceiver_id_type =
    int32_t;
  _transceiver_id_type transceiver_id;
  using _response_id_type =
    int32_t;
  _response_id_type response_id;
  using _data_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _data_type data;

  // setters for named parameter idiom
  Type & set__transceiver_id(
    const int32_t & _arg)
  {
    this->transceiver_id = _arg;
    return *this;
  }
  Type & set__response_id(
    const int32_t & _arg)
  {
    this->response_id = _arg;
    return *this;
  }
  Type & set__data(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->data = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    dave_interfaces::msg::UsblResponse_<ContainerAllocator> *;
  using ConstRawPtr =
    const dave_interfaces::msg::UsblResponse_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<dave_interfaces::msg::UsblResponse_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<dave_interfaces::msg::UsblResponse_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      dave_interfaces::msg::UsblResponse_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<dave_interfaces::msg::UsblResponse_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      dave_interfaces::msg::UsblResponse_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<dave_interfaces::msg::UsblResponse_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<dave_interfaces::msg::UsblResponse_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<dave_interfaces::msg::UsblResponse_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__dave_interfaces__msg__UsblResponse
    std::shared_ptr<dave_interfaces::msg::UsblResponse_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__dave_interfaces__msg__UsblResponse
    std::shared_ptr<dave_interfaces::msg::UsblResponse_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const UsblResponse_ & other) const
  {
    if (this->transceiver_id != other.transceiver_id) {
      return false;
    }
    if (this->response_id != other.response_id) {
      return false;
    }
    if (this->data != other.data) {
      return false;
    }
    return true;
  }
  bool operator!=(const UsblResponse_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct UsblResponse_

// alias to use template instance with default allocator
using UsblResponse =
  dave_interfaces::msg::UsblResponse_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace dave_interfaces

#endif  // DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__STRUCT_HPP_
