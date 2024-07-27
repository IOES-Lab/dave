// generated from rosidl_generator_c/resource/idl__description.c.em
// with input from dave_interfaces:srv/SetOriginSphericalCoord.idl
// generated code does not contain a copyright notice

#include "dave_interfaces/srv/detail/set_origin_spherical_coord__functions.h"

ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_type_hash_t *
dave_interfaces__srv__SetOriginSphericalCoord__get_type_hash(
  const rosidl_service_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0xf5, 0x5b, 0xbf, 0x14, 0xb4, 0xda, 0x7f, 0x61,
      0x11, 0x10, 0x35, 0x40, 0x6e, 0xe6, 0x45, 0x5c,
      0x14, 0x61, 0x5f, 0x64, 0x25, 0xe7, 0x7d, 0x0a,
      0xc0, 0xc1, 0xa1, 0x1b, 0xd5, 0xcc, 0xad, 0xb4,
    }};
  return &hash;
}

ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_type_hash_t *
dave_interfaces__srv__SetOriginSphericalCoord_Request__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0x7e, 0xe5, 0x5d, 0x9a, 0x19, 0x17, 0xc0, 0xff,
      0x3b, 0x08, 0xd8, 0x70, 0xb2, 0xd7, 0x50, 0x91,
      0xa5, 0x4b, 0x17, 0x15, 0x1e, 0x0d, 0xaf, 0x2c,
      0x5f, 0x45, 0xd0, 0x5f, 0x75, 0xed, 0x62, 0x86,
    }};
  return &hash;
}

ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_type_hash_t *
dave_interfaces__srv__SetOriginSphericalCoord_Response__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0xae, 0xc7, 0xec, 0xc5, 0xde, 0x2d, 0x32, 0x01,
      0x9c, 0x94, 0xde, 0x8a, 0x60, 0x00, 0x49, 0x21,
      0xe0, 0x94, 0xb3, 0x7a, 0x62, 0xbe, 0xe7, 0x96,
      0x2d, 0x1b, 0x20, 0xac, 0xdc, 0x32, 0x67, 0x8a,
    }};
  return &hash;
}

ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_type_hash_t *
dave_interfaces__srv__SetOriginSphericalCoord_Event__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0x6f, 0x6f, 0x27, 0xb3, 0x12, 0x77, 0x84, 0xc7,
      0x13, 0x35, 0x96, 0x98, 0xac, 0x1a, 0xa0, 0x51,
      0x11, 0x3f, 0x6a, 0xbc, 0xb7, 0xa0, 0xa0, 0x2f,
      0x00, 0xc0, 0xfa, 0x23, 0x06, 0xf0, 0x56, 0x13,
    }};
  return &hash;
}

#include <assert.h>
#include <string.h>

// Include directives for referenced types
#include "service_msgs/msg/detail/service_event_info__functions.h"
#include "builtin_interfaces/msg/detail/time__functions.h"

// Hashes for external referenced types
#ifndef NDEBUG
static const rosidl_type_hash_t builtin_interfaces__msg__Time__EXPECTED_HASH = {1, {
    0xb1, 0x06, 0x23, 0x5e, 0x25, 0xa4, 0xc5, 0xed,
    0x35, 0x09, 0x8a, 0xa0, 0xa6, 0x1a, 0x3e, 0xe9,
    0xc9, 0xb1, 0x8d, 0x19, 0x7f, 0x39, 0x8b, 0x0e,
    0x42, 0x06, 0xce, 0xa9, 0xac, 0xf9, 0xc1, 0x97,
  }};
static const rosidl_type_hash_t service_msgs__msg__ServiceEventInfo__EXPECTED_HASH = {1, {
    0x41, 0xbc, 0xbb, 0xe0, 0x7a, 0x75, 0xc9, 0xb5,
    0x2b, 0xc9, 0x6b, 0xfd, 0x5c, 0x24, 0xd7, 0xf0,
    0xfc, 0x0a, 0x08, 0xc0, 0xcb, 0x79, 0x21, 0xb3,
    0x37, 0x3c, 0x57, 0x32, 0x34, 0x5a, 0x6f, 0x45,
  }};
#endif

static char dave_interfaces__srv__SetOriginSphericalCoord__TYPE_NAME[] = "dave_interfaces/srv/SetOriginSphericalCoord";
static char builtin_interfaces__msg__Time__TYPE_NAME[] = "builtin_interfaces/msg/Time";
static char dave_interfaces__srv__SetOriginSphericalCoord_Event__TYPE_NAME[] = "dave_interfaces/srv/SetOriginSphericalCoord_Event";
static char dave_interfaces__srv__SetOriginSphericalCoord_Request__TYPE_NAME[] = "dave_interfaces/srv/SetOriginSphericalCoord_Request";
static char dave_interfaces__srv__SetOriginSphericalCoord_Response__TYPE_NAME[] = "dave_interfaces/srv/SetOriginSphericalCoord_Response";
static char service_msgs__msg__ServiceEventInfo__TYPE_NAME[] = "service_msgs/msg/ServiceEventInfo";

