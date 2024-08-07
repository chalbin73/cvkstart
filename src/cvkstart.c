#include "cvkstart.h"
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

#define VS_VALIDATION_LAYER      "VK_LAYER_KHRONOS_validation"
#define VS_DEBUG_UTILS_EXTENSION "VK_EXT_debug_utils"

// We can expect all platforms supporting vulkan, supporting alloca
#include <alloca.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>


static VkResult
_vs_vkCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT   *pCreateInfo,
    const VkAllocationCallbacks                *pAllocator,
    VkDebugUtilsMessengerEXT                   *pMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func)
    {
        return func(instance, pCreateInfo, pAllocator, pMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void
_vs_vkDestroyDebugUtilsMessengerEXT(
    VkInstance                     instance,
    VkDebugUtilsMessengerEXT       messenger,
    const VkAllocationCallbacks   *pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func)
    {
        func(instance, messenger, pAllocator);
    }
}

bool
_vs_instance_buider_check_extension_support(char **extensions, uint32_t extension_count)
{
    uint32_t supported_count = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &supported_count, NULL);
    VkExtensionProperties *props = alloca(sizeof(VkExtensionProperties) * supported_count);
    vkEnumerateInstanceExtensionProperties(NULL, &supported_count, props);

    // Check support
    for(uint32_t i = 0; i < extension_count; i++)
    {
        bool supported = false;
        for(uint32_t j = 0; j < supported_count; j++)
        {
            if( strcmp(extensions[i], props[j].extensionName) )
            {
                supported = true;
            }
        }

        if(!supported)
        {
            // A required extension was not found.
            // TODO: Log which extension was not supported
            return false;
        }
    }

    // All extensions were found
    return true;
}

bool
_vs_instance_buider_check_layers_support(char **layers, uint32_t layer_count)
{
    uint32_t supported_count = 0;
    vkEnumerateInstanceLayerProperties(&supported_count, NULL);
    VkLayerProperties *props = alloca(sizeof(VkLayerProperties) * supported_count);
    vkEnumerateInstanceLayerProperties(&supported_count, props);

    // Check support
    for(uint32_t i = 0; i < layer_count; i++)
    {
        bool supported = false;
        for(uint32_t j = 0; j < supported_count; j++)
        {
            if( strcmp(layers[i], props[j].layerName) )
            {
                supported = true;
            }
        }

        if(!supported)
        {
            // A required extension was not found.
            // TODO: Log which extension was not supported
            return false;
        }
    }

    // All extensions were found
    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
_vc_default_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        printf("[VULKAN][VERBOSE]: %s\n", pCallbackData->pMessage);
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        printf("[VULKAN][INFO]: %s\n", pCallbackData->pMessage);
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        printf("[VULKAN][WARNING]: %s\n", pCallbackData->pMessage);
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        printf("[VULKAN][ERROR]: %s\n", pCallbackData->pMessage);
        break;

    default:
        printf("[VULKAN][...]: %s\n", pCallbackData->pMessage);
        break;
    }
    return VK_FALSE;
}

