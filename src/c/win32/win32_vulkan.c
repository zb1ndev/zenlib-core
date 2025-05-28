#include "../../include/zenlib_core.h"

#if defined(ZEN_OS_WINDOWS)

    #pragma region Utilities

        void zen_set_vulkan_extensions(ZEN_VulkanInfo info) {
            __zencore_context__.vk_context.info = info;
        }

        bool zen_vk_is_device_suitable(ZEN_Window* window, VkPhysicalDevice device) {

            if (zen_vk_find_queue_families(window, device) < 0)
                return false;
            if (zen_vk_device_has_extensions(device) < 0) 
                return false;
            if (zen_vk_query_swapchain_support(window, device) < 0)
                return false;

            return true;

        }

        int zen_vk_find_queue_families(ZEN_Window* window, VkPhysicalDevice device) {

            uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

            if (queue_family_count == 0) {
                log_error("Failed to get physical device queue family properties.");
                return -1;
            }

            VkQueueFamilyProperties* families = malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
            if (families == NULL) {
                log_error("Faild to allocate space for queue family properties.");
                return -1;
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
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, window->vk_context.surface, &presentSupport);
                if (presentSupport) {
                    __zencore_context__.vk_context.present_family = i;
                    found_presentation_family = true;
                }

            }
            free(families);

            if (!(found_graphics_family && found_presentation_family))
                return -1;

            return 0;

        }

        int zen_vk_device_has_extensions(VkPhysicalDevice device) {

            uint32_t extension_count = 0;
            vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);
            if (extension_count == 0) {
                log_error("Failed to enumerate device extensions.");
                return -1;
            }

            VkExtensionProperties* extension_props = malloc(sizeof(VkExtensionProperties) * extension_count);
            if (extension_props == NULL) {
                log_error("Failed to allocate space for extension properties.");
                return -1;
            }

            vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, extension_props);
            for (size_t r = 0; r < __zencore_context__.vk_context.info.device_extension_count; r++) {
                
                bool found_extension = false;
                for (size_t i = 0; i < extension_count; i++) {
                    if (strcmp(__zencore_context__.vk_context.info.device_extensions[r], 
                        extension_props[i].extensionName) == 0
                    ) found_extension = true;
                }

                if (found_extension == false) {
                    log_error("Failed to find device with required extensions.");
                    return -1;
                }
                
            }
            
            return 0;

        }

        int zen_vk_query_swapchain_support(ZEN_Window* window, VkPhysicalDevice device) {

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR (
                device, 
                window->vk_context.surface, 
                &window->vk_context.surface_capabilities
            );

            uint32_t format_count;
            vkGetPhysicalDeviceSurfaceFormatsKHR (
                device, 
                window->vk_context.surface, 
                &format_count, 
                NULL
            );
            window->vk_context.surface_format_count = format_count;

            if (format_count != 0) {

                window->vk_context.surface_formats = malloc(sizeof(VkSurfaceFormatKHR) * format_count);
                if (window->vk_context.surface_formats == NULL) {
                    log_error("Failed to allocate space for surface formats.");
                    return -1;
                }

                vkGetPhysicalDeviceSurfaceFormatsKHR (
                    device, 
                    window->vk_context.surface, 
                    &format_count, 
                    window->vk_context.surface_formats
                );

            }

            uint32_t present_mode_count;
            vkGetPhysicalDeviceSurfacePresentModesKHR (
                device, 
                window->vk_context.surface, 
                &present_mode_count, 
                NULL
            );
            window->vk_context.present_mode_count = present_mode_count;

            if (present_mode_count != 0) {

                window->vk_context.present_modes = malloc(sizeof(VkPresentModeKHR) * present_mode_count);
                if (window->vk_context.present_modes == NULL) {
                    log_error("Failed to allocate space for surface formats.");
                    return -1;
                }

                vkGetPhysicalDeviceSurfacePresentModesKHR (
                    device, 
                    window->vk_context.surface, 
                    &present_mode_count, 
                    window->vk_context.present_modes
                );

            }

            return 0;

        }

        VkSurfaceFormatKHR zen_vk_choose_swap_surface_format(ZEN_Window* window) {

            for (size_t i = 0; i < window->vk_context.surface_format_count; i++) {
                if (window->vk_context.surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && 
                    window->vk_context.surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return window->vk_context.surface_formats[i];
            }

            return window->vk_context.surface_formats[0];

        }

        VkPresentModeKHR zen_vk_choose_swap_present_mode(ZEN_Window* window) {

            for (size_t i = 0; i < window->vk_context.present_mode_count; i++) {
                if (window->vk_context.present_modes[i] == __zencore_context__.vk_context.info.mode)
                    return window->vk_context.present_modes[i];
            }
            
            return VK_PRESENT_MODE_FIFO_KHR;

        }

        VkExtent2D zen_vk_choose_swap_extent(ZEN_Window* window) {

            VkSurfaceCapabilitiesKHR capabilities = window->vk_context.surface_capabilities;

            if (capabilities.currentExtent.width != UINT32_MAX)
                return capabilities.currentExtent;

            VkExtent2D actual_extent = {
                (uint32_t)window->width,
                (uint32_t)window->height
            };

            actual_extent.width =  (actual_extent.width > capabilities.maxImageExtent.width)   ? capabilities.maxImageExtent.width  : 
                                   (actual_extent.width < capabilities.minImageExtent.width)   ? capabilities.minImageExtent.width  : actual_extent.width;
            actual_extent.height = (actual_extent.height > capabilities.maxImageExtent.height) ? capabilities.maxImageExtent.height : 
                                   (actual_extent.height < capabilities.minImageExtent.height) ? capabilities.minImageExtent.height : actual_extent.height;

            return actual_extent;

        }

    #pragma endregion // Utilities

    int zen_vk_create_instance(uint32_t api_version) {
        
        VkApplicationInfo app_info = (VkApplicationInfo) {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "zenlib-core",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),           
            .pEngineName = "NO ENGINE",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = api_version
        };

        size_t extension_count = __zencore_context__.vk_context.info.instance_extension_count;
        const char** extensions = __zencore_context__.vk_context.info.instance_extensions;
        
        VkInstanceCreateInfo create_info = (VkInstanceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app_info,
            .enabledExtensionCount = extension_count,
            .ppEnabledExtensionNames = extensions,
            .enabledLayerCount = 0,
        };

        if (vkCreateInstance(&create_info, NULL, &__zencore_context__.vk_context.instance) != VK_SUCCESS) {
            log_error("Failed to create vulkan instance.");
            return -1;
        }

        __zencore_context__.vk_context.version = api_version;
        __zencore_context__.vk_context.initialized = true;
        return 0;

    }

    int zen_vk_create_surface(ZEN_Window* window) {

        VkWin32SurfaceCreateInfoKHR create_info = (VkWin32SurfaceCreateInfoKHR) {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hwnd = window->handle,
            .hinstance = __zencore_context__.h_instance
        };

        if (vkCreateWin32SurfaceKHR(__zencore_context__.vk_context.instance, &create_info, NULL, &window->vk_context.surface) != VK_SUCCESS) {
            log_error("Failed to create surface.");
            return -1;
        }

        return 0;

    }

    int zen_vk_pick_physical_device(ZEN_Window* window) {

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

        for (size_t d = 0; d < device_count; d++) {
            if (zen_vk_is_device_suitable(window, devices[d])) {
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

        size_t extension_count = __zencore_context__.vk_context.info.device_extension_count;
        const char** extensions = __zencore_context__.vk_context.info.device_extensions;

        VkPhysicalDeviceFeatures device_features = {};
        VkDeviceCreateInfo create_info = (VkDeviceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .enabledExtensionCount = extension_count,
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
    
    int zen_vk_create_swapchain(ZEN_Window* window) {
        
        if (zen_vk_query_swapchain_support(window, __zencore_context__.vk_context.physical_device) < 0) {
            log_error("Failed to query swapchain support.");
            return -1;
        }

        VkSurfaceFormatKHR surface_format = zen_vk_choose_swap_surface_format(window);
        VkPresentModeKHR present_mode = zen_vk_choose_swap_present_mode(window);
        VkExtent2D extent = zen_vk_choose_swap_extent(window);

        uint32_t image_count = window->vk_context.surface_capabilities.minImageCount + 1;

        if (window->vk_context.surface_capabilities.maxImageCount > 0 && 
            image_count > window->vk_context.surface_capabilities.maxImageCount)
            image_count = window->vk_context.surface_capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR create_info = (VkSwapchainCreateInfoKHR) {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageColorSpace = surface_format.colorSpace,
            .surface = window->vk_context.surface,
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

        create_info.preTransform = window->vk_context.surface_capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.oldSwapchain = VK_NULL_HANDLE;
        create_info.presentMode = present_mode;
        create_info.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(__zencore_context__.vk_context.device, &create_info, NULL, &window->vk_context.swap_chain) != VK_SUCCESS) {
            log_error("Failed to create swap chain.");
            return -1;
        }

        vkGetSwapchainImagesKHR(__zencore_context__.vk_context.device, window->vk_context.swap_chain, &image_count, NULL);
        window->vk_context.swap_chain_image_count = (size_t)image_count;
        window->vk_context.swap_chain_images = malloc(sizeof(VkImage) * image_count);
        if (window->vk_context.swap_chain_images == NULL) {
            log_error("Failed to allocate space for chain images.");
            return -1;
        }

        vkGetSwapchainImagesKHR (
            __zencore_context__.vk_context.device, 
            window->vk_context.swap_chain, 
            &image_count, 
            window->vk_context.swap_chain_images
        );

        window->vk_context.swap_chain_extent = extent;
        window->vk_context.swap_chain_image_format = surface_format.format;

        return 0;

    }

    int zen_vk_create_image_views(ZEN_Window* window) {

        window->vk_context.swap_chain_image_view_count = window->vk_context.swap_chain_image_count;
        window->vk_context.swap_chain_image_views = malloc(sizeof(VkImageView) * window->vk_context.swap_chain_image_count);
        if (window->vk_context.swap_chain_image_views == NULL) {
            log_error("Failed to allocate space for swap chain image views.");
            return -1;
        }

        for (size_t i = 0; i < window->vk_context.swap_chain_image_count; i++) {
            
            VkImageViewCreateInfo create_info = (VkImageViewCreateInfo) {
                
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = window->vk_context.swap_chain_images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = window->vk_context.swap_chain_image_format,
              
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

            if (vkCreateImageView(__zencore_context__.vk_context.device, &create_info, NULL, &window->vk_context.swap_chain_image_views[i]) != VK_SUCCESS) {
                log_error("Failed to create image views.");
                return -1;
            }
            
        }

        return 0;

    }

#endif // ZEN_OS_WINDOWS