#include "../zenlib_core.h"

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
        printf(ERRORF "Failed to get physical device queue family properties.\n");
        return false;
    }

    VkQueueFamilyProperties* families = malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
    if (families == NULL) {
        printf(ERRORF "Faild to allocate space for queue family properties.\n");
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
        printf(ERRORF "Failed to find any formats.\n");
        return false;
    }
    info->surface_format_count = (size_t)format_count;

    info->surface_formats = malloc(sizeof(VkSurfaceFormatKHR) * format_count);
    if (info->surface_formats == NULL) {
        printf(ERRORF "Failed to allocate space for surface formats.\n");
        return false;
    }

    if (vkGetPhysicalDeviceSurfaceFormatsKHR (
        device, info->surface, 
        &format_count, info->surface_formats
    ) != VK_SUCCESS) {
        printf(ERRORF "Failed to get surface formats.\n");
        return false;
    }

    // Query Present Modes
    uint32_t present_mode_count;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR (
        device, info->surface, 
        &present_mode_count, NULL
    ) != VK_SUCCESS) {
        printf(ERRORF "Failed to find any present modes.\n");
        return false;
    }
    info->present_mode_count = (size_t)present_mode_count;

    info->present_modes = malloc(sizeof(VkPresentModeKHR) * present_mode_count);
    if (info->present_modes == NULL) {
        printf(ERRORF "Failed to allocate space for surface formats.\n");
        return false;
    }

    if (vkGetPhysicalDeviceSurfacePresentModesKHR (
        device, 
        info->surface, 
        &present_mode_count, 
        info->present_modes
    ) != VK_SUCCESS) {
        printf(ERRORF "Failed to get present modes.\n");
        return false;
    }

    return true;

}

bool zen_vk_device_has_extensions(VkPhysicalDevice device) {

    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);
    if (extension_count == 0) {
        printf(ERRORF "Failed to enumerate device extensions.\n");
        return false;
    }

    VkExtensionProperties* extension_props = malloc(sizeof(VkExtensionProperties) * extension_count);
    if (extension_props == NULL) {
        printf(ERRORF "Failed to allocate space for extension properties.\n");
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
            printf(ERRORF "Failed to find device with required extensions.\n");
            return false;
        }
        
    }

    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(device, &supported_features);
    if (!supported_features.samplerAnisotropy) return false;
    
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
        printf(ERRORF "Failed to create shader module.\n");
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

    static VkVertexInputAttributeDescription attribute_descriptions[3] = { 
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
        },
        (VkVertexInputAttributeDescription) {
            .binding = 0,
            .location = 2,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = zen_vk_offset_of(ZEN_Vertex, tex_coord)
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
        printf(ERRORF "Failed to recreate swapchain.\n");
        return -1;
    }

    if (zen_vk_create_image_views(context_index) < 0) {
        printf(ERRORF "Failed to recreate image views.\n");
        return -1;
    }
    
    if (zen_vk_create_framebuffers(context_index) < 0) {
        printf(ERRORF "Failed to recreate frame buffers.\n");
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
        printf(ERRORF "Failed to allocate space for graphics pipeline.\n");
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
        printf(ERRORF "Failed to create buffer.\n");
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
        printf(ERRORF "Failed to allocate buffer memory.\n");
        return -1;
    }

    vkBindBufferMemory(__zencore_context__.vk_context.device, *buffer, *buffer_memory, 0);
    return 0;

}

int zen_vk_copy_buffer(VkBuffer src_buffer, VkBuffer dest_buffer, VkDeviceSize size) {

    VkCommandBuffer command_buffer = zen_vk_begin_single_command();
    VkBufferCopy copy_region = (VkBufferCopy) {
        .size = size
    };

    vkCmdCopyBuffer(command_buffer, src_buffer, dest_buffer, 1, &copy_region);
    zen_vk_end_single_command(command_buffer);

    return 0;

}