bool
vs_instance_builder_build(vs_instance_builder instance_builder, vs_instance *out_instance)
{
    if(out_instance == NULL)
        return false;

    // ## App Information ##
    // Api version

    // Get version supported
    uint32_t require_version  = 0;
    uint32_t instance_version = 0;
    vkEnumerateInstanceVersion(&instance_version);

    out_instance->messenger_created = false;

    if(instance_builder.required_api_version != 0) // Select version based on strict requirement
    {
        if(instance_version < instance_builder.required_api_version)
        {
            return false;
        }
        require_version = instance_builder.required_api_version;
    }
    else if(instance_builder.minimum_api_version != 0)
    {
        if(instance_version < instance_builder.minimum_api_version)
        {
            return false;
        }

        require_version = instance_version;
    }
    else
    {
        require_version = instance_version;
    }

    VkApplicationInfo app_info =
    {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion         = require_version,
        .pEngineName        = instance_builder.engine_name ? instance_builder.engine_name : "",
        .pApplicationName   = instance_builder.app_name ? instance_builder.app_name : "",
        .applicationVersion = instance_builder.application_version,
    };

    // Setup layers
    uint32_t layer_count = instance_builder.requested_layer_count +
                           (instance_builder.request_validation_layers ? 1 : 0); // Validations layers need a specific layer
    char **all_layers = alloca(sizeof(char *) * layer_count);

    // Fill layers
    {
        uint32_t i = 0;
        for(i = 0; i < instance_builder.requested_layer_count; i++) // Could memcpy, but this is more explicit
        {
            all_layers[i] = instance_builder.requested_layers[i];
        }

        // Implicit layers
        if(instance_builder.request_validation_layers)
        {
            all_layers[i++] = VS_VALIDATION_LAYER;
        }
    }

    // Setup extensions
    uint32_t extension_count = instance_builder.requested_extension_count +
                               (instance_builder.request_validation_layers ? 1 : 0); // Validations layers need a specific layer
    char **all_extensions = alloca(sizeof(char *) * extension_count);

    // Fill extensions
    {
        uint32_t i = 0;
        for(i = 0; i < instance_builder.requested_extension_count; i++) // Could memcpy, but this is more explicit
        {
            all_extensions[i] = instance_builder.requested_extensions[i];
        }

        // Implicit extensions
        if(instance_builder.request_validation_layers)
        {
            all_extensions[i++] = VS_DEBUG_UTILS_EXTENSION;
        }
    }

    // Check support
    if( !_vs_instance_buider_check_layers_support(all_layers, layer_count) )
    {
        return false;
    }

    if( !_vs_instance_buider_check_extension_support(all_extensions, extension_count) )
    {
        return false;
    }

    // All layers and extensions supported at this point

    VkInstanceCreateInfo instance_ci =
    {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags                   = 0,
        .pApplicationInfo        = &app_info,
        .enabledLayerCount       = layer_count,
        .ppEnabledLayerNames     = (const char * const *) all_layers,
        .enabledExtensionCount   = extension_count,
        .ppEnabledExtensionNames = (const char * const *) all_extensions,
    };

    // Create instance
    VkInstance instance   = VK_NULL_HANDLE;
    VkResult instance_res = vkCreateInstance(&instance_ci, instance_builder.allocation_callbacks, &instance);
    if(instance_res != VK_SUCCESS)
    {
        return false;
    }
    out_instance->vk_instance          = instance;
    out_instance->allocation_callbacks = instance_builder.allocation_callbacks;
    out_instance->messenger            = VK_NULL_HANDLE;

    if(!instance_builder.request_validation_layers)
    {
        out_instance->messenger_created = false;
        return true; // Everything done without validation layers
    }

    VkDebugUtilsMessengerCreateInfoEXT messenger_ci =
    {
        .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .flags           = 0,
        .messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType     = instance_builder.validation_layers_message_types,
        .pfnUserCallback = !instance_builder.messenger_callback ? _vc_default_debug_callback : instance_builder.messenger_callback,
        .pUserData       = instance_builder.messenger_user_data,
    };

    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    VkResult messenger_res             = _vs_vkCreateDebugUtilsMessengerEXT(instance, &messenger_ci, instance_builder.allocation_callbacks, &messenger);

    if(messenger_res != VK_SUCCESS)
    {
        vkDestroyInstance(instance, instance_builder.allocation_callbacks);
        return false;
    }

    out_instance->messenger_created = true;
    out_instance->messenger         = messenger;
    return true;
}

void
vs_instance_destroy(vs_instance    instance)
{
    if(instance.messenger_created)
    {
        _vs_vkDestroyDebugUtilsMessengerEXT(instance.vk_instance, instance.messenger, instance.allocation_callbacks);
    }

    vkDestroyInstance(instance.vk_instance, instance.allocation_callbacks);
}

// #################################
// ### PHYSICAL DEVICE SELECTION ###
// #################################

typedef struct
{
    bool                suitable;
    VkPhysicalDevice    device;
} _vs_phydev_candidate;

#define _VS_PHYDEV_UNSUITABLE(dev) \
        (dev).suitable = false;

void
_vs_enumerate_phydev_candidates(VkInstance instance, uint32_t *count, _vs_phydev_candidate *dest)
{
    vkEnumeratePhysicalDevices(instance, count, NULL);

    if(dest == NULL)
    {
        return;
    }

    VkPhysicalDevice *devices = alloca(sizeof(VkPhysicalDevice) * *count);
    vkEnumeratePhysicalDevices(instance, count, devices);

    for(uint32_t i = 0; i < *count; i++)
    {
        dest[i].device   = devices[i];
        dest[i].suitable = true;
    }
}

