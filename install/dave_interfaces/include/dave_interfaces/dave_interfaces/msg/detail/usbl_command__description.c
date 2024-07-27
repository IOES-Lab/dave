// generated from rosidl_generator_c/resource/idl__description.c.em
// with input from dave_interfaces:msg/UsblCommand.idl
// generated code does not contain a copyright notice

#include "dave_interfaces/msg/detail/usbl_command__functions.h"

ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_type_hash_t *
dave_interfaces__msg__UsblCommand__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0x1d, 0x80, 0xfe, 0xd4, 0xff, 0xcb, 0x86, 0x51,
      0xb4, 0x0f, 0x00, 0xef, 0xb8, 0x09, 0xa1, 0x8a,
      0xfd, 0x54, 0x99, 0x78, 0xec, 0x57, 0x15, 0x19,
      0x1e, 0x9b, 0x48, 0xe3, 0x5f, 0x42, 0x39, 0x3c,
    }};
  return &hash;
}

#include <assert.h>
#include <string.h>

// Include directives for referenced types

// Hashes for external referenced types
#ifndef NDEBUG
#endif

static char dave_interfaces__msg__UsblCommand__TYPE_NAME[] = "dave_interfaces/msg/UsblCommand";

// Define type names, field names, and default values
static char dave_interfaces__msg__UsblCommand__FIELD_NAME__transponder_id[] = "transponder_id";
static char dave_interfaces__msg__UsblCommand__FIELD_NAME__command_id[] = "command_id";
static char dave_interfaces__msg__UsblCommand__FIELD_NAME__data[] = "data";

static rosidl_runtime_c__type_description__Field dave_interfaces__msg__UsblCommand__FIELDS[] = {
  {
    {dave_interfaces__msg__UsblCommand__FIELD_NAME__transponder_id, 14, 14},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_INT32,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__msg__UsblCommand__FIELD_NAME__command_id, 10, 10},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_INT32,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__msg__UsblCommand__FIELD_NAME__data, 4, 4},
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
dave_interfaces__msg__UsblCommand__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {dave_interfaces__msg__UsblCommand__TYPE_NAME, 31, 31},
      {dave_interfaces__msg__UsblCommand__FIELDS, 3, 3},
    },
    {NULL, 0, 0},
  };
  if (!constructed) {
    constructed = true;
  }
  return &description;
}

static char toplevel_type_raw_source[] =
  "int32 transponder_id\n"
  "int32 command_id\n"
  "string data";

static char msg_encoding[] = "msg";

// Define all individual source functions

const rosidl_runtime_c__type_description__TypeSource *
dave_interfaces__msg__UsblCommand__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {dave_interfaces__msg__UsblCommand__TYPE_NAME, 31, 31},
    {msg_encoding, 3, 3},
    {toplevel_type_raw_source, 49, 49},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
dave_interfaces__msg__UsblCommand__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[1];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 1, 1};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *dave_interfaces__msg__UsblCommand__get_individual_type_description_source(NULL),
    constructed = true;
  }
  return &source_sequence;
}
