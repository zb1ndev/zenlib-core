#include "../zenlib_core.h"

int zen_init_vulkan(ZEN_Window* window, uint32_t api_version) {

    __zencore_context__.renderer_context.render_object_last_count = __zencore_context__.renderer_context.render_object_count;

    // If the Surfaces array is not initialized, initialize it
    if (__zencore_context__.vk_context.surfaces == NULL) {
        __zencore_context__.vk_context.surfaces = malloc(sizeof(ZEN_VulkanSurfaceInfo) * ZEN_MAX_WINDOWS);
        if (__zencore_context__.vk_context.surfaces == NULL) {
            printf(ERRORF "Failed to allocate space for window inside Vulkan context.\n");
            return -1;
        }
    }

    // Update API version and bind window to renderer context.
    __zencore_context__.vk_context.info.version = api_version;
    window->renderer_context_index = __zencore_context__.vk_context.surface_count++;
    __zencore_context__.vk_context.surfaces[window->renderer_context_index].window = window;

    if (zen_vk_create_instance() < 0) {
        printf(ERRORF "Failed to create Vulkan instance.\n");
        return -1;
    }

    if (zen_vk_create_surface(window->renderer_context_index) < 0) {
        printf(ERRORF "Failed to create Vulkan surface for window %llu.\n", window->renderer_context_index);
        return -1;
    }

    if (zen_vk_pick_physical_device(window->renderer_context_index) < 0) {
        printf(ERRORF "Failed to pick a suitable device.\n");
        return -1;
    }

    if (zen_vk_create_logical_device() < 0) {
        printf(ERRORF "Failed to create a logical device.\n");
        return -1;
    }

    if (zen_vk_create_swap_chain(window->renderer_context_index) < 0) {
        printf(ERRORF "Failed to create swapchain for window %llu.\n", window->renderer_context_index);
        return -1;
    }

    if (zen_vk_create_image_views(window->renderer_context_index) < 0) {
        printf(ERRORF "Failed to create image views for window %llu.\n", window->renderer_context_index);
        return -1;
    }

    if (zen_vk_create_render_pass(window->renderer_context_index) < 0) {
        printf(ERRORF "Failed to create render pass.\n");
        return -1;
    }

    if (zen_vk_create_graphics_pipelines() < 0) {
        printf(ERRORF "Failed to create graphics pipelines.\n");
        return -1;
    }
    
    if (zen_vk_create_framebuffers(window->renderer_context_index) < 0) {
        printf(ERRORF "Failed to create frame buffers for window %llu.\n", window->renderer_context_index);
        return -1;
    }
    
    if (zen_vk_create_command_pool() < 0) {
        printf(ERRORF "Failed to create command pool.\n");
        return -1;
    }

    if (!__zencore_context__.vk_context.info.initialized && zen_vk_create_vertex_buffer() < 0) {
        printf(ERRORF "Failed to create vertex buffer.\n");
        return -1;
    }

    if (!__zencore_context__.vk_context.info.initialized && zen_vk_create_index_buffer() < 0) {
        printf(ERRORF "Failed to create index buffer.\n");
        return -1;
    }

    if (zen_vk_create_command_buffers(window->renderer_context_index) < 0) {
        printf(ERRORF "Failed to create command buffers for window %llu.\n", window->renderer_context_index);
        return -1;
    }

    if (zen_vk_create_sync_objects(window->renderer_context_index) < 0) {
        printf(ERRORF "Failed to create sync objects for window %llu.\n", window->renderer_context_index);
        return -1;
    }

    __zencore_context__.vk_context.info.initialized = true;
    return 0;

}

