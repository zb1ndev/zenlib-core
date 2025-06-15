#include "../../include/zenlib_core.h"

const char **zen_vk_get_instance_extensions(size_t *count) {

    #if defined(ZEN_OS_WINDOWS)

        static const char* extensions[] = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME    
        };

        *count = 2;
        return extensions;

    #endif // ZEN_OS_WINDOWS

}

const char **zen_vk_get_device_extensions(size_t *count) {

    #if defined(ZEN_OS_WINDOWS)

        static const char* extensions[] = {
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
    const char** req_extensions = zen_vk_get_device_extensions(&req_extension_count);

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
    return VK_PRESENT_MODE_IMMEDIATE_KHR;  // VK_PRESENT_MODE_FIFO_KHR;
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

VkVertexInputBindingDescription zen_vk_get_vertex_binding_description() {
        
    VkVertexInputBindingDescription binding_Description = (VkVertexInputBindingDescription) {
        .binding = 0,
        .stride = sizeof(ZEN_Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    return binding_Description;

}

VkVertexInputAttributeDescription* zen_vk_get_vertex_attribute_descriptions() {

    static VkVertexInputAttributeDescription attribute_descriptions[2] = { 
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

uint32_t zen_vk_find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) {

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(__zencore_context__.vk_context.physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
        if (typeFilter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    return -1;

}

int zen_vk_cleanup_swapchain(size_t context_index) {

    ZEN_VulkanSurfaceInfo* info = &__zencore_context__.vk_context.surfaces[context_index];

    for (size_t i = 0; i < info->swap_chain_image_view_count; i++)
        vkDestroyFramebuffer(__zencore_context__.vk_context.device, info->frame_buffers[i], NULL);
    free(info->frame_buffers);
    
    for (size_t i = 0; i < info->swap_chain_image_view_count; ++i)
        vkDestroyImageView(__zencore_context__.vk_context.device, info->swap_chain_image_views[i], NULL);
    free(info->swap_chain_image_views);

    vkDestroySwapchainKHR(__zencore_context__.vk_context.device, info->swap_chain, NULL);
    info->swap_chain = VK_NULL_HANDLE;

    return 0;

}

int zen_vk_recreate_swapchain(size_t context_index) {

    ZEN_VulkanSurfaceInfo* info = &__zencore_context__.vk_context.surfaces[context_index];
    while (info->window->event_handler.minimized) {
        if (zen_window_should_close(info->window))
            return -1;
        if (info->window->event_handler.background_callback != NULL)
            info->window->event_handler.background_callback(info->window);
    }

    vkDeviceWaitIdle(__zencore_context__.vk_context.device);
    zen_vk_cleanup_swapchain(context_index);

    if (zen_vk_create_swap_chain(context_index) < 0) {
        log_error("Failed to recreate swapchain.");
        return -1;
    }

    if (zen_vk_create_image_views(context_index) < 0) {
        log_error("Failed to recreate image views.");
        return -1;
    }
    
    if (zen_vk_create_framebuffers(context_index) < 0) {
        log_error("Failed to recreate frame buffers.");
        return -1;
    }

    return 0;

}

int zen_vk_resize_vertex_buffer(void) {

    if (!__zencore_context__.vk_context.info.initialized)
        return -1;

    vkQueueWaitIdle(__zencore_context__.vk_context.present_queue);

    // Destroy and free Vertex and Index Buffers and Memory
    vkDestroyBuffer(__zencore_context__.vk_context.device, __zencore_context__.vk_context.index_buffer, NULL);
    vkFreeMemory(__zencore_context__.vk_context.device, __zencore_context__.vk_context.index_buffer_memory, NULL);

    vkDestroyBuffer(__zencore_context__.vk_context.device, __zencore_context__.vk_context.vertex_buffer, NULL);
    vkFreeMemory(__zencore_context__.vk_context.device, __zencore_context__.vk_context.vertex_buffer_memory, NULL);

    // Recreate 
    if (zen_vk_create_vertex_buffer() < 0)
        return -1;

    if (zen_vk_create_index_buffer() < 0)
        return -1;

    return 0;

}

int zen_vk_append_graphics_pipeline(size_t shader_index) {

    ZEN_VulkanRenderPipline* temp = (ZEN_VulkanRenderPipline*)realloc (
        __zencore_context__.vk_context.graphics_pipelines,
        sizeof(ZEN_VulkanRenderPipline) * __zencore_context__.renderer_context.shader_count 
    );

    if (temp == NULL) {
        log_error("Failed to allocate space for graphics pipeline.");
        return -1;
    }

    __zencore_context__.vk_context.graphics_pipelines = temp;
    zen_vk_create_graphics_pipeline(&__zencore_context__.renderer_context.shaders[shader_index]);

    return 0;

}

int zen_vk_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* buffer_memory) {

    VkBufferCreateInfo buffer_info = (VkBufferCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    
    if (vkCreateBuffer(__zencore_context__.vk_context.device, &buffer_info, NULL, buffer) != VK_SUCCESS) {
        log_error("Failed to create buffer.");
        return -1;
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(__zencore_context__.vk_context.device, *buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = (VkMemoryAllocateInfo) {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = zen_vk_find_memory_type(mem_requirements.memoryTypeBits, properties)
    };

    if (vkAllocateMemory(__zencore_context__.vk_context.device, &alloc_info, NULL, buffer_memory) != VK_SUCCESS) {
        log_error("Failed to allocate buffer memory.");
        return -1;
    }

    vkBindBufferMemory(__zencore_context__.vk_context.device, *buffer, *buffer_memory, 0);
    return 0;

}

int zen_vk_copy_buffer(VkBuffer src_buffer, VkBuffer dest_buffer, VkDeviceSize size) {

    VkCommandBufferAllocateInfo alloc_info = (VkCommandBufferAllocateInfo) {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = __zencore_context__.vk_context.command_pool,
        .commandBufferCount = 1
    };
    
    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(__zencore_context__.vk_context.device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = (VkCommandBufferBeginInfo) {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkBufferCopy copy_region = (VkBufferCopy) {
        .srcOffset = 0, // Optional
        .dstOffset = 0, // Optional
        .size = size
    };
    vkCmdCopyBuffer(command_buffer, src_buffer, dest_buffer, 1, &copy_region);
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = (VkSubmitInfo) {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer
    };
    

    vkQueueSubmit(__zencore_context__.vk_context.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(__zencore_context__.vk_context.graphics_queue);

    vkFreeCommandBuffers (
        __zencore_context__.vk_context.device, 
        __zencore_context__.vk_context.command_pool, 
        1, 
        &command_buffer
    );

    return 0;

}