// ## Criterions
void
_vs_phydev_crit_minimum_version(_vs_phydev_candidate *candidate, vs_physical_device_selector selector)
{
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(candidate->device, &props);
    if(props.apiVersion < selector.minimum_version)
    {
        _VS_PHYDEV_UNSUITABLE(*candidate);
    }
}

void
_vs_phydev_crit_present_queue(_vs_phydev_candidate *candidate, vs_physical_device_selector selector)
{
    if(!selector.require_present_queue || selector.surface == VK_NULL_HANDLE)
    {
        return;
    }

    uint32_t prop_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(candidate->device, &prop_count, NULL);
    VkQueueFamilyProperties *props = alloca(sizeof(VkQueueFamilyProperties) * prop_count);
    vkGetPhysicalDeviceQueueFamilyProperties(candidate->device, &prop_count, props);

    bool fullfilled = false;
    for(uint32_t i = 0; i < prop_count; i++)
    {
        VkBool32 supports = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(candidate->device, i, selector.surface, &supports);
        if(supports)
        {
            fullfilled = true;
        }
    }

    if(!fullfilled)
    {
        _VS_PHYDEV_UNSUITABLE(*candidate);
    }
}

bool
_vs_phydev_crit_required_features_bool(_vs_phydev_candidate *candidate, vs_physical_device_selector selector)
{
    VkPhysicalDeviceFeatures req = selector.required_features;
    VkPhysicalDeviceFeatures dev;

    vkGetPhysicalDeviceFeatures(candidate->device, &dev);

    if (req.robustBufferAccess && !dev.robustBufferAccess)
        return false;
    if (req.fullDrawIndexUint32 && !dev.fullDrawIndexUint32)
        return false;
    if (req.imageCubeArray && !dev.imageCubeArray)
        return false;
    if (req.independentBlend && !dev.independentBlend)
        return false;
    if (req.geometryShader && !dev.geometryShader)
        return false;
    if (req.tessellationShader && !dev.tessellationShader)
        return false;
    if (req.sampleRateShading && !dev.sampleRateShading)
        return false;
    if (req.dualSrcBlend && !dev.dualSrcBlend)
        return false;
    if (req.logicOp && !dev.logicOp)
        return false;
    if (req.multiDrawIndirect && !dev.multiDrawIndirect)
        return false;
    if (req.drawIndirectFirstInstance && !dev.drawIndirectFirstInstance)
        return false;
    if (req.depthClamp && !dev.depthClamp)
        return false;
    if (req.depthBiasClamp && !dev.depthBiasClamp)
        return false;
    if (req.fillModeNonSolid && !dev.fillModeNonSolid)
        return false;
    if (req.depthBounds && !dev.depthBounds)
        return false;
    if (req.wideLines && !dev.wideLines)
        return false;
    if (req.largePoints && !dev.largePoints)
        return false;
    if (req.alphaToOne && !dev.alphaToOne)
        return false;
    if (req.multiViewport && !dev.multiViewport)
        return false;
    if (req.samplerAnisotropy && !dev.samplerAnisotropy)
        return false;
    if (req.textureCompressionETC2 && !dev.textureCompressionETC2)
        return false;
    if (req.textureCompressionASTC_LDR && !dev.textureCompressionASTC_LDR)
        return false;
    if (req.textureCompressionBC && !dev.textureCompressionBC)
        return false;
    if (req.occlusionQueryPrecise && !dev.occlusionQueryPrecise)
        return false;
    if (req.pipelineStatisticsQuery && !dev.pipelineStatisticsQuery)
        return false;
    if (req.vertexPipelineStoresAndAtomics && !dev.vertexPipelineStoresAndAtomics)
        return false;
    if (req.fragmentStoresAndAtomics && !dev.fragmentStoresAndAtomics)
        return false;
    if (req.shaderTessellationAndGeometryPointSize && !dev.shaderTessellationAndGeometryPointSize)
        return false;
    if (req.shaderImageGatherExtended && !dev.shaderImageGatherExtended)
        return false;
    if (req.shaderStorageImageExtendedFormats && !dev.shaderStorageImageExtendedFormats)
        return false;
    if (req.shaderStorageImageMultisample && !dev.shaderStorageImageMultisample)
        return false;
    if (req.shaderStorageImageReadWithoutFormat && !dev.shaderStorageImageReadWithoutFormat)
        return false;
    if (req.shaderStorageImageWriteWithoutFormat && !dev.shaderStorageImageWriteWithoutFormat)
        return false;
    if (req.shaderUniformBufferArrayDynamicIndexing && !dev.shaderUniformBufferArrayDynamicIndexing)
        return false;
    if (req.shaderSampledImageArrayDynamicIndexing && !dev.shaderSampledImageArrayDynamicIndexing)
        return false;
    if (req.shaderStorageBufferArrayDynamicIndexing && !dev.shaderStorageBufferArrayDynamicIndexing)
        return false;
    if (req.shaderStorageImageArrayDynamicIndexing && !dev.shaderStorageImageArrayDynamicIndexing)
        return false;
    if (req.shaderClipDistance && !dev.shaderClipDistance)
        return false;
    if (req.shaderCullDistance && !dev.shaderCullDistance)
        return false;
    if (req.shaderFloat64 && !dev.shaderFloat64)
        return false;
    if (req.shaderInt64 && !dev.shaderInt64)
        return false;
    if (req.shaderInt16 && !dev.shaderInt16)
        return false;
    if (req.shaderResourceResidency && !dev.shaderResourceResidency)
        return false;
    if (req.shaderResourceMinLod && !dev.shaderResourceMinLod)
        return false;
    if (req.sparseBinding && !dev.sparseBinding)
        return false;
    if (req.sparseResidencyBuffer && !dev.sparseResidencyBuffer)
        return false;
    if (req.sparseResidencyImage2D && !dev.sparseResidencyImage2D)
        return false;
    if (req.sparseResidencyImage3D && !dev.sparseResidencyImage3D)
        return false;
    if (req.sparseResidency2Samples && !dev.sparseResidency2Samples)
        return false;
    if (req.sparseResidency4Samples && !dev.sparseResidency4Samples)
        return false;
    if (req.sparseResidency8Samples && !dev.sparseResidency8Samples)
        return false;
    if (req.sparseResidency16Samples && !dev.sparseResidency16Samples)
        return false;
    if (req.sparseResidencyAliased && !dev.sparseResidencyAliased)
        return false;
    if (req.variableMultisampleRate && !dev.variableMultisampleRate)
        return false;
    if (req.inheritedQueries && !dev.inheritedQueries)
        return false;

    return true;
}

