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
 * @brief Represents the way a queue must follow required flags
 */
typedef enum
{
    /**
     * @brief The queue flags must be strictly equal to those specified by the user
     */
    VS_QUEUE_SELECT_STRICT,

    /**
     * @brief The queue flags must support at least the ones specified by the user
     */
    VS_QUEUE_SELECT_SUPPORTS,
} vs_queue_select_mode;

/**
 * @brief Represents a VkQueue request that must be fullfilled when creating a device
 */
typedef struct
{
    /**
     * @brief The flags that the queue must support
     */
    VkQueueFlagBits         required_flags;

    /**
     * @brief How the queue must support those flags
     */
    vs_queue_select_mode    select_mode;

    /**
     * @brief Where to write the created queue after it was selected
     */
    VkQueue                *destination;
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

} vs_physical_device_selector;


/**
 * @brief Selects a suitable physical device based on the specified selecion criteria
 *
 * @param selector The selector
 * @return A suitable physical device or `VK_NULL_HANDLE` if no suitable device was found.
 */
VkPhysicalDevice vs_select_physical_device(vs_physical_device_selector selector, vs_instance instance);

#endif //__CVKSTART_H__

