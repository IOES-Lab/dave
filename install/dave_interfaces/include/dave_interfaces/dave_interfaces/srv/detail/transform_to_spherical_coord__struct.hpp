// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from dave_interfaces:srv/TransformToSphericalCoord.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "dave_interfaces/srv/transform_to_spherical_coord.hpp"


#ifndef DAVE_INTERFACES__SRV__DETAIL__TRANSFORM_TO_SPHERICAL_COORD__STRUCT_HPP_
#define DAVE_INTERFACES__SRV__DETAIL__TRANSFORM_TO_SPHERICAL_COORD__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'input'
#include "geometry_msgs/msg/detail/vector3__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__dave_interfaces__srv__TransformToSphericalCoord_Request __attribute__((deprecated))
#else
# define DEPRECATED__dave_interfaces__srv__TransformToSphericalCoord_Request __declspec(deprecated)
#endif

namespace dave_interfaces
{

namespace srv
{

// message struct
template<class ContainerAllocator>
struct TransformToSphericalCoord_Request_
{
  using Type = TransformToSphericalCoord_Request_<ContainerAllocator>;

  explicit TransformToSphericalCoord_Request_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : input(_init)
  {
    (void)_init;
  }

  explicit TransformToSphericalCoord_Request_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : input(_alloc, _init)
  {
    (void)_init;
  }

  // field types and members
  using _input_type =
    geometry_msgs::msg::Vector3_<ContainerAllocator>;
  _input_type input;

  // setters for named parameter idiom
  Type & set__input(
    const geometry_msgs::msg::Vector3_<ContainerAllocator> & _arg)
  {
    this->input = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator> *;
  using ConstRawPtr =
    const dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__dave_interfaces__srv__TransformToSphericalCoord_Request
    std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__dave_interfaces__srv__TransformToSphericalCoord_Request
    std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const TransformToSphericalCoord_Request_ & other) const
  {
    if (this->input != other.input) {
      return false;
    }
    return true;
  }
  bool operator!=(const TransformToSphericalCoord_Request_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct TransformToSphericalCoord_Request_

// alias to use template instance with default allocator
using TransformToSphericalCoord_Request =
  dave_interfaces::srv::TransformToSphericalCoord_Request_<std::allocator<void>>;

// constant definitions

}  // namespace srv

}  // namespace dave_interfaces


#ifndef _WIN32
# define DEPRECATED__dave_interfaces__srv__TransformToSphericalCoord_Response __attribute__((deprecated))
#else
# define DEPRECATED__dave_interfaces__srv__TransformToSphericalCoord_Response __declspec(deprecated)
#endif

namespace dave_interfaces
{

namespace srv
{

// message struct
template<class ContainerAllocator>
struct TransformToSphericalCoord_Response_
{
  using Type = TransformToSphericalCoord_Response_<ContainerAllocator>;

  explicit TransformToSphericalCoord_Response_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->latitude_deg = 0.0;
      this->longitude_deg = 0.0;
      this->altitude = 0.0;
    }
  }

  explicit TransformToSphericalCoord_Response_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->latitude_deg = 0.0;
      this->longitude_deg = 0.0;
      this->altitude = 0.0;
    }
  }

  // field types and members
  using _latitude_deg_type =
    double;
  _latitude_deg_type latitude_deg;
  using _longitude_deg_type =
    double;
  _longitude_deg_type longitude_deg;
  using _altitude_type =
    double;
  _altitude_type altitude;

  // setters for named parameter idiom
  Type & set__latitude_deg(
    const double & _arg)
  {
    this->latitude_deg = _arg;
    return *this;
  }
  Type & set__longitude_deg(
    const double & _arg)
  {
    this->longitude_deg = _arg;
    return *this;
  }
  Type & set__altitude(
    const double & _arg)
  {
    this->altitude = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator> *;
  using ConstRawPtr =
    const dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__dave_interfaces__srv__TransformToSphericalCoord_Response
    std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__dave_interfaces__srv__TransformToSphericalCoord_Response
    std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const TransformToSphericalCoord_Response_ & other) const
  {
    if (this->latitude_deg != other.latitude_deg) {
      return false;
    }
    if (this->longitude_deg != other.longitude_deg) {
      return false;
    }
    if (this->altitude != other.altitude) {
      return false;
    }
    return true;
  }
  bool operator!=(const TransformToSphericalCoord_Response_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct TransformToSphericalCoord_Response_

// alias to use template instance with default allocator
using TransformToSphericalCoord_Response =
  dave_interfaces::srv::TransformToSphericalCoord_Response_<std::allocator<void>>;

// constant definitions

}  // namespace srv

}  // namespace dave_interfaces