void
_vs_phydev_crit_required_features(_vs_phydev_candidate *candidate, vs_physical_device_selector selector)
{
    if( !_vs_phydev_crit_required_features_bool(candidate, selector) )
    {
        _VS_PHYDEV_UNSUITABLE(*candidate);
    }
}

void
_vs_phydev_crit_required_extensions(_vs_phydev_candidate *candidate, vs_physical_device_selector selector)
{
    uint32_t supported_count = 0;
    vkEnumerateDeviceExtensionProperties(candidate->device, NULL, &supported_count, NULL);
    VkExtensionProperties *dev_exts = alloca(sizeof(VkExtensionProperties) * supported_count);
    vkEnumerateDeviceExtensionProperties(candidate->device, NULL, &supported_count, dev_exts);

    for(uint32_t i = 0; i < selector.required_extension_count; i++)
    {
        bool supports = false;
        for(uint32_t j = 0; j < supported_count; j++)
        {
            if(strcmp(dev_exts[j].extensionName, selector.required_extensions[i]) == 0)
            {
                supports = true;
            }
        }

        if(!supports)
        {
            _VS_PHYDEV_UNSUITABLE(*candidate);
            return;
        }
    }
}

/**
 * @brief Computes a "distance" between flags supported by a queue and required ones
 *
 * @param queue_flags
 * @param required_flags
 * @return Negative number if the queue does not support required, a distance otherwise
 */
