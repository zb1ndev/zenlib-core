#include "../include/zenlib_core.h"

int zen_initialize_renderer(ZEN_Window* window, ZEN_RendererAPI api) {

    if (window == NULL) 
        return -1;
    
    switch (api) {

        case ZEN_RAPI_None:     return 0;
        case ZEN_RAPI_OpenGL:   {} return 0;
        case ZEN_RAPI_DirectX:  {} return 0;
        
        case ZEN_RAPI_Vulkan:   {
            
            if (!__zencore_context__.vk_context.initialized) {
                if (zen_global_init_vulkan(window, ZEN_VULKAN_VERSION) < 0)
                    return -1;
                debug_log("Vulkan Initialized.");
                return 0;
            }

            // Already initialized, making nth window
        
        } return 0;

    }

    return -1;

}

#pragma region DirectX

    int zen_global_init_directx(void) {
        log_error("Direct X is not implemented yet!");
        return -1;
    }

#pragma endregion // DirectX
#pragma region OpenGL

    int zen_global_init_opengl(void)  {
        log_error("OpenGL is not implemented yet!");
        return -1;
    }

#pragma endregion // OpenGL
#pragma region Vulkan

    int zen_global_init_vulkan(ZEN_Window* window, uint32_t api_version) {

        if (zen_vk_create_instance(api_version) < 0) {
            log_error("Failed to create vulkan instance.");
            return -1;
        } debug_log("Created Instance...");

        if (zen_vk_create_surface(window) < 0) {
            log_error("Failed to create vulkan instance.");
            return -1;
        } debug_log("Created Window Surface...");

        if (zen_vk_pick_physical_device(window) < 0) {
            log_error("Failed to find suitable GPU.");
            return -1;
        } debug_log("Found Device...");

        if (zen_vk_create_logical_device() < 0) {
            log_error("Failed to create logical device.");
            return -1;
        } debug_log("Created Logical Device...");

        if (zen_vk_create_swapchain(window) < 0) {
            log_error("Failed to create swapchain.");
            return -1;
        }

        if (__zencore_context__.vk_context.create_graphics_pipline == NULL) {
            log_error("\"create_graphics_pipline\" was not initialized.");
            return -1;
        }
        
        if (__zencore_context__.vk_context.create_graphics_pipline(window) < 0) {
            log_error("Failed to create graphics pipline.");
            return -1;
        }

        return 0;

    }

#pragma endregion // Vulkan