int zen_vk_create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* image_memory) {

    ZEN_VulkanContext* context = &__zencore_context__.vk_context;

    VkImageCreateInfo image_info = (VkImageCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = width,
        .extent.height = height,
        .extent.depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = format,
        .tiling = tiling,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = usage,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if (vkCreateImage(context->device, &image_info, NULL, image) != VK_SUCCESS) {
        printf(ERRORF "Failed to create vulkan image.\n");
        return -1;
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(context->device, *image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = (VkMemoryAllocateInfo) {

        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = zen_vk_find_memory_type(mem_requirements.memoryTypeBits, properties)

    };

    if (vkAllocateMemory(context->device, &alloc_info, NULL, image_memory) != VK_SUCCESS) {
        printf(ERRORF "Failed to allocate memory for vulkan image.");
        return -1;
    }

    vkBindImageMemory(context->device, *image, *image_memory, 0);

    return 0;

}

VkImageView zen_vk_create_image_view(VkImage image, VkFormat format) {

    ZEN_VulkanContext* context = &__zencore_context__.vk_context;

    VkImageViewCreateInfo view_info = (VkImageViewCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1
    };
    
    VkImageView result;
    if (vkCreateImageView(context->device, &view_info, NULL, &result) != VK_SUCCESS) {
        printf(ERRORF "Failed to create texture image view.\n");
        return VK_NULL_HANDLE;
    }

    return result;

}

VkCommandBuffer zen_vk_begin_single_command(void) {

    ZEN_VulkanContext* context = &__zencore_context__.vk_context;

    VkCommandBufferAllocateInfo alloc_info = (VkCommandBufferAllocateInfo) {

        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = context->command_pool,
        .commandBufferCount = 1

    };

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(context->device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = (VkCommandBufferBeginInfo) {

        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT

    };

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;

}

void zen_vk_end_single_command(VkCommandBuffer command_buffer) {

    ZEN_VulkanContext* context = &__zencore_context__.vk_context;
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = (VkSubmitInfo) {

        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer


    };

    vkQueueSubmit(context->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(context->graphics_queue);

    vkFreeCommandBuffers(context->device, context->command_pool, 1, &command_buffer);

}

void zen_vk_transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) {

    (void)format;

    VkCommandBuffer command_buffer = zen_vk_begin_single_command();

    VkImageMemoryBarrier barrier = (VkImageMemoryBarrier) {

        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
        .srcAccessMask = 0,
        .dstAccessMask = 0

    };

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    
    } else {
        printf(ERRORF "Unsupported layout transition.\n");
        return;
    }

    vkCmdPipelineBarrier (
        command_buffer,
        source_stage, destination_stage,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );

    zen_vk_end_single_command(command_buffer);

}

void zen_vk_copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {

    VkCommandBuffer command_buffer = zen_vk_begin_single_command();

    VkBufferImageCopy region = (VkBufferImageCopy) {

        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .imageSubresource.mipLevel = 0,
        .imageSubresource.baseArrayLayer = 0,
        .imageSubresource.layerCount = 1,
        .imageOffset = {0, 0, 0},
        .imageExtent = {
            width,
            height,
            1
        }

    };

    vkCmdCopyBufferToImage(
        command_buffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
        
    zen_vk_end_single_command(command_buffer);

}

int zen_vk_create_descriptor_pool(void) {

    ZEN_VulkanContext* context = &__zencore_context__.vk_context;
    if (context->descriptor_pool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(context->device, context->descriptor_pool, NULL);

    VkDescriptorPoolSize pool_size = (VkDescriptorPoolSize) {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = context->texture_count + 1
    };

    VkDescriptorPoolCreateInfo pool_info = (VkDescriptorPoolCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size,
        .maxSets = context->texture_capacity
    };

    if (vkCreateDescriptorPool(context->device, &pool_info, NULL, &context->descriptor_pool) != VK_SUCCESS) {
        printf(ERRORF "Failed to create descriptor pool.\n");
        return -1;
    }

    return 0;

}

int zen_vk_create_descriptor_set(size_t index) {

    ZEN_VulkanContext* context = &__zencore_context__.vk_context;
        
    VkDescriptorSetAllocateInfo alloc_info = (VkDescriptorSetAllocateInfo) {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = context->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &context->descriptor_set_layout
    };
    
    if (vkAllocateDescriptorSets(context->device, &alloc_info, &context->textures[index].descriptor) != VK_SUCCESS) {
        printf(ERRORF "Failed to allocate descriptor sets.\n");
        return -1;
    }

    VkDescriptorImageInfo image_info = (VkDescriptorImageInfo) {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView = context->textures[index].view,
        .sampler = context->texture_sampler
    };

    VkWriteDescriptorSet write_info = (VkWriteDescriptorSet) {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = context->textures[index].descriptor,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImageInfo = &image_info
    };

    vkUpdateDescriptorSets(context->device, 1, &write_info, 0, NULL);

    return 0;

}

size_t zen_vk_upload_image_data(byte* raw_image_data, size_t width, size_t height) {

    ZEN_VulkanContext* context = &__zencore_context__.vk_context;

    bool resized = false;
    if ((context->texture_count + 1) >= context->texture_capacity) {

        context->texture_capacity = (context->texture_count + 1) * 1.5f;
        ZEN_VulkanTexture* temp = realloc(context->textures, sizeof(ZEN_VulkanTexture) * context->texture_capacity);
        
        if (temp == NULL) { 
            printf(ERRORF "Failed to allocate space for textures.\n");       
            return 0;
        }

        context->textures = temp;

        if (zen_vk_create_descriptor_pool() < 0) {
            printf(ERRORF "Failed to create / recreate descriptor pool.\n");
            return -1;
        }

        resized = true;

    }

    VkDeviceSize image_size = width * height * 4;
    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;

    if (zen_vk_create_buffer (
        image_size, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        &staging_buffer, 
        &staging_memory
    ) < 0) {
        printf(ERRORF "Failed to create staging buffer for image.\n");
        return 0;
    }

    void* data;
    vkMapMemory(context->device, staging_memory, 0, image_size, 0, &data);
        memcpy(data, raw_image_data, (size_t)image_size);
    vkUnmapMemory(context->device, staging_memory);

    if (zen_vk_create_image (
        width,
        height,
        VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        &context->textures[context->texture_count].image,
        &context->textures[context->texture_count].memory
    ) < 0) {
        printf(ERRORF "Failed to create staging vulkan image.\n");
        return 0;
    }    

    zen_vk_transition_image_layout(context->textures[context->texture_count].image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    zen_vk_copy_buffer_to_image(staging_buffer, context->textures[context->texture_count].image, (uint32_t)width, (uint32_t)height);
    zen_vk_transition_image_layout(context->textures[context->texture_count].image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(context->device, staging_buffer, NULL);
    vkFreeMemory(context->device, staging_memory, NULL);

    context->textures[context->texture_count].view = zen_vk_create_image_view(context->textures[context->texture_count].image, VK_FORMAT_R8G8B8A8_SRGB);
    context->texture_count++;

    if (context->info.initialized) {
        if (resized) zen_vk_create_descriptor_sets();
        else zen_vk_create_descriptor_set(context->texture_count - 1);
    }

    return context->texture_count - 1;

}