int32_t
_vs_queue_flags_distance(VkQueueFlags queue_flags, VkQueueFlags required_flags)
{
    if( (required_flags & queue_flags) != required_flags )
    {
        return -1;
    }

    int32_t distance = 0;
    uint32_t mask    = queue_flags ^ required_flags;  // Will contain ones where the bits are different

    // Count set bits in mask which will be our distance
    while(mask)
    {
        distance += mask & 1;
        mask    >>= 1;
    }
    return distance;
}

void
_vs_phydev_crit_required_queues(_vs_phydev_candidate *candidate, vs_physical_device_selector selector)
{
    uint32_t prop_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(candidate->device, &prop_count, NULL);
    VkQueueFamilyProperties *props = alloca(sizeof(VkQueueFamilyProperties) * prop_count);
    vkGetPhysicalDeviceQueueFamilyProperties(candidate->device, &prop_count, props);

    // Queue selection algorithm principle :
    //
    // Perform a kind of "selection sort" :
    // Foreach request :
    //  Create a queue in the best fitting queue family (using distance function)
    //  Mark request as treated
    // Redo

    for(uint32_t i = 0; i < selector.required_queue_count; i++)
    {
        // Select best fitting queue family
        int32_t best      = -1;
        int32_t best_dist = -1;

        for(uint32_t j = 0; j < prop_count; j++)
        {
            int32_t dist = _vs_queue_flags_distance(props[j].queueFlags, selector.required_queues[i].required_flags);
            if(dist < 0) // Queue supports
                continue;

            if( (dist < best_dist || best_dist < 0) && props[j].queueCount > 0 )
            {
                best_dist = dist;
                best      = j;
            }
        }

        if(best < 0)
        {
            // no fitting queue familly was found
            _VS_PHYDEV_UNSUITABLE(*candidate);
            return;
        }

        // Found a suitable queue familly, "allocate" queue from it
        props[best].queueCount--;
    }
}

void
_vs_phydev_crit_required_types(_vs_phydev_candidate *candidate, vs_physical_device_selector selector, bool required)
{
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(candidate->device, &props);

    VkPhysicalDeviceType req = required ?
                               selector.required_types : selector.preferred_type;

    if( (props.deviceType & req) == 0 && req != 0 )
    {
        _VS_PHYDEV_UNSUITABLE(*candidate);
    }
}

VkPhysicalDevice
vs_select_physical_device(vs_physical_device_selector selector, vs_instance instance)
{
    // Start by listing all available devices
    uint32_t phydev_count = 0;
    _vs_enumerate_phydev_candidates(instance.vk_instance, &phydev_count, NULL);
    _vs_phydev_candidate *candidates = alloca(sizeof(_vs_phydev_candidate) * phydev_count);
    _vs_enumerate_phydev_candidates(instance.vk_instance, &phydev_count, candidates);

    uint32_t suitable_count = 0;
    // Start eliminating candidates based on criterions
    for(uint32_t i = 0; i < phydev_count; i++)
    {
        _vs_phydev_crit_minimum_version(&candidates[i], selector);
        _vs_phydev_crit_present_queue(&candidates[i], selector);
        _vs_phydev_crit_required_queues(&candidates[i], selector);
        _vs_phydev_crit_required_extensions(&candidates[i], selector);
        _vs_phydev_crit_required_features(&candidates[i], selector);
        _vs_phydev_crit_required_types(&candidates[i], selector, true);
        if(candidates[i].suitable)
            suitable_count++;
    }

    // TODO: Score devices based on preferred selectors
    return suitable_count == 0 ? VK_NULL_HANDLE : candidates[0].device;
}

// ## Device creation

typedef struct
{
    uint32_t    familly_index;
    uint32_t    queue_index;

    VkQueue    *destination;
} _vs_dev_queue_write;

