// generated from rosidl_generator_c/resource/idl__description.c.em
// with input from dave_interfaces:srv/TransformToSphericalCoord.idl
// generated code does not contain a copyright notice

#include "dave_interfaces/srv/detail/transform_to_spherical_coord__functions.h"

ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_type_hash_t *
dave_interfaces__srv__TransformToSphericalCoord__get_type_hash(
  const rosidl_service_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0xb9, 0x15, 0x71, 0x90, 0x84, 0x19, 0xb9, 0xa6,
      0xb5, 0x9d, 0xa1, 0x1c, 0x0b, 0x06, 0xc4, 0xc0,
      0xbf, 0x5d, 0x9a, 0x97, 0x77, 0xe3, 0x91, 0x0a,
      0x8a, 0x0c, 0x9f, 0x3c, 0x5c, 0xd7, 0x18, 0xc0,
    }};
  return &hash;
}

ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_type_hash_t *
dave_interfaces__srv__TransformToSphericalCoord_Request__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0x9b, 0x0f, 0xd8, 0xe4, 0xa3, 0xfb, 0x8f, 0x38,
      0xc2, 0xa3, 0xc5, 0xbd, 0x5c, 0x88, 0x10, 0xaa,
      0x67, 0x77, 0x32, 0xe7, 0xb0, 0x40, 0x97, 0x03,
      0xba, 0x21, 0x1d, 0x68, 0xb2, 0x21, 0x92, 0x98,
    }};
  return &hash;
}

ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_type_hash_t *
dave_interfaces__srv__TransformToSphericalCoord_Response__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0x52, 0xbe, 0x93, 0x9b, 0x85, 0x17, 0xbd, 0x28,
      0x15, 0x66, 0x05, 0xdc, 0x0d, 0x9f, 0xf9, 0x40,
      0xcf, 0xc7, 0x8a, 0xfc, 0x43, 0x24, 0x9e, 0x38,
      0x51, 0x5e, 0x68, 0xb4, 0xcb, 0x22, 0x9b, 0x7d,
    }};
  return &hash;
}

ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_type_hash_t *
dave_interfaces__srv__TransformToSphericalCoord_Event__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0x36, 0x83, 0xba, 0x96, 0x84, 0x86, 0xf3, 0x47,
      0x13, 0xa8, 0x3a, 0x0d, 0x20, 0xaa, 0x10, 0x65,
      0xdd, 0xfa, 0x1f, 0xbd, 0xf3, 0x6b, 0xcd, 0xd0,
      0x96, 0x19, 0xd3, 0xf7, 0xa3, 0x5b, 0x2e, 0xad,
    }};
  return &hash;
}

#include <assert.h>
#include <string.h>

// Include directives for referenced types
#include "geometry_msgs/msg/detail/vector3__functions.h"
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
static const rosidl_type_hash_t geometry_msgs__msg__Vector3__EXPECTED_HASH = {1, {
    0xcc, 0x12, 0xfe, 0x83, 0xe4, 0xc0, 0x27, 0x19,
    0xf1, 0xce, 0x80, 0x70, 0xbf, 0xd1, 0x4a, 0xec,
    0xd4, 0x0f, 0x75, 0xa9, 0x66, 0x96, 0xa6, 0x7a,
    0x2a, 0x1f, 0x37, 0xf7, 0xdb, 0xb0, 0x76, 0x5d,
  }};
static const rosidl_type_hash_t service_msgs__msg__ServiceEventInfo__EXPECTED_HASH = {1, {
    0x41, 0xbc, 0xbb, 0xe0, 0x7a, 0x75, 0xc9, 0xb5,
    0x2b, 0xc9, 0x6b, 0xfd, 0x5c, 0x24, 0xd7, 0xf0,
    0xfc, 0x0a, 0x08, 0xc0, 0xcb, 0x79, 0x21, 0xb3,
    0x37, 0x3c, 0x57, 0x32, 0x34, 0x5a, 0x6f, 0x45,
  }};
#endif

