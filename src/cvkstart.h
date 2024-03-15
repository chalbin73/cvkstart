// ### CVKSTART ###

/*
 * Copyright 2024 Albin Chaboissier
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * NOTES :
 *
 * Prefixes for functions and structures belonging to this projects is "vs_" for "vulkan start"
 *
 */

#ifndef __CVKSTART_H__
#define __CVKSTART_H__

#include <vulkan/vulkan.h>
#include <stdbool.h>

#define VS_DEBUG_UTILS_MESSAGE_TYPE_ALL \
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | \
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | \
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT

// ### INSTANCE

/**
 * @brief Represents the instance and its satellite objects created to cvkstart
 */
typedef struct
{
    VkInstance                  vk_instance;

    bool                        messenger_created;
    VkDebugUtilsMessengerEXT    messenger;

    VkAllocationCallbacks      *allocation_callbacks;
} vs_instance;


/**
 * @brief Represents how to build an instance object
 * @note This structure must be zero initialized so that it is considered as "default".
 */
typedef struct
{
    // App info
    const char                             *app_name;
    const char                             *engine_name;
    uint32_t                                application_version;

    // Version stuff
    uint32_t                                minimum_api_version;
    uint32_t                                required_api_version;

    // Validation layers
    bool                                    request_validation_layers;
    PFN_vkDebugUtilsMessengerCallbackEXT    messenger_callback; // If NULL default is provided.
    void                                   *messenger_user_data;
    VkDebugUtilsMessageTypeFlagBitsEXT      validation_layers_message_types;

    // Layers
    uint32_t                                requested_layer_count;
    char                                  **requested_layers;

    // Extensions
    uint32_t                                requested_extension_count;
    char                                  **requested_extensions;

    // Allocator
    VkAllocationCallbacks                  *allocation_callbacks;
} vs_instance_builder;

/**
 * @brief Builds an instance using the provided instance builder
 *
 * @param instance_builder The instance builder
 * @param instance A pointer in which to write the instance (must point to valid memory region to hold the object)
 */
bool vs_instance_builder_build(vs_instance_builder instance_builder, vs_instance *instance);

/**
 * @brief Destroys the instance object given by `vs_instance_builder_build`
 *
 * @param instance The instance to destroy
 */
void vs_instance_destroy(vs_instance    instance);

/**
 * @brief Represents a VkQueue request that must be fullfilled when creating a device
 */
typedef struct
{
    /**
     * @brief The flags that the queue must support
     */
    VkQueueFlagBits    required_flags;

    /**
     * @brief Where to write the created queue after it was selected
     */
    VkQueue           *destination;

} vs_queue_request;

/**
 * @brief Represents the criteria that a physical device must follow in order to be selected
 * @note Must be zero initalized so that fields that aren't used are detected as such
 */
typedef struct
{
    /**
     * @brief The minimum version that the device must support
     */
    uint32_t        minimum_version;

    /**
     * @brief The surface that must be used if present queues are requested
     */
    VkSurfaceKHR    surface;

    /**
     * @brief Wether or not to require a presentation queue
     * @note If true, then, `surface` must not be `VK_NULL_HANDLE`
     */
    bool        require_present_queue;

    /**
     * @brief The number of requested queues
     */
    uint32_t    required_queue_count;

    /**
     * @brief An array to queue requests. Queue destinations can be null on physical device selection as they will
     *        not be created now.
     */
    vs_queue_request           *required_queues;

    /**
     * @brief The number of required extensions
     */
    uint32_t                    required_extension_count;

    /**
     * @brief An array to NULL terminated strings that contains the required extensions.
     */
    char                      **required_extensions;

    /**
     * @brief The required feature set that the device must support
     */
    VkPhysicalDeviceFeatures    required_features;

    /**
     * @brief The device type to strictly require
     */
    VkPhysicalDeviceType        required_types;

    /**
     * @brief In the event of multiple device being available
     *        through previous criterion, which type to prefer
     */
    VkPhysicalDeviceType    preferred_type;

} vs_physical_device_selector;


/**
 * @brief Selects a suitable physical device based on the specified selecion criteria
 *
 * @param selector The selector
 * @return A suitable physical device or `VK_NULL_HANDLE` if no suitable device was found.
 */
VkPhysicalDevice vs_select_physical_device(vs_physical_device_selector selector, vs_instance instance);

// ## DEVICE CREATION

typedef struct
{
    /**
     * @brief The physical device with which to create the device
     */
    VkPhysicalDevice    physical_device;

    /**
     * @brief The number of queues to request
     */
    uint32_t            queue_request_count;

    /**
     * @brief A pointer to a list of queue requests struct, with valid pointers in the `vs_queue_request::destination` field.
     */
    vs_queue_request   *queue_requests;

    /**
     * @brief Wether or not to request a present queue
     */
    bool                request_present_queue;

    /**
     * @brief The surface with which to query for the present queue. Cannot be `VK_NULL_HANDLE` unless `request_present_queue` is `false`
     */
    VkSurfaceKHR        surface;

    /**
     * @brief Where to write the gotten present queue. Cannot be `VK_NULL_HANDLE` unless `request_present_queue` is `false`
     * @note The present queue can be selected amongst the previously acquired queue, but this is not necessary.
     */
    VkQueue                    *present_destination;

    /**
     * @brief The features to enable on the device.
     */
    VkPhysicalDeviceFeatures    features;

    /**
     * @brief The amount of extensions to enable
     */
    uint32_t                    enable_extension_count;

    /**
     * @brief A pointer to a list of string literals of extensions to enable on the device.
     */
    char                      **enable_extensions;

} vs_device_builder;

/**
 * @brief Creates a device using the provided information
 *
 * @param device_builder The information to create the device
 * @param instance The instance with which to create the device
 * @return The device or `VK_NULL_HANDLE` if the device could not be created
 * @note SIDE EFFECTS: The pointers provided in the queue request information will be accessed and modified.
 *       if the device cannot be created the will not be modified and will remain in their initial state
 */
VkDevice vs_device_create(vs_device_builder device_builder, vs_instance instance);

#endif //__CVKSTART_H__

