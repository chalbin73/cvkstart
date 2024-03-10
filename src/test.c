#include <stdio.h>
#include "cvkstart.h"

int
main()
{
    vs_instance instance;
    if(
        !vs_instance_builder_build(
            (vs_instance_builder)
            {
                .app_name = "Eude",
                .engine_name = "Eugène",
                .request_validation_layers = true,
                .minimum_api_version = VK_MAKE_VERSION(1, 3, 0),
            },
            &instance
            )
        )
    {
        return 1;
    }

    vs_instance_destroy(instance);
    return 0;
}

