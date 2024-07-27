// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from dave_interfaces:srv/TransformToSphericalCoord.idl
// generated code does not contain a copyright notice
#include "dave_interfaces/srv/detail/transform_to_spherical_coord__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"

// Include directives for member types
// Member `input`
#include "geometry_msgs/msg/detail/vector3__functions.h"

bool
dave_interfaces__srv__TransformToSphericalCoord_Request__init(dave_interfaces__srv__TransformToSphericalCoord_Request * msg)
{
  if (!msg) {
    return false;
  }
  // input
  if (!geometry_msgs__msg__Vector3__init(&msg->input)) {
    dave_interfaces__srv__TransformToSphericalCoord_Request__fini(msg);
    return false;
  }
  return true;
}

void
dave_interfaces__srv__TransformToSphericalCoord_Request__fini(dave_interfaces__srv__TransformToSphericalCoord_Request * msg)
{
  if (!msg) {
    return;
  }
  // input
  geometry_msgs__msg__Vector3__fini(&msg->input);
}

bool
dave_interfaces__srv__TransformToSphericalCoord_Request__are_equal(const dave_interfaces__srv__TransformToSphericalCoord_Request * lhs, const dave_interfaces__srv__TransformToSphericalCoord_Request * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // input
  if (!geometry_msgs__msg__Vector3__are_equal(
      &(lhs->input), &(rhs->input)))
  {
    return false;
  }
  return true;
}

bool
dave_interfaces__srv__TransformToSphericalCoord_Request__copy(
  const dave_interfaces__srv__TransformToSphericalCoord_Request * input,
  dave_interfaces__srv__TransformToSphericalCoord_Request * output)
{
  if (!input || !output) {
    return false;
  }
  // input
  if (!geometry_msgs__msg__Vector3__copy(
      &(input->input), &(output->input)))
  {
    return false;
  }
  return true;
}

dave_interfaces__srv__TransformToSphericalCoord_Request *
dave_interfaces__srv__TransformToSphericalCoord_Request__create(void)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__TransformToSphericalCoord_Request * msg = (dave_interfaces__srv__TransformToSphericalCoord_Request *)allocator.allocate(sizeof(dave_interfaces__srv__TransformToSphericalCoord_Request), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(dave_interfaces__srv__TransformToSphericalCoord_Request));
  bool success = dave_interfaces__srv__TransformToSphericalCoord_Request__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