// Define type names, field names, and default values
static char dave_interfaces__srv__SetOriginSphericalCoord__FIELD_NAME__request_message[] = "request_message";
static char dave_interfaces__srv__SetOriginSphericalCoord__FIELD_NAME__response_message[] = "response_message";
static char dave_interfaces__srv__SetOriginSphericalCoord__FIELD_NAME__event_message[] = "event_message";

static rosidl_runtime_c__type_description__Field dave_interfaces__srv__SetOriginSphericalCoord__FIELDS[] = {
  {
    {dave_interfaces__srv__SetOriginSphericalCoord__FIELD_NAME__request_message, 15, 15},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE,
      0,
      0,
      {dave_interfaces__srv__SetOriginSphericalCoord_Request__TYPE_NAME, 51, 51},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__SetOriginSphericalCoord__FIELD_NAME__response_message, 16, 16},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE,
      0,
      0,
      {dave_interfaces__srv__SetOriginSphericalCoord_Response__TYPE_NAME, 52, 52},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__SetOriginSphericalCoord__FIELD_NAME__event_message, 13, 13},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE,
      0,
      0,
      {dave_interfaces__srv__SetOriginSphericalCoord_Event__TYPE_NAME, 49, 49},
    },
    {NULL, 0, 0},
  },
};

static rosidl_runtime_c__type_description__IndividualTypeDescription dave_interfaces__srv__SetOriginSphericalCoord__REFERENCED_TYPE_DESCRIPTIONS[] = {
  {
    {builtin_interfaces__msg__Time__TYPE_NAME, 27, 27},
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__SetOriginSphericalCoord_Event__TYPE_NAME, 49, 49},
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__SetOriginSphericalCoord_Request__TYPE_NAME, 51, 51},
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__SetOriginSphericalCoord_Response__TYPE_NAME, 52, 52},
    {NULL, 0, 0},
  },
  {
    {service_msgs__msg__ServiceEventInfo__TYPE_NAME, 33, 33},
    {NULL, 0, 0},
  },
};

const rosidl_runtime_c__type_description__TypeDescription *
dave_interfaces__srv__SetOriginSphericalCoord__get_type_description(
  const rosidl_service_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {dave_interfaces__srv__SetOriginSphericalCoord__TYPE_NAME, 43, 43},
      {dave_interfaces__srv__SetOriginSphericalCoord__FIELDS, 3, 3},
    },
    {dave_interfaces__srv__SetOriginSphericalCoord__REFERENCED_TYPE_DESCRIPTIONS, 5, 5},
  };
  if (!constructed) {
    assert(0 == memcmp(&builtin_interfaces__msg__Time__EXPECTED_HASH, builtin_interfaces__msg__Time__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[0].fields = builtin_interfaces__msg__Time__get_type_description(NULL)->type_description.fields;
    description.referenced_type_descriptions.data[1].fields = dave_interfaces__srv__SetOriginSphericalCoord_Event__get_type_description(NULL)->type_description.fields;
    description.referenced_type_descriptions.data[2].fields = dave_interfaces__srv__SetOriginSphericalCoord_Request__get_type_description(NULL)->type_description.fields;
    description.referenced_type_descriptions.data[3].fields = dave_interfaces__srv__SetOriginSphericalCoord_Response__get_type_description(NULL)->type_description.fields;
    assert(0 == memcmp(&service_msgs__msg__ServiceEventInfo__EXPECTED_HASH, service_msgs__msg__ServiceEventInfo__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[4].fields = service_msgs__msg__ServiceEventInfo__get_type_description(NULL)->type_description.fields;
    constructed = true;
  }
  return &description;
}
// Define type names, field names, and default values
static char dave_interfaces__srv__SetOriginSphericalCoord_Request__FIELD_NAME__latitude_deg[] = "latitude_deg";
static char dave_interfaces__srv__SetOriginSphericalCoord_Request__FIELD_NAME__longitude_deg[] = "longitude_deg";
static char dave_interfaces__srv__SetOriginSphericalCoord_Request__FIELD_NAME__altitude[] = "altitude";

static rosidl_runtime_c__type_description__Field dave_interfaces__srv__SetOriginSphericalCoord_Request__FIELDS[] = {
  {
    {dave_interfaces__srv__SetOriginSphericalCoord_Request__FIELD_NAME__latitude_deg, 12, 12},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_DOUBLE,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__SetOriginSphericalCoord_Request__FIELD_NAME__longitude_deg, 13, 13},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_DOUBLE,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__SetOriginSphericalCoord_Request__FIELD_NAME__altitude, 8, 8},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_DOUBLE,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
};

const rosidl_runtime_c__type_description__TypeDescription *
dave_interfaces__srv__SetOriginSphericalCoord_Request__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {dave_interfaces__srv__SetOriginSphericalCoord_Request__TYPE_NAME, 51, 51},
      {dave_interfaces__srv__SetOriginSphericalCoord_Request__FIELDS, 3, 3},
    },
    {NULL, 0, 0},
  };
  if (!constructed) {
    constructed = true;
  }
  return &description;
}
// Define type names, field names, and default values
static char dave_interfaces__srv__SetOriginSphericalCoord_Response__FIELD_NAME__success[] = "success";