static char dave_interfaces__srv__TransformToSphericalCoord__TYPE_NAME[] = "dave_interfaces/srv/TransformToSphericalCoord";
static char builtin_interfaces__msg__Time__TYPE_NAME[] = "builtin_interfaces/msg/Time";
static char dave_interfaces__srv__TransformToSphericalCoord_Event__TYPE_NAME[] = "dave_interfaces/srv/TransformToSphericalCoord_Event";
static char dave_interfaces__srv__TransformToSphericalCoord_Request__TYPE_NAME[] = "dave_interfaces/srv/TransformToSphericalCoord_Request";
static char dave_interfaces__srv__TransformToSphericalCoord_Response__TYPE_NAME[] = "dave_interfaces/srv/TransformToSphericalCoord_Response";
static char geometry_msgs__msg__Vector3__TYPE_NAME[] = "geometry_msgs/msg/Vector3";
static char service_msgs__msg__ServiceEventInfo__TYPE_NAME[] = "service_msgs/msg/ServiceEventInfo";

// Define type names, field names, and default values
static char dave_interfaces__srv__TransformToSphericalCoord__FIELD_NAME__request_message[] = "request_message";
static char dave_interfaces__srv__TransformToSphericalCoord__FIELD_NAME__response_message[] = "response_message";
static char dave_interfaces__srv__TransformToSphericalCoord__FIELD_NAME__event_message[] = "event_message";

static rosidl_runtime_c__type_description__Field dave_interfaces__srv__TransformToSphericalCoord__FIELDS[] = {
  {
    {dave_interfaces__srv__TransformToSphericalCoord__FIELD_NAME__request_message, 15, 15},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE,
      0,
      0,
      {dave_interfaces__srv__TransformToSphericalCoord_Request__TYPE_NAME, 53, 53},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__TransformToSphericalCoord__FIELD_NAME__response_message, 16, 16},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE,
      0,
      0,
      {dave_interfaces__srv__TransformToSphericalCoord_Response__TYPE_NAME, 54, 54},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__TransformToSphericalCoord__FIELD_NAME__event_message, 13, 13},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE,
      0,
      0,
      {dave_interfaces__srv__TransformToSphericalCoord_Event__TYPE_NAME, 51, 51},
    },
    {NULL, 0, 0},
  },
};

static rosidl_runtime_c__type_description__IndividualTypeDescription dave_interfaces__srv__TransformToSphericalCoord__REFERENCED_TYPE_DESCRIPTIONS[] = {
  {
    {builtin_interfaces__msg__Time__TYPE_NAME, 27, 27},
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__TransformToSphericalCoord_Event__TYPE_NAME, 51, 51},
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__TransformToSphericalCoord_Request__TYPE_NAME, 53, 53},
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__TransformToSphericalCoord_Response__TYPE_NAME, 54, 54},
    {NULL, 0, 0},
  },
  {
    {geometry_msgs__msg__Vector3__TYPE_NAME, 25, 25},
    {NULL, 0, 0},
  },
  {
    {service_msgs__msg__ServiceEventInfo__TYPE_NAME, 33, 33},
    {NULL, 0, 0},
  },
};