dave_interfaces__srv__TransformToSphericalCoord_Request__destroy(dave_interfaces__srv__TransformToSphericalCoord_Request * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    dave_interfaces__srv__TransformToSphericalCoord_Request__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence__init(dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__TransformToSphericalCoord_Request * data = NULL;

  if (size) {
    data = (dave_interfaces__srv__TransformToSphericalCoord_Request *)allocator.zero_allocate(size, sizeof(dave_interfaces__srv__TransformToSphericalCoord_Request), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = dave_interfaces__srv__TransformToSphericalCoord_Request__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        dave_interfaces__srv__TransformToSphericalCoord_Request__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence__fini(dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      dave_interfaces__srv__TransformToSphericalCoord_Request__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence *
dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence * array = (dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence *)allocator.allocate(sizeof(dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence__destroy(dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence__are_equal(const dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence * lhs, const dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!dave_interfaces__srv__TransformToSphericalCoord_Request__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence__copy(
  const dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence * input,
  dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(dave_interfaces__srv__TransformToSphericalCoord_Request);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    dave_interfaces__srv__TransformToSphericalCoord_Request * data =
      (dave_interfaces__srv__TransformToSphericalCoord_Request *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!dave_interfaces__srv__TransformToSphericalCoord_Request__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          dave_interfaces__srv__TransformToSphericalCoord_Request__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!dave_interfaces__srv__TransformToSphericalCoord_Request__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}


bool
dave_interfaces__srv__TransformToSphericalCoord_Response__init(dave_interfaces__srv__TransformToSphericalCoord_Response * msg)
{
  if (!msg) {
    return false;
  }
  // latitude_deg
  // longitude_deg
  // altitude
  return true;
}

void
dave_interfaces__srv__TransformToSphericalCoord_Response__fini(dave_interfaces__srv__TransformToSphericalCoord_Response * msg)
{
  if (!msg) {
    return;
  }
  // latitude_deg
  // longitude_deg
  // altitude
}

bool
dave_interfaces__srv__TransformToSphericalCoord_Response__are_equal(const dave_interfaces__srv__TransformToSphericalCoord_Response * lhs, const dave_interfaces__srv__TransformToSphericalCoord_Response * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // latitude_deg
  if (lhs->latitude_deg != rhs->latitude_deg) {
    return false;
  }
  // longitude_deg
  if (lhs->longitude_deg != rhs->longitude_deg) {
    return false;
  }
  // altitude
  if (lhs->altitude != rhs->altitude) {
    return false;
  }
  return true;
}

bool
dave_interfaces__srv__TransformToSphericalCoord_Response__copy(
  const dave_interfaces__srv__TransformToSphericalCoord_Response * input,
  dave_interfaces__srv__TransformToSphericalCoord_Response * output)
{
  if (!input || !output) {
    return false;
  }
  // latitude_deg
  output->latitude_deg = input->latitude_deg;
  // longitude_deg
  output->longitude_deg = input->longitude_deg;
  // altitude
  output->altitude = input->altitude;
  return true;
}

dave_interfaces__srv__TransformToSphericalCoord_Response *
dave_interfaces__srv__TransformToSphericalCoord_Response__create(void)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__TransformToSphericalCoord_Response * msg = (dave_interfaces__srv__TransformToSphericalCoord_Response *)allocator.allocate(sizeof(dave_interfaces__srv__TransformToSphericalCoord_Response), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(dave_interfaces__srv__TransformToSphericalCoord_Response));
  bool success = dave_interfaces__srv__TransformToSphericalCoord_Response__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
dave_interfaces__srv__TransformToSphericalCoord_Response__destroy(dave_interfaces__srv__TransformToSphericalCoord_Response * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    dave_interfaces__srv__TransformToSphericalCoord_Response__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence__init(dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__TransformToSphericalCoord_Response * data = NULL;

  if (size) {
    data = (dave_interfaces__srv__TransformToSphericalCoord_Response *)allocator.zero_allocate(size, sizeof(dave_interfaces__srv__TransformToSphericalCoord_Response), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = dave_interfaces__srv__TransformToSphericalCoord_Response__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        dave_interfaces__srv__TransformToSphericalCoord_Response__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence__fini(dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      dave_interfaces__srv__TransformToSphericalCoord_Response__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence *
dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence * array = (dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence *)allocator.allocate(sizeof(dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence__destroy(dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence__are_equal(const dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence * lhs, const dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!dave_interfaces__srv__TransformToSphericalCoord_Response__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence__copy(
  const dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence * input,
  dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(dave_interfaces__srv__TransformToSphericalCoord_Response);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    dave_interfaces__srv__TransformToSphericalCoord_Response * data =
      (dave_interfaces__srv__TransformToSphericalCoord_Response *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!dave_interfaces__srv__TransformToSphericalCoord_Response__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          dave_interfaces__srv__TransformToSphericalCoord_Response__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!dave_interfaces__srv__TransformToSphericalCoord_Response__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}


// Include directives for member types
// Member `info`
#include "service_msgs/msg/detail/service_event_info__functions.h"
// Member `request`
// Member `response`
// already included above
// #include "dave_interfaces/srv/detail/transform_to_spherical_coord__functions.h"

bool
dave_interfaces__srv__TransformToSphericalCoord_Event__init(dave_interfaces__srv__TransformToSphericalCoord_Event * msg)
{
  if (!msg) {
    return false;
  }
  // info
  if (!service_msgs__msg__ServiceEventInfo__init(&msg->info)) {
    dave_interfaces__srv__TransformToSphericalCoord_Event__fini(msg);
    return false;
  }
  // request
  if (!dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence__init(&msg->request, 0)) {
    dave_interfaces__srv__TransformToSphericalCoord_Event__fini(msg);
    return false;
  }
  // response
  if (!dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence__init(&msg->response, 0)) {
    dave_interfaces__srv__TransformToSphericalCoord_Event__fini(msg);
    return false;
  }
  return true;
}

void
dave_interfaces__srv__TransformToSphericalCoord_Event__fini(dave_interfaces__srv__TransformToSphericalCoord_Event * msg)
{
  if (!msg) {
    return;
  }
  // info
  service_msgs__msg__ServiceEventInfo__fini(&msg->info);
  // request
  dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence__fini(&msg->request);
  // response
  dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence__fini(&msg->response);
}

bool
dave_interfaces__srv__TransformToSphericalCoord_Event__are_equal(const dave_interfaces__srv__TransformToSphericalCoord_Event * lhs, const dave_interfaces__srv__TransformToSphericalCoord_Event * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // info
  if (!service_msgs__msg__ServiceEventInfo__are_equal(
      &(lhs->info), &(rhs->info)))
  {
    return false;
  }
  // request
  if (!dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence__are_equal(
      &(lhs->request), &(rhs->request)))
  {
    return false;
  }
  // response
  if (!dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence__are_equal(
      &(lhs->response), &(rhs->response)))
  {
    return false;
  }
  return true;
}

bool
dave_interfaces__srv__TransformToSphericalCoord_Event__copy(
  const dave_interfaces__srv__TransformToSphericalCoord_Event * input,
  dave_interfaces__srv__TransformToSphericalCoord_Event * output)
{
  if (!input || !output) {
    return false;
  }
  // info
  if (!service_msgs__msg__ServiceEventInfo__copy(
      &(input->info), &(output->info)))
  {
    return false;
  }
  // request
  if (!dave_interfaces__srv__TransformToSphericalCoord_Request__Sequence__copy(
      &(input->request), &(output->request)))
  {
    return false;
  }
  // response
  if (!dave_interfaces__srv__TransformToSphericalCoord_Response__Sequence__copy(
      &(input->response), &(output->response)))
  {
    return false;
  }
  return true;
}

dave_interfaces__srv__TransformToSphericalCoord_Event *
dave_interfaces__srv__TransformToSphericalCoord_Event__create(void)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__TransformToSphericalCoord_Event * msg = (dave_interfaces__srv__TransformToSphericalCoord_Event *)allocator.allocate(sizeof(dave_interfaces__srv__TransformToSphericalCoord_Event), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(dave_interfaces__srv__TransformToSphericalCoord_Event));
  bool success = dave_interfaces__srv__TransformToSphericalCoord_Event__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
dave_interfaces__srv__TransformToSphericalCoord_Event__destroy(dave_interfaces__srv__TransformToSphericalCoord_Event * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    dave_interfaces__srv__TransformToSphericalCoord_Event__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence__init(dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__TransformToSphericalCoord_Event * data = NULL;

  if (size) {
    data = (dave_interfaces__srv__TransformToSphericalCoord_Event *)allocator.zero_allocate(size, sizeof(dave_interfaces__srv__TransformToSphericalCoord_Event), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = dave_interfaces__srv__TransformToSphericalCoord_Event__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        dave_interfaces__srv__TransformToSphericalCoord_Event__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence__fini(dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      dave_interfaces__srv__TransformToSphericalCoord_Event__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence *
dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence * array = (dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence *)allocator.allocate(sizeof(dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence__destroy(dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence__are_equal(const dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence * lhs, const dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!dave_interfaces__srv__TransformToSphericalCoord_Event__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence__copy(
  const dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence * input,
  dave_interfaces__srv__TransformToSphericalCoord_Event__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(dave_interfaces__srv__TransformToSphericalCoord_Event);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    dave_interfaces__srv__TransformToSphericalCoord_Event * data =
      (dave_interfaces__srv__TransformToSphericalCoord_Event *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!dave_interfaces__srv__TransformToSphericalCoord_Event__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          dave_interfaces__srv__TransformToSphericalCoord_Event__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!dave_interfaces__srv__TransformToSphericalCoord_Event__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
