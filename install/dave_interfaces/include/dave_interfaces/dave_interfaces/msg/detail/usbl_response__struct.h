// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from dave_interfaces:msg/UsblResponse.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "dave_interfaces/msg/usbl_response.h"


#ifndef DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__STRUCT_H_
#define DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

// Include directives for member types
// Member 'data'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/UsblResponse in the package dave_interfaces.
typedef struct dave_interfaces__msg__UsblResponse
{
  int32_t transceiver_id;
  int32_t response_id;
  rosidl_runtime_c__String data;
} dave_interfaces__msg__UsblResponse;

// Struct for a sequence of dave_interfaces__msg__UsblResponse.
typedef struct dave_interfaces__msg__UsblResponse__Sequence
{
  dave_interfaces__msg__UsblResponse * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} dave_interfaces__msg__UsblResponse__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__STRUCT_H_