// Include directives for member types
// Member 'info'
#include "service_msgs/msg/detail/service_event_info__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__dave_interfaces__srv__TransformToSphericalCoord_Event __attribute__((deprecated))
#else
# define DEPRECATED__dave_interfaces__srv__TransformToSphericalCoord_Event __declspec(deprecated)
#endif

namespace dave_interfaces
{

namespace srv
{

// message struct
template<class ContainerAllocator>
struct TransformToSphericalCoord_Event_
{
  using Type = TransformToSphericalCoord_Event_<ContainerAllocator>;

  explicit TransformToSphericalCoord_Event_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : info(_init)
  {
    (void)_init;
  }

  explicit TransformToSphericalCoord_Event_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : info(_alloc, _init)
  {
    (void)_init;
  }

  // field types and members
  using _info_type =
    service_msgs::msg::ServiceEventInfo_<ContainerAllocator>;
  _info_type info;
  using _request_type =
    rosidl_runtime_cpp::BoundedVector<dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator>, 1, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator>>>;
  _request_type request;
  using _response_type =
    rosidl_runtime_cpp::BoundedVector<dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator>, 1, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator>>>;
  _response_type response;

  // setters for named parameter idiom
  Type & set__info(
    const service_msgs::msg::ServiceEventInfo_<ContainerAllocator> & _arg)
  {
    this->info = _arg;
    return *this;
  }
  Type & set__request(
    const rosidl_runtime_cpp::BoundedVector<dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator>, 1, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<dave_interfaces::srv::TransformToSphericalCoord_Request_<ContainerAllocator>>> & _arg)
  {
    this->request = _arg;
    return *this;
  }
  Type & set__response(
    const rosidl_runtime_cpp::BoundedVector<dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator>, 1, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<dave_interfaces::srv::TransformToSphericalCoord_Response_<ContainerAllocator>>> & _arg)
  {
    this->response = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    dave_interfaces::srv::TransformToSphericalCoord_Event_<ContainerAllocator> *;
  using ConstRawPtr =
    const dave_interfaces::srv::TransformToSphericalCoord_Event_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord_Event_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord_Event_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      dave_interfaces::srv::TransformToSphericalCoord_Event_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<dave_interfaces::srv::TransformToSphericalCoord_Event_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      dave_interfaces::srv::TransformToSphericalCoord_Event_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<dave_interfaces::srv::TransformToSphericalCoord_Event_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<dave_interfaces::srv::TransformToSphericalCoord_Event_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<dave_interfaces::srv::TransformToSphericalCoord_Event_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__dave_interfaces__srv__TransformToSphericalCoord_Event
    std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord_Event_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__dave_interfaces__srv__TransformToSphericalCoord_Event
    std::shared_ptr<dave_interfaces::srv::TransformToSphericalCoord_Event_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const TransformToSphericalCoord_Event_ & other) const
  {
    if (this->info != other.info) {
      return false;
    }
    if (this->request != other.request) {
      return false;
    }
    if (this->response != other.response) {
      return false;
    }
    return true;
  }
  bool operator!=(const TransformToSphericalCoord_Event_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct TransformToSphericalCoord_Event_

// alias to use template instance with default allocator
using TransformToSphericalCoord_Event =
  dave_interfaces::srv::TransformToSphericalCoord_Event_<std::allocator<void>>;

// constant definitions

}  // namespace srv

}  // namespace dave_interfaces

namespace dave_interfaces
{

namespace srv
{

struct TransformToSphericalCoord
{
  using Request = dave_interfaces::srv::TransformToSphericalCoord_Request;
  using Response = dave_interfaces::srv::TransformToSphericalCoord_Response;
  using Event = dave_interfaces::srv::TransformToSphericalCoord_Event;
};

}  // namespace srv

}  // namespace dave_interfaces

#endif  // DAVE_INTERFACES__SRV__DETAIL__TRANSFORM_TO_SPHERICAL_COORD__STRUCT_HPP_
