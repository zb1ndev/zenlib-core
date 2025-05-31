#include "../include/zenlib_core.h"

#pragma region Utilities

    char **zen_vk_get_instance_extensions(size_t *count) {

        #if defined(ZEN_OS_WINDOWS)

            char* extensions[] = {
                VK_KHR_SURFACE_EXTENSION_NAME,
                VK_KHR_WIN32_SURFACE_EXTENSION_NAME    
            };

            *count = 2;
            return extensions;

        #endif // ZEN_OS_WINDOWS

    }

    char **zen_vk_get_device_extensions(size_t *count) {

        #if defined(ZEN_OS_WINDOWS)

            char* extensions[] = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME    
            };

            *count = 1;
            return extensions;

        #endif // ZEN_OS_WINDOWS

    }

    bool zen_vk_find_queue_families(VkPhysicalDevice device, size_t context_index) {

        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);
        ZEN_VulkanSurfaceInfo* info = &__zencore_context__.vk_context.surfaces[context_index];

        if (queue_family_count == 0) {
            log_error("Failed to get physical device queue family properties.");
            return false;
        }

        VkQueueFamilyProperties* families = malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
        if (families == NULL) {
            log_error("Faild to allocate space for queue family properties.");
            return false;
        }

        bool found_graphics_family = false;
        bool found_presentation_family = false;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, families);

        for (size_t i = 0; i < queue_family_count; i++) {
        
            // Graphics Family Queue
            if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                __zencore_context__.vk_context.graphics_family = i;
                found_graphics_family = true;
            }

            // Presentation Family
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, info->surface, &presentSupport);
            if (presentSupport) {
                __zencore_context__.vk_context.present_family = i;
                found_presentation_family = true;
            }

        }

        free(families);

        if (!(found_graphics_family && found_presentation_family))
            return false;


        return true;

    }

    bool zen_vk_query_swapchain_support(VkPhysicalDevice device, size_t context_index) {

        ZEN_VulkanSurfaceInfo* info = &__zencore_context__.vk_context.surfaces[context_index];

        // Query Surface Formats
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR (
            device, 
            info->surface, 
            &info->surface_capabilities
        );

        uint32_t format_count;
        if (vkGetPhysicalDeviceSurfaceFormatsKHR (
            device, info->surface, 
            &format_count, NULL
        ) != VK_SUCCESS) {
            log_error("Failed to find any formats");
            return false;
        }
        info->surface_format_count = (size_t)format_count;

        info->surface_formats = malloc(sizeof(VkSurfaceFormatKHR) * format_count);
        if (info->surface_formats == NULL) {
            log_error("Failed to allocate space for surface formats.");
            return false;
        }

        if (vkGetPhysicalDeviceSurfaceFormatsKHR (
            device, info->surface, 
            &format_count, info->surface_formats
        ) != VK_SUCCESS) {
            log_error("Failed to get surface formats.");
            return false;
        }

        // Query Present Modes
        uint32_t present_mode_count;
        if (vkGetPhysicalDeviceSurfacePresentModesKHR (
            device, info->surface, 
            &present_mode_count, NULL
        ) != VK_SUCCESS) {
            log_error("Failed to find any present modes");
            return false;
        }
        info->present_mode_count = (size_t)present_mode_count;

        info->present_modes = malloc(sizeof(VkPresentModeKHR) * present_mode_count);
        if (info->present_modes == NULL) {
            log_error("Failed to allocate space for surface formats.");
            return false;
        }

        if (vkGetPhysicalDeviceSurfacePresentModesKHR (
            device, 
            info->surface, 
            &present_mode_count, 
            info->present_modes
        ) != VK_SUCCESS) {
            log_error("Failed to get present modes.");
            return false;
        }

        return true;

    }

    bool zen_vk_device_has_extensions(VkPhysicalDevice device) {

        uint32_t extension_count = 0;
        vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);
        if (extension_count == 0) {
            log_error("Failed to enumerate device extensions.");
            return false;
        }

        VkExtensionProperties* extension_props = malloc(sizeof(VkExtensionProperties) * extension_count);
        if (extension_props == NULL) {
            log_error("Failed to allocate space for extension properties.");
            return false;
        }

        size_t req_extension_count = 0;
        char** req_extensions = zen_vk_get_device_extensions(&req_extension_count);

        vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, extension_props);
        for (size_t r = 0; r < req_extension_count; r++) {
            
            bool found_extension = false;
            for (size_t i = 0; i < extension_count; i++) {
                if (strcmp(req_extensions[r], 
                    extension_props[i].extensionName) == 0
                ) found_extension = true;
            }

            if (found_extension == false) {
                log_error("Failed to find device with required extensions.");
                return false;
            }
            
        }
        
        return true;

    }

    bool zen_vk_check_device_api_version(VkPhysicalDevice device) {

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        uint32_t major = VK_VERSION_MAJOR(properties.apiVersion);
        uint32_t minor = VK_VERSION_MINOR(properties.apiVersion);
        uint32_t patch = VK_VERSION_PATCH(properties.apiVersion);

        uint32_t req_major = VK_VERSION_MAJOR(__zencore_context__.vk_context.info.version);
        uint32_t req_minor = VK_VERSION_MINOR(__zencore_context__.vk_context.info.version);
        uint32_t req_patch = VK_VERSION_PATCH(__zencore_context__.vk_context.info.version);

        if (major > req_major) return true;
        if (major == req_major &&  minor > req_minor) return true;
        if (major == req_major &&  minor == req_minor && patch >= req_patch) return true;
        
        return false;

    }

    bool zen_vk_device_is_suitable(VkPhysicalDevice device, size_t context_index) {

        if (!zen_vk_check_device_api_version(device))
            return false;
        if (!zen_vk_device_has_extensions(device))
            return false;
        if (!zen_vk_find_queue_families(device, context_index))
            return false;
        return true;

    }

    VkSurfaceFormatKHR zen_vk_choose_swap_surface_format(size_t context_index) {

        ZEN_VulkanSurfaceInfo* info = &__zencore_context__.vk_context.surfaces[context_index];

        for (size_t i = 0; i < info->surface_format_count; i++) {
            if (info->surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && 
                info->surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return info->surface_formats[i];
        }

        return info->surface_formats[0];

    }

    VkPresentModeKHR zen_vk_choose_swap_present_mode(size_t context_index) {
        (void)context_index;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D zen_vk_choose_swap_extent(size_t context_index) {

        ZEN_VulkanSurfaceInfo* info = &__zencore_context__.vk_context.surfaces[context_index];
        VkSurfaceCapabilitiesKHR capabilities = info->surface_capabilities;

        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;

        VkExtent2D actual_extent = {
            (uint32_t)info->window->width,
            (uint32_t)info->window->height
        };

        actual_extent.width =  (actual_extent.width > capabilities.maxImageExtent.width)   ? capabilities.maxImageExtent.width  : 
                                (actual_extent.width < capabilities.minImageExtent.width)   ? capabilities.minImageExtent.width  : actual_extent.width;
        actual_extent.height = (actual_extent.height > capabilities.maxImageExtent.height) ? capabilities.maxImageExtent.height : 
                                (actual_extent.height < capabilities.minImageExtent.height) ? capabilities.minImageExtent.height : actual_extent.height;

        return actual_extent;

    }

    VkShaderModule zen_vk_create_shader_module(const char* code, size_t code_size) {

        VkShaderModuleCreateInfo create_info = (VkShaderModuleCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code_size,
            .pCode = (uint32_t*)code
        };

        VkShaderModule shader_module;
        if (vkCreateShaderModule(__zencore_context__.vk_context.device, &create_info, NULL, &shader_module) != VK_SUCCESS) {
            log_error("Failed to create shader module.");
            return VK_NULL_HANDLE;
        }
        
        return shader_module;

    }

    static VkVertexInputBindingDescription zen_vk_get_vertex_binding_description() {
            
        VkVertexInputBindingDescription binding_Description = (VkVertexInputBindingDescription) {
            .binding = 0,
            .stride = sizeof(ZEN_Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

        return binding_Description;

    }

    static VkVertexInputAttributeDescription* zen_vk_get_vertex_attribute_descriptions() {

        VkVertexInputAttributeDescription attribute_descriptions[2] = {
            (VkVertexInputAttributeDescription) {
                .binding = 0,
                .location = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = zen_vk_offset_of(ZEN_Vertex, pos)
            },
            (VkVertexInputAttributeDescription) {
                .binding = 0,
                .location = 1,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = zen_vk_offset_of(ZEN_Vertex, color)
            }
        };

        return attribute_descriptions;

    }

#pragma endregion // Utilities

int zen_init_vulkan(ZEN_Window* window, uint32_t api_version) {

    // If the Surfaces array is not initialized, initialize it
    if (__zencore_context__.vk_context.surfaces == NULL) {
        __zencore_context__.vk_context.surfaces = malloc(sizeof(ZEN_VulkanSurfaceInfo) * ZEN_MAX_WINDOWS);
        if (__zencore_context__.vk_context.surfaces == NULL) {
            log_error("Failed to allocate space for window inside Vulkan context.");
            return -1;
        }
    }

    // Update API version and bind window to renderer context.
    __zencore_context__.vk_context.info.version = api_version;
    window->renderer_context_index = __zencore_context__.vk_context.surface_count++;
    __zencore_context__.vk_context.surfaces[window->renderer_context_index].window = window;

    if (zen_vk_create_instance() < 0) {
        log_error("Failed to create Vulkan instance.");
        return -1;
    }

    if (zen_vk_create_surface(window->renderer_context_index) < 0) {
        log_error_va("Failed to create Vulkan surface for window %llu.", window->renderer_context_index);
        return -1;
    }

    if (zen_vk_pick_physical_device(window->renderer_context_index) < 0) {
        log_error("Failed to pick a suitable device.");
        return -1;
    }

    if (zen_vk_create_logical_device() < 0) {
        log_error("Failed to create a logical device.");
        return -1;
    }

    if (zen_vk_create_swap_chain(window->renderer_context_index) < 0) {
        log_error_va("Failed to create swapchain for window %llu.", window->renderer_context_index);
        return -1;
    }

    if (zen_vk_create_image_views(window->renderer_context_index) < 0) {
        log_error_va("Failed to create image views for window %llu.", window->renderer_context_index);
        return -1;
    }

    if (zen_vk_create_render_pass(window->renderer_context_index) < 0) {
        log_error("Failed to create render pass.");
        return -1;
    }

    if (zen_vk_create_graphics_pipelines() < 0) {
        log_error("Failed to create graphics pipelines.");
        return -1;
    }
        
    // createFramebuffers();
    // createCommandPool();
    // createVertexBuffer();
    // createCommandBuffers();
    // createSyncObjects();

    __zencore_context__.vk_context.info.initialized = true;
    return 0;
        
}

void zen_destroy_vulkan(size_t context_index) {
    
    (void)context_index;
    return;

}

int zen_vk_create_instance(void) {

    if (__zencore_context__.vk_context.info.initialized)
        return 0;

    VkApplicationInfo app_info = (VkApplicationInfo) {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "zenlib-core",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),           
        .pEngineName = "NO ENGINE",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = __zencore_context__.vk_context.info.version
    };

    size_t extension_count = 0;
    const char** extensions = zen_vk_get_instance_extensions(&extension_count);

    VkInstanceCreateInfo create_info = (VkInstanceCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = (uint32_t)extension_count,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = 0,
    };

    if (vkCreateInstance(&create_info, NULL, &__zencore_context__.vk_context.instance) != VK_SUCCESS) {
        log_error("Failed to create vulkan instance.");
        return -1;
    }

    return 0;

}

int zen_vk_create_surface(size_t context_index) {

    VkWin32SurfaceCreateInfoKHR create_info = (VkWin32SurfaceCreateInfoKHR) {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hwnd = __zencore_context__.vk_context.surfaces[context_index].window->handle,
        .hinstance = __zencore_context__.h_instance
    };

    if (vkCreateWin32SurfaceKHR (
        __zencore_context__.vk_context.instance, 
        &create_info, NULL, 
        &__zencore_context__.vk_context.surfaces[context_index].surface
    ) != VK_SUCCESS) {
        log_error("Failed to create surface.");
        return -1;
    }

    return 0;

}

int zen_vk_pick_physical_device(size_t context_index) {

    if (__zencore_context__.vk_context.info.initialized)
        return 0;
    
    __zencore_context__.vk_context.physical_device = VK_NULL_HANDLE;
    VkInstance instance = __zencore_context__.vk_context.instance;

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, NULL);

    if (device_count == 0) {
        log_error("Failed to find GPUs with Vulkan support.");
        return -1;
    }

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * device_count);
    if (vkEnumeratePhysicalDevices(instance, &device_count, devices) != VK_SUCCESS) {
        log_error("Failed to enumerate devices.");
        return -1;
    }

    // TODO : Add some form of ranking system and prioritize dedicated GPUs
    for (size_t d = 0; d < device_count; d++) {
        if (zen_vk_device_is_suitable(devices[d], context_index)) {
            __zencore_context__.vk_context.physical_device = devices[d];
            break;
        }
    }

    free(devices);

    if (__zencore_context__.vk_context.physical_device == VK_NULL_HANDLE) {
        log_error("Failed to find suitable physical device.");
        return -1;
    }

    return 0;

}

int zen_vk_create_logical_device(void) {

    if (__zencore_context__.vk_context.info.initialized)
        return 0;

    VkPhysicalDevice physical_device = __zencore_context__.vk_context.physical_device; 
    __zencore_context__.vk_context.device = VK_NULL_HANDLE;

    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_infos[2] ={ 
        (VkDeviceQueueCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = __zencore_context__.vk_context.graphics_family,
            .pQueuePriorities = &queue_priority,
            .queueCount = 1
        },
        (VkDeviceQueueCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = __zencore_context__.vk_context.present_family,
            .pQueuePriorities = &queue_priority,
            .queueCount = 1
        }
    };

    size_t extension_count = 0;
    const char** extensions = zen_vk_get_device_extensions(&extension_count);

    VkPhysicalDeviceFeatures device_features = {};
    VkDeviceCreateInfo create_info = (VkDeviceCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .enabledExtensionCount = (uint32_t)extension_count,
        .pQueueCreateInfos = queue_create_infos,
        .ppEnabledExtensionNames = extensions,
        .pEnabledFeatures = &device_features,
        .queueCreateInfoCount = 2,
        .enabledLayerCount = 0
    };

    if (vkCreateDevice(physical_device, &create_info, NULL, &__zencore_context__.vk_context.device) != VK_SUCCESS) {
        log_error("Failed to create logical device.");
        return -1;
    }

    vkGetDeviceQueue (
        __zencore_context__.vk_context.device, 
        __zencore_context__.vk_context.graphics_family, 
        0, &__zencore_context__.vk_context.graphics_queue
    );

    vkGetDeviceQueue (
        __zencore_context__.vk_context.device, 
        __zencore_context__.vk_context.present_family, 
        0, &__zencore_context__.vk_context.present_queue
    );

    return 0;

}

int zen_vk_create_swap_chain(size_t context_index) {

    ZEN_VulkanSurfaceInfo* info = &__zencore_context__.vk_context.surfaces[context_index];

    if (!zen_vk_query_swapchain_support (
        __zencore_context__.vk_context.physical_device, 
        context_index
    )) {
        log_error("Selected device does not support swapchain.");
        return -1;
    }

    VkSurfaceFormatKHR surface_format = zen_vk_choose_swap_surface_format(context_index);
    VkPresentModeKHR present_mode = zen_vk_choose_swap_present_mode(context_index);
    VkExtent2D extent = zen_vk_choose_swap_extent(context_index);

    uint32_t image_count = info->surface_capabilities.minImageCount + 1;
    if (info->surface_capabilities.maxImageCount > 0 && 
        image_count > info->surface_capabilities.maxImageCount)
        image_count = info->surface_capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR create_info = (VkSwapchainCreateInfoKHR) {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageColorSpace = surface_format.colorSpace,
        .surface = info->surface,
        .imageFormat = surface_format.format,
        .minImageCount = image_count,
        .imageExtent = extent,
        .imageArrayLayers = 1
    };

    uint32_t queue_family_indices[2] = {
        __zencore_context__.vk_context.graphics_family,
        __zencore_context__.vk_context.present_family 
    };

    if (queue_family_indices[0] != queue_family_indices[1]) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = NULL;
    }

    create_info.preTransform = info->surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.oldSwapchain = VK_NULL_HANDLE;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(__zencore_context__.vk_context.device, &create_info, NULL, &info->swap_chain) != VK_SUCCESS) {
        log_error("Failed to create swap chain.");
        return -1;
    }

    vkGetSwapchainImagesKHR(__zencore_context__.vk_context.device, info->swap_chain, &image_count, NULL);
    if (image_count == 0) {
        log_error("Failed to find any chain images.");
        return -1;
    }

    info->swap_chain_image_count = (size_t)image_count;
    info->swap_chain_images = malloc(sizeof(VkImage) * image_count);
    if (info->swap_chain_images == NULL) {
        log_error("Failed to allocate space for chain images.");
        return -1;
    }

    vkGetSwapchainImagesKHR (
        __zencore_context__.vk_context.device, 
        info->swap_chain, 
        &image_count, 
        info->swap_chain_images
    );

    info->swap_chain_extent = extent;
    info->swap_chain_image_format = surface_format.format;
    return 0;

}

