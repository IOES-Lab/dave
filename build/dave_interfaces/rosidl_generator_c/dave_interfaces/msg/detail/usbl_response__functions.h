// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from dave_interfaces:msg/UsblResponse.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "dave_interfaces/msg/usbl_response.h"


#ifndef DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__FUNCTIONS_H_
#define DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/action_type_support_struct.h"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_runtime_c/service_type_support_struct.h"
#include "rosidl_runtime_c/type_description/type_description__struct.h"
#include "rosidl_runtime_c/type_description/type_source__struct.h"
#include "rosidl_runtime_c/type_hash.h"
#include "rosidl_runtime_c/visibility_control.h"
#include "dave_interfaces/msg/rosidl_generator_c__visibility_control.h"

#include "dave_interfaces/msg/detail/usbl_response__struct.h"

/// Initialize msg/UsblResponse message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * dave_interfaces__msg__UsblResponse
 * )) before or use
 * dave_interfaces__msg__UsblResponse__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__msg__UsblResponse__init(dave_interfaces__msg__UsblResponse * msg);

/// Finalize msg/UsblResponse message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__msg__UsblResponse__fini(dave_interfaces__msg__UsblResponse * msg);

/// Create msg/UsblResponse message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * dave_interfaces__msg__UsblResponse__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
dave_interfaces__msg__UsblResponse *
dave_interfaces__msg__UsblResponse__create(void);

/// Destroy msg/UsblResponse message.
/**
 * It calls
 * dave_interfaces__msg__UsblResponse__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__msg__UsblResponse__destroy(dave_interfaces__msg__UsblResponse * msg);

/// Check for msg/UsblResponse message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__msg__UsblResponse__are_equal(const dave_interfaces__msg__UsblResponse * lhs, const dave_interfaces__msg__UsblResponse * rhs);

/// Copy a msg/UsblResponse message.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source message pointer.
 * \param[out] output The target message pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer is null
 *   or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__msg__UsblResponse__copy(
  const dave_interfaces__msg__UsblResponse * input,
  dave_interfaces__msg__UsblResponse * output);

/// Retrieve pointer to the hash of the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_type_hash_t *
dave_interfaces__msg__UsblResponse__get_type_hash(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_runtime_c__type_description__TypeDescription *
dave_interfaces__msg__UsblResponse__get_type_description(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the single raw source text that defined this type.
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_runtime_c__type_description__TypeSource *
dave_interfaces__msg__UsblResponse__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the recursive raw sources that defined the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
const rosidl_runtime_c__type_description__TypeSource__Sequence *
dave_interfaces__msg__UsblResponse__get_type_description_sources(
  const rosidl_message_type_support_t * type_support);

/// Initialize array of msg/UsblResponse messages.
/**
 * It allocates the memory for the number of elements and calls
 * dave_interfaces__msg__UsblResponse__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__msg__UsblResponse__Sequence__init(dave_interfaces__msg__UsblResponse__Sequence * array, size_t size);

/// Finalize array of msg/UsblResponse messages.
/**
 * It calls
 * dave_interfaces__msg__UsblResponse__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__msg__UsblResponse__Sequence__fini(dave_interfaces__msg__UsblResponse__Sequence * array);

/// Create array of msg/UsblResponse messages.
/**
 * It allocates the memory for the array and calls
 * dave_interfaces__msg__UsblResponse__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
dave_interfaces__msg__UsblResponse__Sequence *
dave_interfaces__msg__UsblResponse__Sequence__create(size_t size);

/// Destroy array of msg/UsblResponse messages.
/**
 * It calls
 * dave_interfaces__msg__UsblResponse__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
void
dave_interfaces__msg__UsblResponse__Sequence__destroy(dave_interfaces__msg__UsblResponse__Sequence * array);

/// Check for msg/UsblResponse message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__msg__UsblResponse__Sequence__are_equal(const dave_interfaces__msg__UsblResponse__Sequence * lhs, const dave_interfaces__msg__UsblResponse__Sequence * rhs);

/// Copy an array of msg/UsblResponse messages.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source array pointer.
 * \param[out] output The target array pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer
 *   is null or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_dave_interfaces
bool
dave_interfaces__msg__UsblResponse__Sequence__copy(
  const dave_interfaces__msg__UsblResponse__Sequence * input,
  dave_interfaces__msg__UsblResponse__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // DAVE_INTERFACES__MSG__DETAIL__USBL_RESPONSE__FUNCTIONS_H_
