// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from dave_interfaces:msg/Location.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "dave_interfaces/msg/location.h"


#ifndef DAVE_INTERFACES__MSG__DETAIL__LOCATION__STRUCT_H_
#define DAVE_INTERFACES__MSG__DETAIL__LOCATION__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

/// Struct defined in msg/Location in the package dave_interfaces.
typedef struct dave_interfaces__msg__Location
{
  int32_t transponder_id;
  double x;
  double y;
  double z;
} dave_interfaces__msg__Location;

// Struct for a sequence of dave_interfaces__msg__Location.
typedef struct dave_interfaces__msg__Location__Sequence
{
  dave_interfaces__msg__Location * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} dave_interfaces__msg__Location__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // DAVE_INTERFACES__MSG__DETAIL__LOCATION__STRUCT_H_
