#include <stdio.h>
//#include "cvkstart.h"
#include "cvkstart.c"

int
main()
{
    vs_instance instance;
    if(
        !vs_instance_builder_build(
            (vs_instance_builder)
            {
                .app_name = "Eude",
                .engine_name = "Eugene",
                .request_validation_layers = true,
                .minimum_api_version = VK_MAKE_VERSION(1, 2, 0),
                .validation_layers_message_types = VS_DEBUG_UTILS_MESSAGE_TYPE_ALL,
            },
            &instance
            )
        )
    {
        printf("Could not create instance. LEBRON JAMES\n");
        return 1;
    }

    vs_queue_request q_req[5] =
    {
        [0] =
        {
        .required_flags = VK_QUEUE_TRANSFER_BIT
        },
        [1] =
        {
        .required_flags = VK_QUEUE_TRANSFER_BIT
        },
        [2] =
        {
        .required_flags = VK_QUEUE_TRANSFER_BIT
        },
        [3] =
        {
        .required_flags = VK_QUEUE_TRANSFER_BIT
        },
        [4] =
        {
        .required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT
        },
    };

    VkPhysicalDevice phy_dev =
        vs_select_physical_device(
            (vs_physical_device_selector)
            {
                .required_types                   = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU | VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
                .required_features.geometryShader = true,
                .minimum_version                  = VK_VERSION_1_3,
                .require_present_queue            = false,
                .required_queue_count             = 5,
                .required_queues                  = q_req,
            },
            instance
            );

    if(phy_dev == VK_NULL_HANDLE)
    {
        printf("Could not find physical device.\n");
        return 1;
    }

    VkDevice device = vs_device_create(
        phy_dev,
        (vs_device_builder)
        {
            .queue_request_count     = 5,
            .queue_requests          = q_req,
            .enable_extension_count  = 0,
            .features.geometryShader = true,
        },
        instance
        );

    if(device == VK_NULL_HANDLE)
    {
        printf("Could not create physical device.\n");
        return 1;
    }

    vkDestroyDevice(device, NULL);

    vs_instance_destroy(instance);
    return 0;
}