bool
_vs_dev_create_queues_info(VkPhysicalDevice device, vs_device_builder builder,
                           uint32_t *queue_write_count, _vs_dev_queue_write *queue_writes,
                           uint32_t *queue_create_info_count, VkDeviceQueueCreateInfo *queue_create_infos)
{
    // Statics
    static float _one_prio = 1.0f;

    uint32_t prop_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &prop_count, NULL);
    VkQueueFamilyProperties *props = alloca(sizeof(VkQueueFamilyProperties) * prop_count);

    // Will represent the amount of allocations per queue family
    uint32_t *allocations = alloca(sizeof(uint32_t) * prop_count);
    memset(allocations, 0, sizeof(uint32_t) * prop_count);

    vkGetPhysicalDeviceQueueFamilyProperties(device, &prop_count, props);

    // Queue selection algorithm principle :
    //
    // Perform a kind of "selection sort" :
    // Foreach request :
    //  Create a queue in the best fitting queue family (using distance function)
    //  Mark request as treated
    // Redo

    bool present_found   = false;
    uint32_t write_count = 0;
    uint32_t ci_count    = 0;
    for(uint32_t i = 0; i < builder.queue_request_count; i++)
    {
        // ### Select best fitting queue family ###
        int32_t best      = -1;
        int32_t best_dist = -1;

        for(uint32_t j = 0; j < prop_count; j++)
        {
            int32_t dist = _vs_queue_flags_distance(props[j].queueFlags, builder.queue_requests[i].required_flags);
            if(dist < 0) // Queue supports
                continue;

            if( (dist < best_dist || best_dist < 0) && props[j].queueCount > 0 )
            {
                best_dist = dist;
                best      = j;
            }
        }

        // --- IF NO FITTING QUEUE FAMILY WAS FOUND
        if(best < 0)
        {
            // no fitting queue familly was found, request cannot be fullfilled
            return false;
        }

        queue_writes[write_count++] = (_vs_dev_queue_write)
        {
            .destination   = builder.queue_requests[i].destination,
            .queue_index   = allocations[best],
            .familly_index = best,
        };

        // Handle support
        if(!present_found && builder.request_present_queue)
        {
            VkBool32 presents = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, best, builder.surface, &presents);
            if(presents)
            {
                queue_writes[write_count++] = (_vs_dev_queue_write)
                {
                    .destination   = builder.present_destination,
                    .queue_index   = allocations[best],
                    .familly_index = best,
                };
            }
        }

        props[best].queueCount--;
        allocations[best]++;
    }

    // Manage queue cis
    for(uint32_t i = 0; i < prop_count; i++)
    {
        if(allocations[i] != 0)
        {
            queue_create_infos[ci_count++] = (VkDeviceQueueCreateInfo)
            {
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueCount       = allocations[i],
                .flags            = 0,
                .queueFamilyIndex = i,
                .pQueuePriorities = &_one_prio,
            };
        }
    }

    // Check for present support
    if(!builder.request_present_queue || present_found) // Logical equivalent of request_present => present_found
    {
        *queue_create_info_count = ci_count;
        *queue_write_count       = write_count;
        return true;
    }

    // Create queue for present
    // Search for present queue
    present_found = false;
    uint32_t present_family = 0;
    for(uint32_t i = 0; i < prop_count; i++)
    {
        VkBool32 presents = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, builder.surface, &presents);

        if(presents && props[i].queueCount > 0)
        {
            present_found  = true;
            present_family = i;
            break;
        }
    }

    if(!present_found)
    {
        return false;
    }

    // Write new present queue
    queue_writes[write_count++] = (_vs_dev_queue_write)
    {
        .destination   = builder.present_destination,
        .queue_index   = 0,
        .familly_index = present_family,
    };

    queue_create_infos[ci_count++] = (VkDeviceQueueCreateInfo)
    {
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueCount       = 1,
        .flags            = 0,
        .queueFamilyIndex = present_family,
        .pQueuePriorities = &_one_prio,
    };

    *queue_create_info_count = ci_count;
    *queue_write_count       = write_count;
    return true;
}

