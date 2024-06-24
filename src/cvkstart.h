/**
 * @file cvkstart.h
 * @brief Main header for the `cvkstart`
 */

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
    /**
     * @brief The vulkan instance created by `vs_instance_builder_build`
     */
    VkInstance    vk_instance;

    /**
     * @brief Wether or not a messenger has been created with this instance (i.e. validation layers enabled)
     */
    bool          messenger_created;

    /**
     * @brief Messenger for this instance
     * @note Not valid handle unless `messenger_created` is `true`.
     */
    VkDebugUtilsMessengerEXT    messenger;

    /**
     * @brief The allocation callbacks to be used on all calls by `cvkstart`
     */
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
     * @note Unused/Can be null when used for querying physical device
     */
    VkQueue   *destination;

    /**
     * @brief A pointer to a floating point used for creating queues
     * @note Unused/Can be null when used for querying physical device
     * @note If null when creating device a static float of value `1.0` will be used
     */
    float   *queue_priority;

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
VkDevice vs_device_create(VkPhysicalDevice physical_device, vs_device_builder device_builder, vs_instance instance);

/**
 * @brief Routine to handle the destruction of a device.
 * @note You don't really need to use this function unless you passed some allocation callbacks on instance creation. (this function applies them)
 *
 * @param device The device to destroy
 * @param instance The instance with which the device was created
 */
void     vs_device_destroy(VkDevice device, vs_instance instance);

// ## FORMAT STUFF

/**
 * @brief Represents a set of vulkan formats.
 */
typedef struct
{
    /**
     * @brief The number of formats in the set
     */
    uint32_t    format_count;

    /**
     * @brief A pointer to an array of formats, containing `format_count` elements
     */
    VkFormat   *formats;
} vs_format_set;

/**
 * @brief Represents a query for a format
 */
typedef struct
{
    VkFormatFeatureFlags    required_linear_tiling_features;
    VkFormatFeatureFlags    required_optimal_tiling_features;
    VkFormatFeatureFlags    required_buffer_features;
} vs_format_query;

// TODO: Add useful format sets like : RGB, RGBA ...

/**
 * @brief Finds the first format in `candidates` supporting the features in `query`
 *
 * @param physical_device The physical device on which to query
 * @param query The requirements
 * @param candidates The formats in which to find a suitable one
 * @param[out] index A pointer to where to write the first suitable format
 * @return Wether or not a suitable format was found
 * @note The integer pointed to by `index` will not be modified if the return value is false
 */
bool     vs_format_query_index(VkPhysicalDevice physical_device, vs_format_query query, vs_format_set candidates, uint32_t *index);

/**
 * @brief Finds the first format in `candidates` supporting the features in `query`
 *
 * @param physical_device The physical device on which to query
 * @param query The requirements
 * @param candidates The formats in which to find a suitable one
 * @return The first suitable format found. `VK_FORMAT_UNDEFINED` if no suitable format was found.
 */
VkFormat vs_format_query_format(VkPhysicalDevice physical_device, vs_format_query query, vs_format_set candidates);

/**
 * @brief Finds the formats in `candidates` supporting the features in `query`
 *
 * @param physical_device The physical device on which to query
 * @param query The requirements
 * @param candidates The formats in which to find a suitable one
 * @param[out] out_count The number of output formats
 * @param[out] out_formats Where to write the output formats (can be NULL)
 */
void     vs_format_query_formats(VkPhysicalDevice physical_device, vs_format_query query, vs_format_set set, uint32_t *out_count, VkFormat *out_formats);

// ## SWAPCHAIN

#ifndef VS_SWAPCHAIN_MAX_IMG_COUNT
#define VS_SWAPCHAIN_MAX_IMG_COUNT 8
#endif

typedef struct vs_swapchain_info
{
    VkExtent2D           swapchain_extent;
    VkFormat             swapchain_image_format;
    VkColorSpaceKHR      swapchain_color_space;
    VkImageUsageFlags    image_usage;

    uint32_t             image_count;
    VkImage              swapchain_images[VS_SWAPCHAIN_MAX_IMG_COUNT];
} vs_swapchain_info;

typedef void (*vs_swapchain_callback_func)(VkDevice device, void *udata, vs_swapchain_info swapchain);

typedef struct vs_swapchain
{
    bool                 swapchain_created;
    VkSwapchainKHR       vk_swapchain;
    VkSurfaceKHR         surface;

    vs_swapchain_info    swapchain_info;
} vs_swapchain;

/**
 * @brief Sets up a `cvkstart` swapchain
 *
 * @param device The device with which to create the swapchain
 * @param surface The surface with which to create the swapchain
 * @param image_usage The image usage of the swapchain images
 * @param image_format The format used for the images
 * @param swapchain_color_space The colorspace to use for the swapchain
 * @param[out] swapchain A pointer to were to write the new swapchain object
 * @returns Wether or not the swapchain set up was successful
 * @note The swapchain won't be ready after the call of this function.
 */
bool vs_swapchain_preconfigure(VkDevice device,
                               VkSurfaceKHR surface,
                               VkImageUsageFlags image_usage, VkFormat image_format, VkColorSpaceKHR swapchain_color_space,
                               vs_swapchain *swapchain);



#endif //__CVKSTART_H__