const rosidl_runtime_c__type_description__TypeDescription *
dave_interfaces__srv__TransformToSphericalCoord__get_type_description(
  const rosidl_service_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {dave_interfaces__srv__TransformToSphericalCoord__TYPE_NAME, 45, 45},
      {dave_interfaces__srv__TransformToSphericalCoord__FIELDS, 3, 3},
    },
    {dave_interfaces__srv__TransformToSphericalCoord__REFERENCED_TYPE_DESCRIPTIONS, 6, 6},
  };
  if (!constructed) {
    assert(0 == memcmp(&builtin_interfaces__msg__Time__EXPECTED_HASH, builtin_interfaces__msg__Time__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[0].fields = builtin_interfaces__msg__Time__get_type_description(NULL)->type_description.fields;
    description.referenced_type_descriptions.data[1].fields = dave_interfaces__srv__TransformToSphericalCoord_Event__get_type_description(NULL)->type_description.fields;
    description.referenced_type_descriptions.data[2].fields = dave_interfaces__srv__TransformToSphericalCoord_Request__get_type_description(NULL)->type_description.fields;
    description.referenced_type_descriptions.data[3].fields = dave_interfaces__srv__TransformToSphericalCoord_Response__get_type_description(NULL)->type_description.fields;
    assert(0 == memcmp(&geometry_msgs__msg__Vector3__EXPECTED_HASH, geometry_msgs__msg__Vector3__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[4].fields = geometry_msgs__msg__Vector3__get_type_description(NULL)->type_description.fields;
    assert(0 == memcmp(&service_msgs__msg__ServiceEventInfo__EXPECTED_HASH, service_msgs__msg__ServiceEventInfo__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[5].fields = service_msgs__msg__ServiceEventInfo__get_type_description(NULL)->type_description.fields;
    constructed = true;
  }
  return &description;
}
// Define type names, field names, and default values
static char dave_interfaces__srv__TransformToSphericalCoord_Request__FIELD_NAME__input[] = "input";

static rosidl_runtime_c__type_description__Field dave_interfaces__srv__TransformToSphericalCoord_Request__FIELDS[] = {
  {
    {dave_interfaces__srv__TransformToSphericalCoord_Request__FIELD_NAME__input, 5, 5},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE,
      0,
      0,
      {geometry_msgs__msg__Vector3__TYPE_NAME, 25, 25},
    },
    {NULL, 0, 0},
  },
};

static rosidl_runtime_c__type_description__IndividualTypeDescription dave_interfaces__srv__TransformToSphericalCoord_Request__REFERENCED_TYPE_DESCRIPTIONS[] = {
  {
    {geometry_msgs__msg__Vector3__TYPE_NAME, 25, 25},
    {NULL, 0, 0},
  },
};

const rosidl_runtime_c__type_description__TypeDescription *
dave_interfaces__srv__TransformToSphericalCoord_Request__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {dave_interfaces__srv__TransformToSphericalCoord_Request__TYPE_NAME, 53, 53},
      {dave_interfaces__srv__TransformToSphericalCoord_Request__FIELDS, 1, 1},
    },
    {dave_interfaces__srv__TransformToSphericalCoord_Request__REFERENCED_TYPE_DESCRIPTIONS, 1, 1},
  };
  if (!constructed) {
    assert(0 == memcmp(&geometry_msgs__msg__Vector3__EXPECTED_HASH, geometry_msgs__msg__Vector3__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[0].fields = geometry_msgs__msg__Vector3__get_type_description(NULL)->type_description.fields;
    constructed = true;
  }
  return &description;
}
// Define type names, field names, and default values
static char dave_interfaces__srv__TransformToSphericalCoord_Response__FIELD_NAME__latitude_deg[] = "latitude_deg";
static char dave_interfaces__srv__TransformToSphericalCoord_Response__FIELD_NAME__longitude_deg[] = "longitude_deg";
static char dave_interfaces__srv__TransformToSphericalCoord_Response__FIELD_NAME__altitude[] = "altitude";

static rosidl_runtime_c__type_description__Field dave_interfaces__srv__TransformToSphericalCoord_Response__FIELDS[] = {
  {
    {dave_interfaces__srv__TransformToSphericalCoord_Response__FIELD_NAME__latitude_deg, 12, 12},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_DOUBLE,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__TransformToSphericalCoord_Response__FIELD_NAME__longitude_deg, 13, 13},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_DOUBLE,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__TransformToSphericalCoord_Response__FIELD_NAME__altitude, 8, 8},
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
dave_interfaces__srv__TransformToSphericalCoord_Response__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {dave_interfaces__srv__TransformToSphericalCoord_Response__TYPE_NAME, 54, 54},
      {dave_interfaces__srv__TransformToSphericalCoord_Response__FIELDS, 3, 3},
    },
    {NULL, 0, 0},
  };
  if (!constructed) {
    constructed = true;
  }
  return &description;
}
// Define type names, field names, and default values
static char dave_interfaces__srv__TransformToSphericalCoord_Event__FIELD_NAME__info[] = "info";
static char dave_interfaces__srv__TransformToSphericalCoord_Event__FIELD_NAME__request[] = "request";
static char dave_interfaces__srv__TransformToSphericalCoord_Event__FIELD_NAME__response[] = "response";