int zen_vk_create_image_views(size_t context_index) {

    ZEN_VulkanSurfaceInfo* info = &__zencore_context__.vk_context.surfaces[context_index];

    info->swap_chain_image_view_count = 0;
    info->swap_chain_image_views = malloc(sizeof(VkImageView) * info->swap_chain_image_count);
    if (info->swap_chain_image_views == NULL) {
        log_error("Failed to allocate space for swap chain image views.");
        return -1;
    }

    for (size_t i = 0; i < info->swap_chain_image_count; i++) {
        
        VkImageViewCreateInfo create_info = (VkImageViewCreateInfo) {
            
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = info->swap_chain_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = info->swap_chain_image_format,
            
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1,

        };

        if (vkCreateImageView(__zencore_context__.vk_context.device, &create_info, NULL, &info->swap_chain_image_views[i]) != VK_SUCCESS) {
            log_error("Failed to create image views.");
            return -1;
        }

        info->swap_chain_image_view_count++;

    }

    return 0;


}

int zen_vk_create_render_pass(size_t context_index) {

    if (__zencore_context__.vk_context.info.initialized)
        return 0;

    ZEN_VulkanSurfaceInfo* info = &__zencore_context__.vk_context.surfaces[context_index];

    VkAttachmentDescription color_attachment = (VkAttachmentDescription) { 
        .format = info->swap_chain_image_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference color_attachment_ref = (VkAttachmentReference) {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = (VkSubpassDescription) {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, 
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref
    };

    VkSubpassDependency dependency = (VkSubpassDependency) {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        
    };

    VkRenderPassCreateInfo render_pass_info = (VkRenderPassCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };

    if (vkCreateRenderPass (
        __zencore_context__.vk_context.device, 
        &render_pass_info, NULL, 
        &__zencore_context__.vk_context.render_pass
    ) != VK_SUCCESS) {
        log_error("Failed to create render pass.");
        return -1;
    }

    return 0;

}

int zen_vk_create_default_shader(void) {

    ZEN_Shader shader = (ZEN_Shader) {
        .name = "main",
        .fragment_shader_path = "default",
        .vertex_shader_path = "default"
    };

    zen_append_shader(shader);
    return 0;

}

int zen_vk_create_graphics_pipelines(void) {

    if (__zencore_context__.vk_context.info.initialized)
        return 0;

    __zencore_context__.vk_context.graphics_pipelines = malloc(__zencore_context__.vk_context.shader_count * sizeof(ZEN_RenderPipline));
    if (__zencore_context__.vk_context.graphics_pipelines == NULL) {
        log_error("Failed to allocate space for piplines.");
        return -1;
    }

    for (size_t i = 0; i < __zencore_context__.vk_context.shader_count; i++)
        if (zen_vk_create_graphics_pipeline(__zencore_context__.vk_context.shaders[i]) < 0)
            return -1;
    
    return 0;

}

int zen_vk_create_graphics_pipeline(ZEN_Shader shader) {

    ZEN_RenderPipline* pipline = &__zencore_context__.vk_context.graphics_pipelines[__zencore_context__.vk_context.graphics_pipline_count];

    size_t vert_shader_buf_len = 0;
    size_t frag_shader_buf_len = 0;

    bool default_frag = (strcmp(shader.fragment_shader_path, "default") == 0);
    bool default_vert = (strcmp(shader.vertex_shader_path, "default") == 0);
  
    char* vert_shader_buf = default_vert ? ZEN_DEFAULT_VERTEX_SHADER : zen_read_file_contents(shader.vertex_shader_path, &vert_shader_buf_len);
    char* frag_shader_buf = default_frag ? ZEN_DEFAULT_FRAGMENT_SHADER : zen_read_file_contents(shader.fragment_shader_path, &frag_shader_buf_len);

    VkShaderModule vert_shader_module = zen_vk_create_shader_module(vert_shader_buf, vert_shader_buf_len);
    VkShaderModule frag_shader_module = zen_vk_create_shader_module(frag_shader_buf, frag_shader_buf_len);

    VkPipelineShaderStageCreateInfo vert_shader_stage_info = (VkPipelineShaderStageCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_shader_module,
        .pName = shader.name
    };

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = (VkPipelineShaderStageCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_shader_module,
        .pName = shader.name
    };

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

    VkDynamicState* dynamic_states = malloc(sizeof(VkDynamicState) * 2);
    if (dynamic_states == NULL) {
        log_error("Failed to allocate space for dynamic states.");
        return -1;
    }

    dynamic_states[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamic_states[1] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineDynamicStateCreateInfo dynamic_state = (VkPipelineDynamicStateCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamic_states
    };

    VkVertexInputAttributeDescription* attribute_descriptions = zen_vk_get_vertex_attribute_descriptions();
    VkVertexInputBindingDescription binding_description = zen_vk_get_vertex_binding_description();

    VkPipelineVertexInputStateCreateInfo vertex_input_info = (VkPipelineVertexInputStateCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding_description,
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = attribute_descriptions
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly = (VkPipelineInputAssemblyStateCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };
    
    VkPipelineViewportStateCreateInfo viewport_state = (VkPipelineViewportStateCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = (VkPipelineRasterizationStateCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampling = (VkPipelineMultisampleStateCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = (VkPipelineColorBlendAttachmentState) {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD
    };

    VkPipelineColorBlendStateCreateInfo color_blending = (VkPipelineColorBlendStateCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants[0] = 0.0f,
        .blendConstants[1] = 0.0f, 
        .blendConstants[2] = 0.0f,
        .blendConstants[3] = 0.0f 
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = (VkPipelineLayoutCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = NULL,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL
    };
    
    if (vkCreatePipelineLayout (
        __zencore_context__.vk_context.device, 
        &pipeline_layout_info, NULL, 
        &pipline->pipeline_layout
    ) != VK_SUCCESS) {
        log_error("Failed to create pipeline layout.");
        return -1;
    }

    VkGraphicsPipelineCreateInfo pipeline_info = (VkGraphicsPipelineCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = NULL,
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_state,
        .layout = pipline->pipeline_layout,
        .renderPass = __zencore_context__.vk_context.render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    if (vkCreateGraphicsPipelines(
        __zencore_context__.vk_context.device, 
        VK_NULL_HANDLE, 1, 
        &pipeline_info, NULL, 
        &pipline->graphics_pipeline) != VK_SUCCESS
    ) {
        log_error("Failed to create graphics pipeline.");
        return -1;
    }

    __zencore_context__.vk_context.graphics_pipline_count++;
    vkDestroyShaderModule(__zencore_context__.vk_context.device, vert_shader_module, NULL);
    vkDestroyShaderModule(__zencore_context__.vk_context.device, frag_shader_module, NULL);

    return 0;

}

int zen_vk_create_framebuffers(size_t context_index);
int zen_vk_create_command_buffers(size_t context_index);
int zen_vk_create_sync_objects(size_t context_index);

int zen_vk_create_command_pool(void);
int zen_vk_create_vertex_buffer(void);