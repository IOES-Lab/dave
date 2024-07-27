// generated from rosidl_generator_c/resource/idl__description.c.em
// with input from dave_interfaces:msg/UsblResponse.idl
// generated code does not contain a copyright notice

#include "dave_interfaces/msg/detail/usbl_response__functions.h"

ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_type_hash_t *
dave_interfaces__msg__UsblResponse__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0xde, 0x3e, 0x3f, 0x7c, 0xc1, 0x17, 0x71, 0x88,
      0xb1, 0xce, 0xa1, 0xc7, 0x2d, 0x52, 0x72, 0x30,
      0x70, 0x08, 0x1c, 0x4d, 0x20, 0xb5, 0x87, 0x1e,
      0x2d, 0x66, 0x66, 0x6e, 0x87, 0x51, 0x3c, 0x15,
    }};
  return &hash;
}

#include <assert.h>
#include <string.h>

// Include directives for referenced types

// Hashes for external referenced types
#ifndef NDEBUG
#endif

static char dave_interfaces__msg__UsblResponse__TYPE_NAME[] = "dave_interfaces/msg/UsblResponse";

// Define type names, field names, and default values
static char dave_interfaces__msg__UsblResponse__FIELD_NAME__transceiver_id[] = "transceiver_id";
static char dave_interfaces__msg__UsblResponse__FIELD_NAME__response_id[] = "response_id";
static char dave_interfaces__msg__UsblResponse__FIELD_NAME__data[] = "data";

static rosidl_runtime_c__type_description__Field dave_interfaces__msg__UsblResponse__FIELDS[] = {
  {
    {dave_interfaces__msg__UsblResponse__FIELD_NAME__transceiver_id, 14, 14},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_INT32,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__msg__UsblResponse__FIELD_NAME__response_id, 11, 11},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_INT32,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__msg__UsblResponse__FIELD_NAME__data, 4, 4},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_STRING,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
};

const rosidl_runtime_c__type_description__TypeDescription *
dave_interfaces__msg__UsblResponse__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {dave_interfaces__msg__UsblResponse__TYPE_NAME, 32, 32},
      {dave_interfaces__msg__UsblResponse__FIELDS, 3, 3},
    },
    {NULL, 0, 0},
  };
  if (!constructed) {
    constructed = true;
  }
  return &description;
}

static char toplevel_type_raw_source[] =
  "int32 transceiver_id\n"
  "int32 response_id\n"
  "string data";

static char msg_encoding[] = "msg";

// Define all individual source functions

const rosidl_runtime_c__type_description__TypeSource *
dave_interfaces__msg__UsblResponse__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {dave_interfaces__msg__UsblResponse__TYPE_NAME, 32, 32},
    {msg_encoding, 3, 3},
    {toplevel_type_raw_source, 50, 50},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
dave_interfaces__msg__UsblResponse__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[1];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 1, 1};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *dave_interfaces__msg__UsblResponse__get_individual_type_description_source(NULL),
    constructed = true;
  }
  return &source_sequence;
}
