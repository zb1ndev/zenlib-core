#include "../../include/zenlib_core.h"

#if defined(ZEN_OS_WINDOWS)

    int zen_initialize_renderer(ZEN_Window* window, ZEN_RendererAPI api) {

        if (window == NULL) return -1;
        
        switch (api) {

            case ZEN_RAPI_None:     return 0;
            case ZEN_RAPI_OpenGL:   {} return 0;
            case ZEN_RAPI_Vulkan:   {} return 0;
            case ZEN_RAPI_DirectX:  {} return 0;

        }

        return -1;

    }

#endif // ZEN_OS_WINDOWS