static rosidl_runtime_c__type_description__Field dave_interfaces__srv__TransformToSphericalCoord_Event__FIELDS[] = {
  {
    {dave_interfaces__srv__TransformToSphericalCoord_Event__FIELD_NAME__info, 4, 4},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE,
      0,
      0,
      {service_msgs__msg__ServiceEventInfo__TYPE_NAME, 33, 33},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__TransformToSphericalCoord_Event__FIELD_NAME__request, 7, 7},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE_BOUNDED_SEQUENCE,
      1,
      0,
      {dave_interfaces__srv__TransformToSphericalCoord_Request__TYPE_NAME, 53, 53},
    },
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__TransformToSphericalCoord_Event__FIELD_NAME__response, 8, 8},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE_BOUNDED_SEQUENCE,
      1,
      0,
      {dave_interfaces__srv__TransformToSphericalCoord_Response__TYPE_NAME, 54, 54},
    },
    {NULL, 0, 0},
  },
};

static rosidl_runtime_c__type_description__IndividualTypeDescription dave_interfaces__srv__TransformToSphericalCoord_Event__REFERENCED_TYPE_DESCRIPTIONS[] = {
  {
    {builtin_interfaces__msg__Time__TYPE_NAME, 27, 27},
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__TransformToSphericalCoord_Request__TYPE_NAME, 53, 53},
    {NULL, 0, 0},
  },
  {
    {dave_interfaces__srv__TransformToSphericalCoord_Response__TYPE_NAME, 54, 54},
    {NULL, 0, 0},
  },
  {
    {geometry_msgs__msg__Vector3__TYPE_NAME, 25, 25},
    {NULL, 0, 0},
  },
  {
    {service_msgs__msg__ServiceEventInfo__TYPE_NAME, 33, 33},
    {NULL, 0, 0},
  },
};