static rosidl_runtime_c__type_description__Field dave_interfaces__srv__SetOriginSphericalCoord_Response__FIELDS[] = {
  {
    {dave_interfaces__srv__SetOriginSphericalCoord_Response__FIELD_NAME__success, 7, 7},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_BOOLEAN,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
};

const rosidl_runtime_c__type_description__TypeDescription *
dave_interfaces__srv__SetOriginSphericalCoord_Response__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {dave_interfaces__srv__SetOriginSphericalCoord_Response__TYPE_NAME, 52, 52},
      {dave_interfaces__srv__SetOriginSphericalCoord_Response__FIELDS, 1, 1},
    },
    {NULL, 0, 0},
  };
  if (!constructed) {
    constructed = true;
  }
  return &description;
}
// Define type names, field names, and default values
static char dave_interfaces__srv__SetOriginSphericalCoord_Event__FIELD_NAME__info[] = "info";
static char dave_interfaces__srv__SetOriginSphericalCoord_Event__FIELD_NAME__request[] = "request";
static char dave_interfaces__srv__SetOriginSphericalCoord_Event__FIELD_NAME__response[] = "response";

static rosidl_runtime_c__type_description__Field dave_interfaces__srv__SetOriginSphericalCoord_Event__FIELDS[] = {
  {
    {dave_interfaces__srv__SetOriginSphericalCoord_Event__FIELD_NAME__info, 4, 4},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE,
      0,
      0,
      {service_msgs__msg__ServiceEventInfo__TYPE_NAME, 33, 33},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__SetOriginSphericalCoord_Event__FIELD_NAME__request, 7, 7},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE_BOUNDED_SEQUENCE,
      1,
      0,
      {dave_interfaces__srv__SetOriginSphericalCoord_Request__TYPE_NAME, 51, 51},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__SetOriginSphericalCoord_Event__FIELD_NAME__response, 8, 8},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE_BOUNDED_SEQUENCE,
      1,
      0,
      {dave_interfaces__srv__SetOriginSphericalCoord_Response__TYPE_NAME, 52, 52},
    },
    {NULL, 0, 0},
  },
};

static rosidl_runtime_c__type_description__IndividualTypeDescription dave_interfaces__srv__SetOriginSphericalCoord_Event__REFERENCED_TYPE_DESCRIPTIONS[] = {
  {
    {builtin_interfaces__msg__Time__TYPE_NAME, 27, 27},
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__SetOriginSphericalCoord_Request__TYPE_NAME, 51, 51},
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__SetOriginSphericalCoord_Response__TYPE_NAME, 52, 52},
    {NULL, 0, 0},
  },
  {
    {service_msgs__msg__ServiceEventInfo__TYPE_NAME, 33, 33},
    {NULL, 0, 0},
  },
};