void zen_destroy_vulkan(bool is_last, size_t context_index) {
    
    ZEN_VulkanSurfaceInfo* info = &__zencore_context__.vk_context.surfaces[context_index];

    vkDeviceWaitIdle(__zencore_context__.vk_context.device);

    for (size_t i = 0; i < ZEN_MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(__zencore_context__.vk_context.device, info->render_finished_semaphores[i], NULL);
        vkDestroySemaphore(__zencore_context__.vk_context.device, info->image_available_semaphores[i], NULL);
        vkDestroyFence(__zencore_context__.vk_context.device, info->in_flight_fences[i], NULL);
    }

    free(info->image_available_semaphores);
    free(info->render_finished_semaphores);
    free(info->in_flight_fences);

    free(info->command_buffers);

    zen_vk_cleanup_swapchain(context_index);
    
    free(info->swap_chain_images);
    free(info->present_modes);
    free(info->surface_formats);

    vkDestroySurfaceKHR(__zencore_context__.vk_context.instance, info->surface, NULL);

    if (is_last) {

        for (size_t i = 0; i < __zencore_context__.vk_context.graphics_pipline_count; ++i) {
            
            vkDestroyPipeline ( 
                __zencore_context__.vk_context.device,  
                __zencore_context__.vk_context.graphics_pipelines[i].graphics_pipeline, 
                NULL
            );

            vkDestroyPipelineLayout (
                __zencore_context__.vk_context.device, 
                __zencore_context__.vk_context.graphics_pipelines[i].pipeline_layout, 
                NULL
            );

        }

        free(__zencore_context__.vk_context.graphics_pipelines);
        free(__zencore_context__.renderer_context.shaders);

        vkDestroyRenderPass(__zencore_context__.vk_context.device, __zencore_context__.vk_context.render_pass, NULL);
        vkDestroyCommandPool(__zencore_context__.vk_context.device, __zencore_context__.vk_context.command_pool, NULL);

        vkDestroyBuffer(__zencore_context__.vk_context.device, __zencore_context__.vk_context.vertex_buffer, NULL);
        vkFreeMemory(__zencore_context__.vk_context.device, __zencore_context__.vk_context.vertex_buffer_memory, NULL);
        
        vkDestroyBuffer(__zencore_context__.vk_context.device, __zencore_context__.vk_context.index_buffer, NULL);
        vkFreeMemory(__zencore_context__.vk_context.device, __zencore_context__.vk_context.index_buffer_memory, NULL);
        
        vkDestroyDevice(__zencore_context__.vk_context.device, NULL);
        vkDestroyInstance(__zencore_context__.vk_context.instance, NULL);

    }

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
        printf(ERRORF "Failed to create vulkan instance.\n");
        return -1;
    }

    return 0;

}

int zen_vk_create_surface(size_t context_index) {

    #if defined(ZEN_OS_WINDOWS)

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
            printf(ERRORF "Failed to create surface.\n");
            return -1;
        }

        return 0;

    #endif

    return -1;

}

int zen_vk_pick_physical_device(size_t context_index) {

    if (__zencore_context__.vk_context.info.initialized)
        return 0;
    
    __zencore_context__.vk_context.physical_device = VK_NULL_HANDLE;
    VkInstance instance = __zencore_context__.vk_context.instance;

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, NULL);

    if (device_count == 0) {
        printf(ERRORF "Failed to find GPUs with Vulkan support.\n");
        return -1;
    }

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * device_count);
    if (vkEnumeratePhysicalDevices(instance, &device_count, devices) != VK_SUCCESS) {
        printf(ERRORF "Failed to enumerate devices.\n");
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
        printf(ERRORF "Failed to find suitable physical device.\n");
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
        printf(ERRORF "Failed to create logical device.\n");
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
        printf(ERRORF "Selected device does not support swapchain.\n");
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
        printf(ERRORF "Failed to create swap chain.\n");
        return -1;
    }

    vkGetSwapchainImagesKHR(__zencore_context__.vk_context.device, info->swap_chain, &image_count, NULL);
    if (image_count == 0) {
        printf(ERRORF "Failed to find any chain images.\n");
        return -1;
    }

    info->swap_chain_image_count = (size_t)image_count;
    info->swap_chain_images = malloc(sizeof(VkImage) * image_count);
    if (info->swap_chain_images == NULL) {
        printf(ERRORF "Failed to allocate space for chain images.\n");
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
        printf(ERRORF "Failed to allocate space for swap chain image views.\n");
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
            printf(ERRORF "Failed to create image views.\n");
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
        printf(ERRORF "Failed to create render pass.\n");
        return -1;
    }

    return 0;

}

int zen_vk_create_graphics_pipelines(void) {

    if (__zencore_context__.vk_context.info.initialized)
        return 0;

    __zencore_context__.vk_context.graphics_pipelines = malloc(__zencore_context__.renderer_context.shader_count * sizeof(ZEN_VulkanRenderPipline));
    if (__zencore_context__.vk_context.graphics_pipelines == NULL) {
        printf(ERRORF "Failed to allocate space for piplines.\n");
        return -1;
    }

    for (size_t i = 0; i < __zencore_context__.renderer_context.shader_count; i++)
        if (zen_vk_create_graphics_pipeline(&__zencore_context__.renderer_context.shaders[i]) < 0)
            return -1;
    return 0;

}

