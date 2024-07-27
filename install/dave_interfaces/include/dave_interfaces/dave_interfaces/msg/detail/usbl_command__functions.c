// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from dave_interfaces:msg/UsblCommand.idl
// generated code does not contain a copyright notice
#include "dave_interfaces/msg/detail/usbl_command__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `data`
#include "rosidl_runtime_c/string_functions.h"

bool
dave_interfaces__msg__UsblCommand__init(dave_interfaces__msg__UsblCommand * msg)
{
  if (!msg) {
    return false;
  }
  // transponder_id
  // command_id
  // data
  if (!rosidl_runtime_c__String__init(&msg->data)) {
    dave_interfaces__msg__UsblCommand__fini(msg);
    return false;
  }
  return true;
}

void
dave_interfaces__msg__UsblCommand__fini(dave_interfaces__msg__UsblCommand * msg)
{
  if (!msg) {
    return;
  }
  // transponder_id
  // command_id
  // data
  rosidl_runtime_c__String__fini(&msg->data);
}

bool
dave_interfaces__msg__UsblCommand__are_equal(const dave_interfaces__msg__UsblCommand * lhs, const dave_interfaces__msg__UsblCommand * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // transponder_id
  if (lhs->transponder_id != rhs->transponder_id) {
    return false;
  }
  // command_id
  if (lhs->command_id != rhs->command_id) {
    return false;
  }
  // data
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->data), &(rhs->data)))
  {
    return false;
  }
  return true;
}

bool
dave_interfaces__msg__UsblCommand__copy(
  const dave_interfaces__msg__UsblCommand * input,
  dave_interfaces__msg__UsblCommand * output)
{
  if (!input || !output) {
    return false;
  }
  // transponder_id
  output->transponder_id = input->transponder_id;
  // command_id
  output->command_id = input->command_id;
  // data
  if (!rosidl_runtime_c__String__copy(
      &(input->data), &(output->data)))
  {
    return false;
  }
  return true;
}

dave_interfaces__msg__UsblCommand *
dave_interfaces__msg__UsblCommand__create(void)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__msg__UsblCommand * msg = (dave_interfaces__msg__UsblCommand *)allocator.allocate(sizeof(dave_interfaces__msg__UsblCommand), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(dave_interfaces__msg__UsblCommand));
  bool success = dave_interfaces__msg__UsblCommand__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
dave_interfaces__msg__UsblCommand__destroy(dave_interfaces__msg__UsblCommand * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    dave_interfaces__msg__UsblCommand__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
dave_interfaces__msg__UsblCommand__Sequence__init(dave_interfaces__msg__UsblCommand__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__msg__UsblCommand * data = NULL;

  if (size) {
    data = (dave_interfaces__msg__UsblCommand *)allocator.zero_allocate(size, sizeof(dave_interfaces__msg__UsblCommand), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = dave_interfaces__msg__UsblCommand__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        dave_interfaces__msg__UsblCommand__fini(&data[i - 1]);
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
dave_interfaces__msg__UsblCommand__Sequence__fini(dave_interfaces__msg__UsblCommand__Sequence * array)
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
      dave_interfaces__msg__UsblCommand__fini(&array->data[i]);
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

dave_interfaces__msg__UsblCommand__Sequence *
dave_interfaces__msg__UsblCommand__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  dave_interfaces__msg__UsblCommand__Sequence * array = (dave_interfaces__msg__UsblCommand__Sequence *)allocator.allocate(sizeof(dave_interfaces__msg__UsblCommand__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = dave_interfaces__msg__UsblCommand__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
dave_interfaces__msg__UsblCommand__Sequence__destroy(dave_interfaces__msg__UsblCommand__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    dave_interfaces__msg__UsblCommand__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
dave_interfaces__msg__UsblCommand__Sequence__are_equal(const dave_interfaces__msg__UsblCommand__Sequence * lhs, const dave_interfaces__msg__UsblCommand__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!dave_interfaces__msg__UsblCommand__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
dave_interfaces__msg__UsblCommand__Sequence__copy(
  const dave_interfaces__msg__UsblCommand__Sequence * input,
  dave_interfaces__msg__UsblCommand__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(dave_interfaces__msg__UsblCommand);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    dave_interfaces__msg__UsblCommand * data =
      (dave_interfaces__msg__UsblCommand *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!dave_interfaces__msg__UsblCommand__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          dave_interfaces__msg__UsblCommand__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!dave_interfaces__msg__UsblCommand__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