const rosidl_runtime_c__type_description__TypeDescription *
dave_interfaces__srv__TransformToSphericalCoord_Event__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {dave_interfaces__srv__TransformToSphericalCoord_Event__TYPE_NAME, 51, 51},
      {dave_interfaces__srv__TransformToSphericalCoord_Event__FIELDS, 3, 3},
    },
    {dave_interfaces__srv__TransformToSphericalCoord_Event__REFERENCED_TYPE_DESCRIPTIONS, 5, 5},
  };
  if (!constructed) {
    assert(0 == memcmp(&builtin_interfaces__msg__Time__EXPECTED_HASH, builtin_interfaces__msg__Time__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[0].fields = builtin_interfaces__msg__Time__get_type_description(NULL)->type_description.fields;
    description.referenced_type_descriptions.data[1].fields = dave_interfaces__srv__TransformToSphericalCoord_Request__get_type_description(NULL)->type_description.fields;
    description.referenced_type_descriptions.data[2].fields = dave_interfaces__srv__TransformToSphericalCoord_Response__get_type_description(NULL)->type_description.fields;
    assert(0 == memcmp(&geometry_msgs__msg__Vector3__EXPECTED_HASH, geometry_msgs__msg__Vector3__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[3].fields = geometry_msgs__msg__Vector3__get_type_description(NULL)->type_description.fields;
    assert(0 == memcmp(&service_msgs__msg__ServiceEventInfo__EXPECTED_HASH, service_msgs__msg__ServiceEventInfo__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[4].fields = service_msgs__msg__ServiceEventInfo__get_type_description(NULL)->type_description.fields;
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
  "geometry_msgs/Vector3 input\n"
  "---\n"
  "# Latitude [degrees]. Positive is north of equator; negative is south.\n"
  "float64 latitude_deg\n"
  "# Longitude [degrees]. Positive is east of prime meridian; negative is west.\n"
  "float64 longitude_deg\n"
  "# Altitude [m]. Positive is above the WGS 84 ellipsoid\n"
  "float64 altitude";

static char srv_encoding[] = "srv";
static char implicit_encoding[] = "implicit";

// Define all individual source functions

const rosidl_runtime_c__type_description__TypeSource *
dave_interfaces__srv__TransformToSphericalCoord__get_individual_type_description_source(
  const rosidl_service_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {dave_interfaces__srv__TransformToSphericalCoord__TYPE_NAME, 45, 45},
    {srv_encoding, 3, 3},
    {toplevel_type_raw_source, 913, 913},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource *
dave_interfaces__srv__TransformToSphericalCoord_Request__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {dave_interfaces__srv__TransformToSphericalCoord_Request__TYPE_NAME, 53, 53},
    {implicit_encoding, 8, 8},
    {NULL, 0, 0},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource *
dave_interfaces__srv__TransformToSphericalCoord_Response__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {dave_interfaces__srv__TransformToSphericalCoord_Response__TYPE_NAME, 54, 54},
    {implicit_encoding, 8, 8},
    {NULL, 0, 0},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource *
dave_interfaces__srv__TransformToSphericalCoord_Event__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {dave_interfaces__srv__TransformToSphericalCoord_Event__TYPE_NAME, 51, 51},
    {implicit_encoding, 8, 8},
    {NULL, 0, 0},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
dave_interfaces__srv__TransformToSphericalCoord__get_type_description_sources(
  const rosidl_service_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[7];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 7, 7};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *dave_interfaces__srv__TransformToSphericalCoord__get_individual_type_description_source(NULL),
    sources[1] = *builtin_interfaces__msg__Time__get_individual_type_description_source(NULL);
    sources[2] = *dave_interfaces__srv__TransformToSphericalCoord_Event__get_individual_type_description_source(NULL);
    sources[3] = *dave_interfaces__srv__TransformToSphericalCoord_Request__get_individual_type_description_source(NULL);
    sources[4] = *dave_interfaces__srv__TransformToSphericalCoord_Response__get_individual_type_description_source(NULL);
    sources[5] = *geometry_msgs__msg__Vector3__get_individual_type_description_source(NULL);
    sources[6] = *service_msgs__msg__ServiceEventInfo__get_individual_type_description_source(NULL);
    constructed = true;
  }
  return &source_sequence;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
dave_interfaces__srv__TransformToSphericalCoord_Request__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[2];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 2, 2};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *dave_interfaces__srv__TransformToSphericalCoord_Request__get_individual_type_description_source(NULL),
    sources[1] = *geometry_msgs__msg__Vector3__get_individual_type_description_source(NULL);
    constructed = true;
  }
  return &source_sequence;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
dave_interfaces__srv__TransformToSphericalCoord_Response__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[1];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 1, 1};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *dave_interfaces__srv__TransformToSphericalCoord_Response__get_individual_type_description_source(NULL),
    constructed = true;
  }
  return &source_sequence;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
dave_interfaces__srv__TransformToSphericalCoord_Event__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[6];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 6, 6};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *dave_interfaces__srv__TransformToSphericalCoord_Event__get_individual_type_description_source(NULL),
    sources[1] = *builtin_interfaces__msg__Time__get_individual_type_description_source(NULL);
    sources[2] = *dave_interfaces__srv__TransformToSphericalCoord_Request__get_individual_type_description_source(NULL);
    sources[3] = *dave_interfaces__srv__TransformToSphericalCoord_Response__get_individual_type_description_source(NULL);
    sources[4] = *geometry_msgs__msg__Vector3__get_individual_type_description_source(NULL);
    sources[5] = *service_msgs__msg__ServiceEventInfo__get_individual_type_description_source(NULL);
    constructed = true;
  }
  return &source_sequence;
}
