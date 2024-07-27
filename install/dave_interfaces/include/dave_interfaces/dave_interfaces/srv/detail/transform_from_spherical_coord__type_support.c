// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from dave_interfaces:srv/TransformFromSphericalCoord.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "dave_interfaces/srv/detail/transform_from_spherical_coord__rosidl_typesupport_introspection_c.h"
#include "dave_interfaces/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "dave_interfaces/srv/detail/transform_from_spherical_coord__functions.h"
#include "dave_interfaces/srv/detail/transform_from_spherical_coord__struct.h"


#ifdef __cplusplus
extern "C"
{
#endif

void dave_interfaces__srv__TransformFromSphericalCoord_Request__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  dave_interfaces__srv__TransformFromSphericalCoord_Request__init(message_memory);
}

void dave_interfaces__srv__TransformFromSphericalCoord_Request__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_fini_function(void * message_memory)
{
  dave_interfaces__srv__TransformFromSphericalCoord_Request__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember dave_interfaces__srv__TransformFromSphericalCoord_Request__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_message_member_array[3] = {
  {
    "latitude_deg",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(dave_interfaces__srv__TransformFromSphericalCoord_Request, latitude_deg),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "longitude_deg",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(dave_interfaces__srv__TransformFromSphericalCoord_Request, longitude_deg),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "altitude",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(dave_interfaces__srv__TransformFromSphericalCoord_Request, altitude),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers dave_interfaces__srv__TransformFromSphericalCoord_Request__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_message_members = {
  "dave_interfaces__srv",  // message namespace
  "TransformFromSphericalCoord_Request",  // message name
  3,  // number of fields
  sizeof(dave_interfaces__srv__TransformFromSphericalCoord_Request),
  false,  // has_any_key_member_
  dave_interfaces__srv__TransformFromSphericalCoord_Request__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_message_member_array,  // message members
  dave_interfaces__srv__TransformFromSphericalCoord_Request__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_init_function,  // function to initialize message memory (memory has to be allocated)
  dave_interfaces__srv__TransformFromSphericalCoord_Request__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t dave_interfaces__srv__TransformFromSphericalCoord_Request__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_message_type_support_handle = {
  0,
  &dave_interfaces__srv__TransformFromSphericalCoord_Request__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_message_members,
  get_message_typesupport_handle_function,
  &dave_interfaces__srv__TransformFromSphericalCoord_Request__get_type_hash,
  &dave_interfaces__srv__TransformFromSphericalCoord_Request__get_type_description,
  &dave_interfaces__srv__TransformFromSphericalCoord_Request__get_type_description_sources,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_dave_interfaces
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, dave_interfaces, srv, TransformFromSphericalCoord_Request)() {
  if (!dave_interfaces__srv__TransformFromSphericalCoord_Request__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_message_type_support_handle.typesupport_identifier) {
    dave_interfaces__srv__TransformFromSphericalCoord_Request__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &dave_interfaces__srv__TransformFromSphericalCoord_Request__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif

// already included above
// #include <stddef.h>
// already included above
// #include "dave_interfaces/srv/detail/transform_from_spherical_coord__rosidl_typesupport_introspection_c.h"
// already included above
// #include "dave_interfaces/msg/rosidl_typesupport_introspection_c__visibility_control.h"
// already included above
// #include "rosidl_typesupport_introspection_c/field_types.h"
// already included above
// #include "rosidl_typesupport_introspection_c/identifier.h"
// already included above
// #include "rosidl_typesupport_introspection_c/message_introspection.h"
// already included above
// #include "dave_interfaces/srv/detail/transform_from_spherical_coord__functions.h"
// already included above
// #include "dave_interfaces/srv/detail/transform_from_spherical_coord__struct.h"


// Include directives for member types
// Member `output`
#include "geometry_msgs/msg/vector3.h"
// Member `output`
#include "geometry_msgs/msg/detail/vector3__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  dave_interfaces__srv__TransformFromSphericalCoord_Response__init(message_memory);
}

void dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_fini_function(void * message_memory)
{
  dave_interfaces__srv__TransformFromSphericalCoord_Response__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_message_member_array[1] = {
  {
    "output",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(dave_interfaces__srv__TransformFromSphericalCoord_Response, output),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_message_members = {
  "dave_interfaces__srv",  // message namespace
  "TransformFromSphericalCoord_Response",  // message name
  1,  // number of fields
  sizeof(dave_interfaces__srv__TransformFromSphericalCoord_Response),
  false,  // has_any_key_member_
  dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_message_member_array,  // message members
  dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_init_function,  // function to initialize message memory (memory has to be allocated)
  dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_message_type_support_handle = {
  0,
  &dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_message_members,
  get_message_typesupport_handle_function,
  &dave_interfaces__srv__TransformFromSphericalCoord_Response__get_type_hash,
  &dave_interfaces__srv__TransformFromSphericalCoord_Response__get_type_description,
  &dave_interfaces__srv__TransformFromSphericalCoord_Response__get_type_description_sources,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_dave_interfaces
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, dave_interfaces, srv, TransformFromSphericalCoord_Response)() {
  dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, geometry_msgs, msg, Vector3)();
  if (!dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_message_type_support_handle.typesupport_identifier) {
    dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif

// already included above
// #include <stddef.h>
// already included above
// #include "dave_interfaces/srv/detail/transform_from_spherical_coord__rosidl_typesupport_introspection_c.h"
// already included above
// #include "dave_interfaces/msg/rosidl_typesupport_introspection_c__visibility_control.h"
// already included above
// #include "rosidl_typesupport_introspection_c/field_types.h"
// already included above
// #include "rosidl_typesupport_introspection_c/identifier.h"
// already included above
// #include "rosidl_typesupport_introspection_c/message_introspection.h"
// already included above
// #include "dave_interfaces/srv/detail/transform_from_spherical_coord__functions.h"
// already included above
// #include "dave_interfaces/srv/detail/transform_from_spherical_coord__struct.h"


// Include directives for member types
// Member `info`
#include "service_msgs/msg/service_event_info.h"
// Member `info`
#include "service_msgs/msg/detail/service_event_info__rosidl_typesupport_introspection_c.h"
// Member `request`
// Member `response`
#include "dave_interfaces/srv/transform_from_spherical_coord.h"
// Member `request`
// Member `response`
// already included above
// #include "dave_interfaces/srv/detail/transform_from_spherical_coord__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  dave_interfaces__srv__TransformFromSphericalCoord_Event__init(message_memory);
}

void dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_fini_function(void * message_memory)
{
  dave_interfaces__srv__TransformFromSphericalCoord_Event__fini(message_memory);
}

size_t dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__size_function__TransformFromSphericalCoord_Event__request(
  const void * untyped_member)
{
  const dave_interfaces__srv__TransformFromSphericalCoord_Request__Sequence * member =
    (const dave_interfaces__srv__TransformFromSphericalCoord_Request__Sequence *)(untyped_member);
  return member->size;
}

const void * dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__get_const_function__TransformFromSphericalCoord_Event__request(
  const void * untyped_member, size_t index)
{
  const dave_interfaces__srv__TransformFromSphericalCoord_Request__Sequence * member =
    (const dave_interfaces__srv__TransformFromSphericalCoord_Request__Sequence *)(untyped_member);
  return &member->data[index];
}

void * dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__get_function__TransformFromSphericalCoord_Event__request(
  void * untyped_member, size_t index)
{
  dave_interfaces__srv__TransformFromSphericalCoord_Request__Sequence * member =
    (dave_interfaces__srv__TransformFromSphericalCoord_Request__Sequence *)(untyped_member);
  return &member->data[index];
}

void dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__fetch_function__TransformFromSphericalCoord_Event__request(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const dave_interfaces__srv__TransformFromSphericalCoord_Request * item =
    ((const dave_interfaces__srv__TransformFromSphericalCoord_Request *)
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__get_const_function__TransformFromSphericalCoord_Event__request(untyped_member, index));
  dave_interfaces__srv__TransformFromSphericalCoord_Request * value =
    (dave_interfaces__srv__TransformFromSphericalCoord_Request *)(untyped_value);
  *value = *item;
}

void dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__assign_function__TransformFromSphericalCoord_Event__request(
  void * untyped_member, size_t index, const void * untyped_value)
{
  dave_interfaces__srv__TransformFromSphericalCoord_Request * item =
    ((dave_interfaces__srv__TransformFromSphericalCoord_Request *)
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__get_function__TransformFromSphericalCoord_Event__request(untyped_member, index));
  const dave_interfaces__srv__TransformFromSphericalCoord_Request * value =
    (const dave_interfaces__srv__TransformFromSphericalCoord_Request *)(untyped_value);
  *item = *value;
}

bool dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__resize_function__TransformFromSphericalCoord_Event__request(
  void * untyped_member, size_t size)
{
  dave_interfaces__srv__TransformFromSphericalCoord_Request__Sequence * member =
    (dave_interfaces__srv__TransformFromSphericalCoord_Request__Sequence *)(untyped_member);
  dave_interfaces__srv__TransformFromSphericalCoord_Request__Sequence__fini(member);
  return dave_interfaces__srv__TransformFromSphericalCoord_Request__Sequence__init(member, size);
}

size_t dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__size_function__TransformFromSphericalCoord_Event__response(
  const void * untyped_member)
{
  const dave_interfaces__srv__TransformFromSphericalCoord_Response__Sequence * member =
    (const dave_interfaces__srv__TransformFromSphericalCoord_Response__Sequence *)(untyped_member);
  return member->size;
}

const void * dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__get_const_function__TransformFromSphericalCoord_Event__response(
  const void * untyped_member, size_t index)
{
  const dave_interfaces__srv__TransformFromSphericalCoord_Response__Sequence * member =
    (const dave_interfaces__srv__TransformFromSphericalCoord_Response__Sequence *)(untyped_member);
  return &member->data[index];
}

void * dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__get_function__TransformFromSphericalCoord_Event__response(
  void * untyped_member, size_t index)
{
  dave_interfaces__srv__TransformFromSphericalCoord_Response__Sequence * member =
    (dave_interfaces__srv__TransformFromSphericalCoord_Response__Sequence *)(untyped_member);
  return &member->data[index];
}

void dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__fetch_function__TransformFromSphericalCoord_Event__response(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const dave_interfaces__srv__TransformFromSphericalCoord_Response * item =
    ((const dave_interfaces__srv__TransformFromSphericalCoord_Response *)
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__get_const_function__TransformFromSphericalCoord_Event__response(untyped_member, index));
  dave_interfaces__srv__TransformFromSphericalCoord_Response * value =
    (dave_interfaces__srv__TransformFromSphericalCoord_Response *)(untyped_value);
  *value = *item;
}

void dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__assign_function__TransformFromSphericalCoord_Event__response(
  void * untyped_member, size_t index, const void * untyped_value)
{
  dave_interfaces__srv__TransformFromSphericalCoord_Response * item =
    ((dave_interfaces__srv__TransformFromSphericalCoord_Response *)
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__get_function__TransformFromSphericalCoord_Event__response(untyped_member, index));
  const dave_interfaces__srv__TransformFromSphericalCoord_Response * value =
    (const dave_interfaces__srv__TransformFromSphericalCoord_Response *)(untyped_value);
  *item = *value;
}

bool dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__resize_function__TransformFromSphericalCoord_Event__response(
  void * untyped_member, size_t size)
{
  dave_interfaces__srv__TransformFromSphericalCoord_Response__Sequence * member =
    (dave_interfaces__srv__TransformFromSphericalCoord_Response__Sequence *)(untyped_member);
  dave_interfaces__srv__TransformFromSphericalCoord_Response__Sequence__fini(member);
  return dave_interfaces__srv__TransformFromSphericalCoord_Response__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_message_member_array[3] = {
  {
    "info",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(dave_interfaces__srv__TransformFromSphericalCoord_Event, info),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "request",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    true,  // is array
    1,  // array size
    true,  // is upper bound
    offsetof(dave_interfaces__srv__TransformFromSphericalCoord_Event, request),  // bytes offset in struct
    NULL,  // default value
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__size_function__TransformFromSphericalCoord_Event__request,  // size() function pointer
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__get_const_function__TransformFromSphericalCoord_Event__request,  // get_const(index) function pointer
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__get_function__TransformFromSphericalCoord_Event__request,  // get(index) function pointer
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__fetch_function__TransformFromSphericalCoord_Event__request,  // fetch(index, &value) function pointer
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__assign_function__TransformFromSphericalCoord_Event__request,  // assign(index, value) function pointer
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__resize_function__TransformFromSphericalCoord_Event__request  // resize(index) function pointer
  },
  {
    "response",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    true,  // is array
    1,  // array size
    true,  // is upper bound
    offsetof(dave_interfaces__srv__TransformFromSphericalCoord_Event, response),  // bytes offset in struct
    NULL,  // default value
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__size_function__TransformFromSphericalCoord_Event__response,  // size() function pointer
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__get_const_function__TransformFromSphericalCoord_Event__response,  // get_const(index) function pointer
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__get_function__TransformFromSphericalCoord_Event__response,  // get(index) function pointer
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__fetch_function__TransformFromSphericalCoord_Event__response,  // fetch(index, &value) function pointer
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__assign_function__TransformFromSphericalCoord_Event__response,  // assign(index, value) function pointer
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__resize_function__TransformFromSphericalCoord_Event__response  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_message_members = {
  "dave_interfaces__srv",  // message namespace
  "TransformFromSphericalCoord_Event",  // message name
  3,  // number of fields
  sizeof(dave_interfaces__srv__TransformFromSphericalCoord_Event),
  false,  // has_any_key_member_
  dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_message_member_array,  // message members
  dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_init_function,  // function to initialize message memory (memory has to be allocated)
  dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_message_type_support_handle = {
  0,
  &dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_message_members,
  get_message_typesupport_handle_function,
  &dave_interfaces__srv__TransformFromSphericalCoord_Event__get_type_hash,
  &dave_interfaces__srv__TransformFromSphericalCoord_Event__get_type_description,
  &dave_interfaces__srv__TransformFromSphericalCoord_Event__get_type_description_sources,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_dave_interfaces
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, dave_interfaces, srv, TransformFromSphericalCoord_Event)() {
  dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, service_msgs, msg, ServiceEventInfo)();
  dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_message_member_array[1].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, dave_interfaces, srv, TransformFromSphericalCoord_Request)();
  dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_message_member_array[2].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, dave_interfaces, srv, TransformFromSphericalCoord_Response)();
  if (!dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_message_type_support_handle.typesupport_identifier) {
    dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif

#include "rosidl_runtime_c/service_type_support_struct.h"
// already included above
// #include "dave_interfaces/msg/rosidl_typesupport_introspection_c__visibility_control.h"
// already included above
// #include "dave_interfaces/srv/detail/transform_from_spherical_coord__rosidl_typesupport_introspection_c.h"
// already included above
// #include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/service_introspection.h"

// this is intentionally not const to allow initialization later to prevent an initialization race
static rosidl_typesupport_introspection_c__ServiceMembers dave_interfaces__srv__detail__transform_from_spherical_coord__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_service_members = {
  "dave_interfaces__srv",  // service namespace
  "TransformFromSphericalCoord",  // service name
  // the following fields are initialized below on first access
  NULL,  // request message
  // dave_interfaces__srv__detail__transform_from_spherical_coord__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_message_type_support_handle,
  NULL,  // response message
  // dave_interfaces__srv__detail__transform_from_spherical_coord__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_message_type_support_handle
  NULL  // event_message
  // dave_interfaces__srv__detail__transform_from_spherical_coord__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_message_type_support_handle
};


static rosidl_service_type_support_t dave_interfaces__srv__detail__transform_from_spherical_coord__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_service_type_support_handle = {
  0,
  &dave_interfaces__srv__detail__transform_from_spherical_coord__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_service_members,
  get_service_typesupport_handle_function,
  &dave_interfaces__srv__TransformFromSphericalCoord_Request__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Request_message_type_support_handle,
  &dave_interfaces__srv__TransformFromSphericalCoord_Response__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Response_message_type_support_handle,
  &dave_interfaces__srv__TransformFromSphericalCoord_Event__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_Event_message_type_support_handle,
  ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_CREATE_EVENT_MESSAGE_SYMBOL_NAME(
    rosidl_typesupport_c,
    dave_interfaces,
    srv,
    TransformFromSphericalCoord
  ),
  ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_DESTROY_EVENT_MESSAGE_SYMBOL_NAME(
    rosidl_typesupport_c,
    dave_interfaces,
    srv,
    TransformFromSphericalCoord
  ),
  &dave_interfaces__srv__TransformFromSphericalCoord__get_type_hash,
  &dave_interfaces__srv__TransformFromSphericalCoord__get_type_description,
  &dave_interfaces__srv__TransformFromSphericalCoord__get_type_description_sources,
};

// Forward declaration of message type support functions for service members
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, dave_interfaces, srv, TransformFromSphericalCoord_Request)(void);

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, dave_interfaces, srv, TransformFromSphericalCoord_Response)(void);

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, dave_interfaces, srv, TransformFromSphericalCoord_Event)(void);

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_dave_interfaces
const rosidl_service_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_introspection_c, dave_interfaces, srv, TransformFromSphericalCoord)(void) {
  if (!dave_interfaces__srv__detail__transform_from_spherical_coord__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_service_type_support_handle.typesupport_identifier) {
    dave_interfaces__srv__detail__transform_from_spherical_coord__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_service_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  rosidl_typesupport_introspection_c__ServiceMembers * service_members =
    (rosidl_typesupport_introspection_c__ServiceMembers *)dave_interfaces__srv__detail__transform_from_spherical_coord__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_service_type_support_handle.data;

  if (!service_members->request_members_) {
    service_members->request_members_ =
      (const rosidl_typesupport_introspection_c__MessageMembers *)
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, dave_interfaces, srv, TransformFromSphericalCoord_Request)()->data;
  }
  if (!service_members->response_members_) {
    service_members->response_members_ =
      (const rosidl_typesupport_introspection_c__MessageMembers *)
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, dave_interfaces, srv, TransformFromSphericalCoord_Response)()->data;
  }
  if (!service_members->event_members_) {
    service_members->event_members_ =
      (const rosidl_typesupport_introspection_c__MessageMembers *)
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, dave_interfaces, srv, TransformFromSphericalCoord_Event)()->data;
  }

  return &dave_interfaces__srv__detail__transform_from_spherical_coord__rosidl_typesupport_introspection_c__TransformFromSphericalCoord_service_type_support_handle;
}