int zen_vk_create_graphics_pipeline(ZEN_Shader* shader) {

    ZEN_VulkanRenderPipline* pipline = &__zencore_context__.vk_context.graphics_pipelines[__zencore_context__.vk_context.graphics_pipline_count];
    
    // const char* fragment_shader_path = "./examples/vulkan/shaders/compiled/frag.spv";
    // const char* vertex_shader_path = "./examples/vulkan/shaders/compiled/vert.spv";

    size_t vert_shader_buf_len = 0;
    size_t frag_shader_buf_len = 0;

    char* vert_shader_buf = zen_read_file_contents(shader->vertex_shader_path, &vert_shader_buf_len);
    char* frag_shader_buf = zen_read_file_contents(shader->fragment_shader_path, &frag_shader_buf_len);

    VkShaderModule vert_shader_module = zen_vk_create_shader_module(vert_shader_buf, vert_shader_buf_len);
    VkShaderModule frag_shader_module = zen_vk_create_shader_module(frag_shader_buf, frag_shader_buf_len);

    VkPipelineShaderStageCreateInfo vert_shader_stage_info = (VkPipelineShaderStageCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_shader_module,
        .pName = "main"
    };

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = (VkPipelineShaderStageCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_shader_module,
        .pName = "main"
    };

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

    VkDynamicState* dynamic_states = malloc(sizeof(VkDynamicState) * 2);
    if (dynamic_states == NULL) {
        printf(ERRORF "Failed to allocate space for dynamic states.\n");
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

    VkPushConstantRange push_constant_range = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(mat4)
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = (VkPipelineLayoutCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = NULL,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant_range
    };
    
    if (vkCreatePipelineLayout (
        __zencore_context__.vk_context.device, 
        &pipeline_layout_info, NULL, 
        &pipline->pipeline_layout
    ) != VK_SUCCESS) {
        printf(ERRORF "Failed to create pipeline layout.\n");
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
        printf(ERRORF "Failed to create graphics pipeline.\n");
        return -1;
    }
    
    shader->pipeline = __zencore_context__.vk_context.graphics_pipline_count;

    __zencore_context__.vk_context.graphics_pipline_count++;
    vkDestroyShaderModule(__zencore_context__.vk_context.device, vert_shader_module, NULL);
    vkDestroyShaderModule(__zencore_context__.vk_context.device, frag_shader_module, NULL);

    return 0;

}

int zen_vk_create_framebuffers(size_t context_index) {

    ZEN_VulkanSurfaceInfo* info = &__zencore_context__.vk_context.surfaces[context_index];

    info->frame_buffers = malloc(sizeof(VkFramebuffer) * info->swap_chain_image_view_count);
    if (info->frame_buffers == NULL) {
        printf(ERRORF "Failed to allocate space for frame buffers.\n");
        return -1;
    }

    for (size_t i = 0; i < info->swap_chain_image_view_count; i++) {
            
        VkImageView attachments[] = {
            info->swap_chain_image_views[i]
        };

        VkFramebufferCreateInfo framebuffer_info = (VkFramebufferCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = __zencore_context__.vk_context.render_pass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = info->swap_chain_extent.width,
            .height = info->swap_chain_extent.height,
            .layers = 1
        };

        if (vkCreateFramebuffer(
            __zencore_context__.vk_context.device, 
            &framebuffer_info, NULL, 
            &info->frame_buffers[i]
        ) != VK_SUCCESS) {
            printf(ERRORF "Failed to create framebuffer.\n");
            return -1;
        }

    }

    return 0;

}

int zen_vk_create_command_pool(void) {

    if (__zencore_context__.vk_context.info.initialized)
        return 0;

    VkCommandPoolCreateInfo pool_info = (VkCommandPoolCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = __zencore_context__.vk_context.graphics_family
    };

    if (vkCreateCommandPool(
        __zencore_context__.vk_context.device, 
        &pool_info, NULL, 
        &__zencore_context__.vk_context.command_pool
    ) != VK_SUCCESS) {
        printf(ERRORF "Failed to create command pool.\n");
        return -1;
    }

    return 0;

}

int zen_vk_create_vertex_buffer(void) {

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    uint64_t count = zen_get_vertex_count();
    if (count == 0) count = 3;
    
    uint64_t size = sizeof(ZEN_Vertex) * count;

    zen_vk_create_buffer (size, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        &staging_buffer, 
        &staging_buffer_memory
    );

    void* data;
    vkMapMemory(__zencore_context__.vk_context.device, staging_buffer_memory, 0, size, 0, &data);
        memcpy(data, zen_get_vertices(), (size_t)size);
    vkUnmapMemory(__zencore_context__.vk_context.device, staging_buffer_memory);

    zen_vk_create_buffer (
        size, 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        &__zencore_context__.vk_context.vertex_buffer, 
        &__zencore_context__.vk_context.vertex_buffer_memory
    );

    zen_vk_copy_buffer(staging_buffer, __zencore_context__.vk_context.vertex_buffer, size);
    vkDestroyBuffer(__zencore_context__.vk_context.device, staging_buffer, NULL);
    vkFreeMemory(__zencore_context__.vk_context.device, staging_buffer_memory, NULL);

    return 0;

}

int zen_vk_create_index_buffer(void) {

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    uint64_t count = zen_get_index_count();
    if (count == 0) count = 3;

    VkDeviceSize buffer_size = sizeof(uint16_t) * count;

    zen_vk_create_buffer (
        buffer_size, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        &staging_buffer, 
        &staging_buffer_memory
    );

    void* data;
    vkMapMemory(__zencore_context__.vk_context.device, staging_buffer_memory, 0, buffer_size, 0, &data);
        memcpy(data, zen_get_indices(), (size_t)buffer_size);
    vkUnmapMemory(__zencore_context__.vk_context.device, staging_buffer_memory);
    
    zen_vk_create_buffer (
        buffer_size, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        &__zencore_context__.vk_context.index_buffer, 
        &__zencore_context__.vk_context.index_buffer_memory
    );
    
    zen_vk_copy_buffer(staging_buffer, __zencore_context__.vk_context.index_buffer, buffer_size);
    vkDestroyBuffer(__zencore_context__.vk_context.device, staging_buffer, NULL);
    vkFreeMemory(__zencore_context__.vk_context.device, staging_buffer_memory, NULL);

    return 0;

}

int zen_vk_create_command_buffers(size_t context_index) {

    ZEN_VulkanSurfaceInfo* info = &__zencore_context__.vk_context.surfaces[context_index];

    info->command_buffers = malloc(sizeof(VkCommandBuffer) * ZEN_MAX_FRAMES_IN_FLIGHT);
    if (info->command_buffers == NULL) {
        printf(ERRORF "Failed to allocate space for command buffers.\n");
        return -1;
    }

    VkCommandBufferAllocateInfo alloc_info = (VkCommandBufferAllocateInfo) {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = __zencore_context__.vk_context.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = ZEN_MAX_FRAMES_IN_FLIGHT
    };

    if (vkAllocateCommandBuffers(
        __zencore_context__.vk_context.device, 
        &alloc_info, 
        info->command_buffers
    ) != VK_SUCCESS) {
        printf(ERRORF "Failed to allocate command buffers.\n");
        return -1;
    }

    return 0;

}

int zen_vk_create_sync_objects(size_t context_index) {

    ZEN_VulkanSurfaceInfo* info = &__zencore_context__.vk_context.surfaces[context_index];

    info->image_available_semaphores = malloc(sizeof(VkSemaphore) * ZEN_MAX_FRAMES_IN_FLIGHT);
    if (info->image_available_semaphores == NULL) {
        printf(ERRORF "Failed to allocate space for command buffers.\n");
        return -1;
    }
    info->render_finished_semaphores = malloc(sizeof(VkSemaphore) * ZEN_MAX_FRAMES_IN_FLIGHT);
    if (info->render_finished_semaphores == NULL) {
        printf(ERRORF "Failed to allocate space for command buffers.\n");
        return -1;
    }
    info->in_flight_fences = malloc(sizeof(VkFence) * ZEN_MAX_FRAMES_IN_FLIGHT);
    if (info->in_flight_fences == NULL) {
        printf(ERRORF "Failed to allocate space for command buffers.\n");
        return -1;
    }

    VkSemaphoreCreateInfo semaphore_info = (VkSemaphoreCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fence_info = (VkFenceCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (size_t i = 0; i < ZEN_MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(__zencore_context__.vk_context.device, &semaphore_info, NULL, &info->image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(__zencore_context__.vk_context.device, &semaphore_info, NULL, &info->render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(__zencore_context__.vk_context.device, &fence_info, NULL, &info->in_flight_fences[i]) != VK_SUCCESS) {
            printf(ERRORF "Failed to create semaphores.\n");
            return -1;
        }
    }

    return 0;

}