VkDevice
vs_device_create(VkPhysicalDevice physical_device, vs_device_builder device_builder, vs_instance instance)
{
    // We don't exactly know how big those arrays are, but we have a good upper bound
    _vs_dev_queue_write *queue_writes  = alloca( sizeof(_vs_dev_queue_write) * (device_builder.queue_request_count + 1) ); // +1 to accomodate for present queue
    VkDeviceQueueCreateInfo *queue_cis = alloca( sizeof(VkDeviceQueueCreateInfo) * (device_builder.queue_request_count + 1) );

    uint32_t queue_write_count = 0;
    uint32_t queue_ci_count    = 0;

    bool queue_result = _vs_dev_create_queues_info(
        physical_device,
        device_builder,
        &queue_write_count,
        queue_writes,
        &queue_ci_count,
        queue_cis
        );

    if(!queue_result)
    {
        return VK_NULL_HANDLE;
    }

    VkDeviceCreateInfo device_ci =
    {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .flags                   = 0,
        .pEnabledFeatures        = &device_builder.features,
        .pQueueCreateInfos       = queue_cis,
        .queueCreateInfoCount    = queue_ci_count,
        .enabledExtensionCount   = device_builder.enable_extension_count,
        .ppEnabledExtensionNames = (const char * const *)device_builder.enable_extensions,
    };

    VkDevice device        = VK_NULL_HANDLE;
    VkResult device_result = vkCreateDevice(physical_device, &device_ci, instance.allocation_callbacks, &device);

    if(device_result != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }

    // Retrieve queues
    for(uint32_t i = 0; i < queue_write_count; i++)
    {
        VkQueue q = VK_NULL_HANDLE;
        vkGetDeviceQueue(device, queue_writes[i].familly_index, queue_writes[i].queue_index, &q);

        if(queue_writes[i].destination)
        {
            *queue_writes[i].destination = q;
        }
    }

    return device;
}

void
vs_device_destroy(VkDevice device, vs_instance instance)
{
    vkDestroyDevice(device, instance.allocation_callbacks);
}

// ## FORMAT STUFF

bool
vs_format_query_index(VkPhysicalDevice physical_device, vs_format_query query, vs_format_set candidates, uint32_t *index)
{
    for(int i = 0; i < candidates.format_count; i++)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, candidates.formats[i], &props);

        bool valid = true;

        valid &= ( (props.optimalTilingFeatures & query.required_optimal_tiling_features) == query.required_optimal_tiling_features );
        valid &= ( (props.linearTilingFeatures & query.required_linear_tiling_features) == query.required_linear_tiling_features );
        valid &= ( (props.bufferFeatures & query.required_buffer_features) == query.required_buffer_features );

        if(valid)
        {
            *index = i;
            return true;
        }
    }
    return false;
}

VkFormat
vs_format_query_format(VkPhysicalDevice physical_device, vs_format_query query, vs_format_set candidates)
{
    uint32_t index = 0;
    if( vs_format_query_index(physical_device, query, candidates, &index) )
    {
        return candidates.formats[index];
    }
    return VK_FORMAT_UNDEFINED;
}

void
vs_format_query_formats(VkPhysicalDevice physical_device, vs_format_query query, vs_format_set candidates, uint32_t *out_count, VkFormat *out_formats)
{
    if(!out_count)
    {
        return;
    }
    *out_count = 0;

    for(int i = 0; i < candidates.format_count; i++)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, candidates.formats[i], &props);

        bool valid = true;

        valid &= ( (props.optimalTilingFeatures & query.required_optimal_tiling_features) == query.required_optimal_tiling_features );
        valid &= ( (props.linearTilingFeatures & query.required_linear_tiling_features) == query.required_linear_tiling_features );
        valid &= ( (props.bufferFeatures & query.required_buffer_features) == query.required_buffer_features );

        if(valid)
        {
            if(out_formats)
            {
                out_formats[*out_count] = candidates.formats[i];
            }
            (*out_count)++;
        }
    }
}

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
bool
vs_swapchain_preconfigure(VkDevice device,
                          VkSurfaceKHR surface,
                          VkImageUsageFlags image_usage, VkFormat image_format, VkColorSpaceKHR swapchain_color_space,
                          vs_swapchain *swapchain)
{
    // Copy necessary info into stuct
    swapchain->surface                               = surface;
    swapchain->swapchain_info.image_usage            = image_usage;
    swapchain->swapchain_info.swapchain_image_format = image_format;
    swapchain->swapchain_info.swapchain_color_space  = swapchain_color_space;

    return true;
}

#define VS_MIN(a, b) ( (a) < (b) ? (a) : (b) )

bool
vs_swapchain_create(vs_swapchain swapchain, VkPhysicalDevice phy_dev, VkSurfaceKHR surface, uint32_t min_img_count)
{
    VkSurfaceCapabilitiesKHR surf_caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phy_dev, surface, &surf_caps);

    VkSwapchainCreateInfoKHR swp_ci =
    {
        .sType         = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface       = surface,
        .minImageCount = VS_MIN(surf_caps.maxImageCount, min_img_count);
    };
}