const rosidl_runtime_c__type_description__TypeDescription *
dave_interfaces__srv__SetOriginSphericalCoord_Event__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {dave_interfaces__srv__SetOriginSphericalCoord_Event__TYPE_NAME, 49, 49},
      {dave_interfaces__srv__SetOriginSphericalCoord_Event__FIELDS, 3, 3},
    },
    {dave_interfaces__srv__SetOriginSphericalCoord_Event__REFERENCED_TYPE_DESCRIPTIONS, 4, 4},
  };
  if (!constructed) {
    assert(0 == memcmp(&builtin_interfaces__msg__Time__EXPECTED_HASH, builtin_interfaces__msg__Time__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[0].fields = builtin_interfaces__msg__Time__get_type_description(NULL)->type_description.fields;
    description.referenced_type_descriptions.data[1].fields = dave_interfaces__srv__SetOriginSphericalCoord_Request__get_type_description(NULL)->type_description.fields;
    description.referenced_type_descriptions.data[2].fields = dave_interfaces__srv__SetOriginSphericalCoord_Response__get_type_description(NULL)->type_description.fields;
    assert(0 == memcmp(&service_msgs__msg__ServiceEventInfo__EXPECTED_HASH, service_msgs__msg__ServiceEventInfo__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[3].fields = service_msgs__msg__ServiceEventInfo__get_type_description(NULL)->type_description.fields;
    constructed = true;
  }
  return &description;
}

static char toplevel_type_raw_source[] =
  "# Copyright (c) 2016 The dave Simulator Authors.\n"
  "# All rights reserved.\n"
  "#\n"
  "# Licensed under the Apache License, Version 2.0 (the \"License\");\n"
  "# you may not use this file except in compliance with the License.\n"
  "# You may obtain a copy of the License at\n"
  "#\n"
  "#     http://www.apache.org/licenses/LICENSE-2.0\n"
  "#\n"
  "# Unless required by applicable law or agreed to in writing, software\n"
  "# distributed under the License is distributed on an \"AS IS\" BASIS,\n"
  "# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
  "# See the License for the specific language governing permissions and\n"
  "# limitations under the License.\n"
  "\n"
  "# Latitude [degrees]. Positive is north of equator; negative is south.\n"
  "float64 latitude_deg\n"
  "# Longitude [degrees]. Positive is east of prime meridian; negative is west.\n"
  "float64 longitude_deg\n"
  "# Altitude [m]. Positive is above the WGS 84 ellipsoid\n"
  "float64 altitude\n"
  "---\n"
  "bool success";

static char srv_encoding[] = "srv";
static char implicit_encoding[] = "implicit";

// Define all individual source functions

const rosidl_runtime_c__type_description__TypeSource *
dave_interfaces__srv__SetOriginSphericalCoord__get_individual_type_description_source(
  const rosidl_service_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {dave_interfaces__srv__SetOriginSphericalCoord__TYPE_NAME, 43, 43},
    {srv_encoding, 3, 3},
    {toplevel_type_raw_source, 898, 898},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource *
dave_interfaces__srv__SetOriginSphericalCoord_Request__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {dave_interfaces__srv__SetOriginSphericalCoord_Request__TYPE_NAME, 51, 51},
    {implicit_encoding, 8, 8},
    {NULL, 0, 0},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource *
dave_interfaces__srv__SetOriginSphericalCoord_Response__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {dave_interfaces__srv__SetOriginSphericalCoord_Response__TYPE_NAME, 52, 52},
    {implicit_encoding, 8, 8},
    {NULL, 0, 0},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource *
dave_interfaces__srv__SetOriginSphericalCoord_Event__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {dave_interfaces__srv__SetOriginSphericalCoord_Event__TYPE_NAME, 49, 49},
    {implicit_encoding, 8, 8},
    {NULL, 0, 0},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
dave_interfaces__srv__SetOriginSphericalCoord__get_type_description_sources(
  const rosidl_service_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[6];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 6, 6};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *dave_interfaces__srv__SetOriginSphericalCoord__get_individual_type_description_source(NULL),
    sources[1] = *builtin_interfaces__msg__Time__get_individual_type_description_source(NULL);
    sources[2] = *dave_interfaces__srv__SetOriginSphericalCoord_Event__get_individual_type_description_source(NULL);
    sources[3] = *dave_interfaces__srv__SetOriginSphericalCoord_Request__get_individual_type_description_source(NULL);
    sources[4] = *dave_interfaces__srv__SetOriginSphericalCoord_Response__get_individual_type_description_source(NULL);
    sources[5] = *service_msgs__msg__ServiceEventInfo__get_individual_type_description_source(NULL);
    constructed = true;
  }
  return &source_sequence;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
dave_interfaces__srv__SetOriginSphericalCoord_Request__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[1];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 1, 1};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *dave_interfaces__srv__SetOriginSphericalCoord_Request__get_individual_type_description_source(NULL),
    constructed = true;
  }
  return &source_sequence;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
dave_interfaces__srv__SetOriginSphericalCoord_Response__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[1];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 1, 1};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *dave_interfaces__srv__SetOriginSphericalCoord_Response__get_individual_type_description_source(NULL),
    constructed = true;
  }
  return &source_sequence;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
dave_interfaces__srv__SetOriginSphericalCoord_Event__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[5];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 5, 5};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *dave_interfaces__srv__SetOriginSphericalCoord_Event__get_individual_type_description_source(NULL),
    sources[1] = *builtin_interfaces__msg__Time__get_individual_type_description_source(NULL);
    sources[2] = *dave_interfaces__srv__SetOriginSphericalCoord_Request__get_individual_type_description_source(NULL);
    sources[3] = *dave_interfaces__srv__SetOriginSphericalCoord_Response__get_individual_type_description_source(NULL);
    sources[4] = *service_msgs__msg__ServiceEventInfo__get_individual_type_description_source(NULL);
    constructed = true;
  }
  return &source_sequence;
}
