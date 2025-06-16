/* zenlib-core - v0.0.1 - MIT License - https://github.com/zb1ndev/zenlib-core

    MIT License
    Copyright (c) 2025 Joel Zbinden

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    #define ZEN_STRIP_PREFIX - to strip the "zen" prefix from all functions and types.
    #define ZEN_MAX_WINDOWS - to enable the use of more windows in a single app. Default is 10.

*/

#if !defined(ZENLIB_CORE_H)
#define ZENLIB_CORE_H
    
    /////////////////////////
    // Platform Specifiers //
    /////////////////////////

    #if defined(_WIN32) || defined(_WIN64)
        #define ZEN_OS_WINDOWS
    #elif defined(__APPLE__) && defined(__MACH__)
        #define ZEN_OS_MAC
    #elif defined(__linux__)
        #define ZEN_OS_LINUX
    #endif
    
    #if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
        #define ZEN_OS_UNIX
    #endif

    //////////////////////
    // Core Dependecies //
    //////////////////////

    #include "../deps/zstring.h"
    #include "../deps/zstopwatch.h"

    #include "vulkan/vulkan.h"
    #include "../deps/cglm/cglm.h"

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <assert.h>
    #include <stdbool.h>

    #if defined(ZEN_OS_WINDOWS)

        #include <windows.h>
        #include <windowsx.h>
        #include <commctrl.h>
        #include <vulkan/vulkan_win32.h>

    #endif // ZEN_OS_WINDOWS

    ////////////////////
    // Utility Macros //
    ////////////////////

    #define ZEN_CLEAR_TERMINAL "\033[2J\033[H"

    #define debug_log(msg) printf("[LOG] "msg"\n")
    #define debug_log_va(msg, ...) printf("[LOG] "msg"\n", __VA_ARGS__)
 
    #define log_error(msg) printf("[ERROR] "msg"\n")
    #define log_error_va(msg, ...) printf("[ERROR] "msg"\n", __VA_ARGS__)

    #define ZEN_VULKAN_VERSION VK_MAKE_VERSION(1,0,0)
    #define ZEN_MAX_WINDOWS 10
    #define ZEN_MAX_FRAMES_IN_FLIGHT 2
    
    #define zen_vk_offset_of(s, m) offsetof(s,m)

    ///////////////////////
    // Type Declarations //
    ///////////////////////

    typedef enum ZEN_RendererAPI {

        ZEN_RAPI_None, 
        ZEN_RAPI_Vulkan, ZEN_RAPI_DirectX

    } ZEN_RendererAPI;

    typedef enum ZEN_KeyCode {

        // Letters
        Z_KEY_A, Z_KEY_B, Z_KEY_C, Z_KEY_D, Z_KEY_E, Z_KEY_F, Z_KEY_G, Z_KEY_H, Z_KEY_I, Z_KEY_J,
        Z_KEY_K, Z_KEY_L, Z_KEY_M, Z_KEY_N, Z_KEY_O, Z_KEY_P, Z_KEY_Q, Z_KEY_R, Z_KEY_S, Z_KEY_T,
        Z_KEY_U, Z_KEY_V, Z_KEY_W, Z_KEY_X, Z_KEY_Y, Z_KEY_Z,

        // Numbers
        Z_KEY_0, Z_KEY_1, Z_KEY_2, Z_KEY_3, Z_KEY_4, Z_KEY_5, Z_KEY_6, Z_KEY_7, Z_KEY_8, Z_KEY_9,

        // Function keys
        Z_KEY_F1, Z_KEY_F2, Z_KEY_F3, Z_KEY_F4, Z_KEY_F5, Z_KEY_F6, Z_KEY_F7, Z_KEY_F8, Z_KEY_F9, Z_KEY_F10,
        Z_KEY_F11, Z_KEY_F12,

        // Control keys
        Z_KEY_Escape, Z_KEY_Tab, Z_KEY_CapsLock, Z_KEY_Shift, Z_KEY_Control,
        Z_KEY_Alt, Z_KEY_Space, Z_KEY_Enter, Z_KEY_Backspace, Z_KEY_Menu,

        // Arrow keys
        Z_KEY_Left, Z_KEY_Up, Z_KEY_Right, Z_KEY_Down,

        // Navigation keys
        Z_KEY_Insert, Z_KEY_Delete, Z_KEY_Home, Z_KEY_End, Z_KEY_PageUp,
        Z_KEY_PageDown, Z_KEY_Pause, Z_KEY_PrintScreen, Z_KEY_ScrollLock,

        // Numpad keys
        Z_KEY_NumLock, Z_KEY_Num0, Z_KEY_Num1, Z_KEY_Num2, Z_KEY_Num3, Z_KEY_Num4, Z_KEY_Num5, Z_KEY_Num6, Z_KEY_Num7, Z_KEY_Num8,
        Z_KEY_Num9, Z_KEY_NumDivide, Z_KEY_NumMultiply, Z_KEY_NumSubtract, Z_KEY_NumAdd,
        Z_KEY_NumDecimal, Z_KEY_NumEnter,

        // Symbol keys
        Z_KEY_Semicolon, Z_KEY_Equals, Z_KEY_Comma, Z_KEY_Minus, Z_KEY_Period,
        Z_KEY_Slash, Z_KEY_Backtick, Z_KEY_LeftBracket, Z_KEY_Backslash, Z_KEY_RightBracket,
        Z_KEY_Quote,

        // Modifiers (left/right)
        Z_KEY_LeftShift, Z_KEY_RightShift, Z_KEY_LeftControl, Z_KEY_RightControl, Z_KEY_LeftAlt,
        Z_KEY_RightAlt, Z_KEY_LeftSuper, Z_KEY_RightSuper,

        // Count of keys
        Z_KEY_Count

    } ZEN_KeyCode;

    typedef struct ZEN_MouseState {

        ssize_t x;
        ssize_t y;
        
        bool button_states[3];
        bool prev_button_states[3];

    } ZEN_MouseState;

    typedef struct ZEN_EventHandler {
        
        bool key_states[107];
        bool prev_key_states[107];

        ZEN_MouseState mouse_state;

        void (*on_key_up_callback)(void* data);
        void (*on_key_down_callback)(void* data);
        
        void (*on_mouse_move_callback)(void* data);
        void (*on_mouse_button)(void* data);
        
        void (*on_resize_callback)(void* data, ssize_t new_width, ssize_t new_height);
        void (*on_move_callback)(void* data);
        void (*on_minimize_callback)(void* data);
        void (*on_restore_callback)(void* data);
        bool (*on_close_callback)(void* data);

        void (*background_callback)(void* data);
        
        bool should_close;
        bool minimized, resized;

    } ZEN_EventHandler;

    typedef enum ZEN_ViewMode {

        ZEN_VIEW_MODE_2D, 
        ZEN_VIEW_MODE_3D_ORTHOGRAPHIC,
        ZEN_VIEW_MODE_3D_PERSPECTIVE
    
    } ZEN_ViewMode;

    typedef enum ZEN_CoordinateSystem {

        ZEN_NDC_SPACE,
        ZEN_SCREEN_SPACE,
        ZEN_WORLD_SPACE

    } ZEN_CoordinateSystem;

    typedef struct ZEN_Window {

        char* class_name;
        const char* title; 
        size_t width; 
        size_t height;

        #if defined(ZEN_OS_WINDOWS)
        
            MSG msg;
            HWND handle;
            void (*win_proc_extension)(void* window, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        
        #endif // ZEN_OS_WINDOWS

        ZEN_RendererAPI api;
        ZEN_EventHandler event_handler;

        size_t context_index;
        size_t renderer_context_index;

        double frame_rate, delta_time;

        size_t frame_count;
        ZEN_Stopwatch frame_timer;
        double current_frame, last_frame;

        vec4 clear_color;
        ZEN_ViewMode view_mode;

    } ZEN_Window;

    typedef struct ZEN_VulkanRenderPipline {

        VkPipeline graphics_pipeline;
        VkPipelineLayout pipeline_layout;
    
    } ZEN_VulkanRenderPipline;

    typedef struct ZEN_Vertex {
        
        vec2 pos;
        vec3 color;

    } ZEN_Vertex;

    typedef struct ZEN_Shader {

        char* name;
        char* vertex_shader_path;
        char* fragment_shader_path;

        size_t pipeline;

    } ZEN_Shader;

    typedef struct ZEN_Transform {

        vec3 position;
        vec4 rotation;
        vec3 scale;

    } ZEN_Transform;

    typedef struct ZEN_RenderObject {
        
        bool enabled;
        ZEN_CoordinateSystem coord_system;

        size_t index;

        size_t vertex_count;
        ZEN_Vertex* vertices;

        size_t index_count;
        uint16_t* indices;

        size_t shader;
        ZEN_Transform transform;

    } ZEN_RenderObject;

    typedef struct ZEN_VulkanSurfaceInfo {

        ZEN_Window* window;
        VkSurfaceKHR surface;

        size_t surface_format_count;
        VkSurfaceFormatKHR* surface_formats;

        size_t present_mode_count;
        VkPresentModeKHR* present_modes;

        VkExtent2D swap_chain_extent;
        VkFormat swap_chain_image_format;
        VkSurfaceCapabilitiesKHR surface_capabilities;

        VkSwapchainKHR swap_chain;

        size_t swap_chain_image_count;
        VkImage* swap_chain_images;

        size_t swap_chain_image_view_count;
        VkImageView* swap_chain_image_views;

        VkSemaphore* image_available_semaphores;
        VkSemaphore* render_finished_semaphores;
        VkFence* in_flight_fences;

        VkFramebuffer* frame_buffers;
        VkCommandBuffer* command_buffers;

    } ZEN_VulkanSurfaceInfo;

    typedef struct ZEN_VulkanInfo {

        bool initialized;
        uint32_t version;

    } ZEN_VulkanInfo;

    typedef struct ZEN_VulkanContext {
        
        ZEN_VulkanInfo info;
        VkInstance instance;
        size_t current_frame;

        size_t surface_count;
        ZEN_VulkanSurfaceInfo* surfaces;

        VkBuffer vertex_buffer;
        VkDeviceMemory vertex_buffer_memory;

        VkBuffer index_buffer;
        VkDeviceMemory index_buffer_memory;

        VkDevice device;
        VkPhysicalDevice physical_device;

        uint32_t graphics_family;
        uint32_t present_family;

        VkQueue graphics_queue;
        VkQueue present_queue;

        VkCommandPool command_pool;

        size_t graphics_pipline_count;
        ZEN_VulkanRenderPipline* graphics_pipelines;

        VkRenderPass render_pass;

    } ZEN_VulkanContext;

    typedef struct ZEN_RendererContext {

        size_t shader_count;
        size_t shader_capacity;
        ZEN_Shader* shaders;
        
        size_t render_object_count;
        size_t render_object_last_count;

        size_t render_object_capacity;
        ZEN_RenderObject* render_objects;

    } ZEN_RendererContext; 

    /////////////////////////
    // ZEN MAIN Definition //
    /////////////////////////

    #if defined(ZEN_OS_WINDOWS)
    
        typedef struct ZEN_CoreContext {
                
            HINSTANCE h_instance;
            HINSTANCE h_prev_instance; 
            LPSTR lp_cmd_line; 
            int show_cmd;

            ZEN_Window windows[ZEN_MAX_WINDOWS];
            size_t window_count;

            ZEN_VulkanContext vk_context;
            ZEN_RendererContext renderer_context;
        
        } ZEN_CoreContext;
        extern ZEN_CoreContext __zencore_context__;

        int zen_main(int argc, char** argv);
        #define main(a, b) WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {\
            __zencore_context__.h_instance = hInstance;\
            __zencore_context__.h_prev_instance = hPrevInstance;\
            __zencore_context__.lp_cmd_line = lpCmdLine;\
            __zencore_context__.show_cmd = nCmdShow;\
            return zen_main(__argc, __argv);\
        } int zen_main(a, b)

    #endif // ZEN_OS_WINDOWS
    
    ///////////////////////////
    // Function Declarations //
    ///////////////////////////

    #pragma region Window Management

        /** A function that creates a window.
         * @param title The title of the window.
         * @param width The width of the window.
         * @param height The height of the window.
         * @returns A reference to the window.
         */
        ZEN_Window* zen_create_window(const char* title, size_t width, size_t height, ZEN_RendererAPI api); 
        
        /** A function that determines if a window should close.
         * @param window The window you want to watch.
         * @returns If said window should close.
         */
        bool zen_window_should_close(ZEN_Window* window);
        
        /** A function that destoys a window.
         * @param window The window you want to destroy.
         */
        void zen_destroy_window(ZEN_Window* window);

        /** Minimizes the specified window.
         * @param window The window to minimize.
         */
        void zen_minimize_window(ZEN_Window* window);
        
        /** Restores the specified window from minimized or maximized state.
         * @param window The window to restore.
         */
        void zen_restore_window(ZEN_Window* window);
        
        /** Maximizes the specified window.
         * @param window The window to maximize.
         */
        void zen_maximize_window(ZEN_Window* window);
        
        /** Sets the title of the specified window.
         * @param window The window whose title to set.
         * @param title The new title for the window.
         */
        void zen_set_window_title(ZEN_Window* window, const char* title);
        
        /** Sets the icon of the specified window.
         * @param window The window whose icon to set.
         * @param path The file path to the icon image.
         */
        void zen_set_window_icon(ZEN_Window* window, const char* path);
        
        /** Resizes the specified window to the given dimensions.
         * @param window The window to resize.
         * @param width The new width of the window.
         * @param height The new height of the window.
         */
        void zen_set_window_size(ZEN_Window* window, size_t width, size_t height);
        
        /** A function that gets the specified window's position.
         * @param window The window you want to check.
         * @returns The position stored in an array, ```[0]``` is X, ```[1]``` is Y.
         */
        size_t* zen_get_window_position(ZEN_Window* window);

        /** A function that gets the specified window's size.
         * @param window The window you want to check.
         * @returns The position stored in an array, ```[0]``` is Width, ```[1]``` is Height.
         */
        size_t* zen_get_window_size(ZEN_Window* window);

        /** Sets the position of the specified window on the screen.
         * @param window The window to move.
         * @param x The new x-coordinate of the window.
         * @param y The new y-coordinate of the window.
         */
        void zen_set_window_position(ZEN_Window* window, size_t x, size_t y, bool on_top);
        
        /** Sets whether the specified window is focused.
         * @param window The window to focus or unfocus.
         * @param focused True to focus the window, false to unfocus.
         */
        void zen_set_window_focused(ZEN_Window* window, bool on_top);

    #pragma endregion // Window Management
    #pragma region Event System

        #if defined(ZEN_OS_WINDOWS)

            LRESULT CALLBACK ZEN_WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
            
            /** A function that updates a window's event handler key states.
             * @param window The window you want to update.
             * @param code The key you want to update.
             * @param value Whether the key is up or down
             */
            void zen_update_key_states(ZEN_Window* window, UINT code, bool value);

            /** A function that sets a window's event handler key state to a value.
             * @param window The window you want to update.
             * @param index The key you want to update in the key state array.
             * @param value Whether the key is up or down
             */
            void zen_set_key_state(ZEN_Window* window, int index, bool value);

        #endif // ZEN_OS_WINDOWS

        /** A function that gets whether a key is released on the specified window.
         * @param window The window you want to check.
         * @param code The key you want to check for.
         * @returns Whether the key has been released.
         */
        bool zen_get_key_up(ZEN_Window* window, ZEN_KeyCode code);
        
        /** A function that gets whether a key is pressed on the specified window.
         * @param window The window you want to check.
         * @param code The key you want to check for.
         * @returns Whether the key has been pressed.
         */
        bool zen_get_key_down(ZEN_Window* window, ZEN_KeyCode code);
        
        /** A function that gets whether a key is being pressed on the specified window.
         * @param window The window you want to check.
         * @param code The key you want to check for.
         * @returns Whether the key is pressed.
         */
        bool zen_get_key_pressed(ZEN_Window* window, ZEN_KeyCode code);

        /** A function that gets whether a mouse button has been released on the specified window.
         * @param window The window you want to check.
         * @param button ```0``` for LMB, ```1``` for MMB, ```2``` for RMB.
         * @returns Whether the button has been released.
         */
        bool zen_get_mouse_up(ZEN_Window* window, size_t button);
        
        /** A function that gets whether a mouse button has been pressed on the specified window.
         * @param window The window you want to check.
         * @param button ```0``` for LMB, ```1``` for MMB, ```2``` for RMB.
         * @returns Whether the button has been pressed.
         */
        bool zen_get_mouse_down(ZEN_Window* window, size_t button);
        
        /** A function that gets whether a mouse button is being pressed on the specified window.
         * @param window The window you want to check.
         * @param button ```0``` for LMB, ```1``` for MMB, ```2``` for RMB.
         * @returns Whether the button is pressed.
         */
        bool zen_get_mouse_pressed(ZEN_Window* window, size_t button);

    #pragma endregion // Event System
    #pragma region File System

        /** A function that reads the contents of the specified file.
         * @param file_path The path to the file you want to read.
         * @param file_size A pointer to a ```size_t``` that will be set to the size of the file.
         * @returns The contents of the file as a ```char*```.
         */
        char* zen_read_file_contents(const char* file_path, size_t* file_size);

    #pragma endregion // File System
    #pragma region Rendering

        /**
         * Initializes the given renderer API on the specified window.
         * @param window The window to bind the renderer API to.
         * @param api The renderer API to initialize.
         * @returns 0 if successful, otherwise an error code.
         */
        int zen_initialize_renderer(ZEN_Window* window, ZEN_RendererAPI api);

        /**
         * Destroys the renderer API associated with the specified window.
         * @param window The window whose renderer API will be destroyed.
         * @param api The renderer API to destroy.
         * @returns 0 if successful, otherwise an error code.
         */
        int zen_destroy_renderer(ZEN_Window* window, ZEN_RendererAPI api);

        /**
         * Sets the clear color.
         * @param color The color you wish to set the clear color to.
         */
        void zen_set_clear_color(ZEN_Window* window, vec4 color);

        /**
         * Appends a shader to the internal shader list.
         * @param shader The shader object to add.
         * @returns The index of the appended shader in the shader list.
         */
        size_t zen_append_shader(ZEN_Shader shader);

        /**
         * Gets a reference to the shader at ```index``` in the internal shader list.
         * @param index The index where the shader resides.
         * @returns A reference to the specified shader.
         */
        ZEN_Shader* zen_get_shader(size_t index);

        /**
         * Appends a render object to the internal render object list.
         * @param object The render object to add.
         * @returns The index of the appended render object.
         */
        size_t zen_append_render_object(ZEN_RenderObject object);

        /**
         * Gets a reference to the render object at ```index``` in the internal shader list.
         * @param index The index where the render object resides.
         * @returns A reference to the specified render object.
         */
        ZEN_RenderObject* zen_get_render_object(size_t index);

        /**
         * Removes a render object from the internal render object list.
         * @param object Pointer to the render object to remove.
         * @returns 0 if successful, otherwise an error code.
         */
        int zen_remove_render_object(ZEN_RenderObject* object);

        /**
         * Clears all render objects from the internal render object list.
         * @returns 0 if successful, otherwise an error code.
         */
        int zen_clear_render_objects(void);

        /**
         * Gets the total number of vertices currently managed.
         * @returns The total vertex count.
         */
        uint64_t zen_get_vertex_count(void);

        /**
         * Gets the vertex count for the render object at the given index.
         * @param index The index of the render object.
         * @returns The number of vertices for that render object.
         */
        uint64_t zen_get_vertex_count_at_index(size_t index);

        /**
         * Gets a pointer to the array of all vertices managed internally.
         * @returns Pointer to the vertex array.
         */
        ZEN_Vertex* zen_get_vertices(void);

        /**
         * Gets the total number of indices currently managed.
         * @returns The total index count.
         */
        uint64_t zen_get_index_count(void);

        /**
         * Gets the index count for the render object at the given index.
         * @param index The index of the render object.
         * @returns The number of indices for that render object.
         */
        uint64_t zen_get_index_count_at_index(size_t index);

        /**
         * Gets a pointer to the array of all indices managed internally.
         * @returns Pointer to the index array.
         */
        uint16_t* zen_get_indices(void);

        /**
         * Sets the view mode for the renderer.
         * @param mode The view mode you want to set it to.
         */
        void zen_set_view_mode(ZEN_Window* window, ZEN_ViewMode mode);

        /**
         * Draws a frame on the specified window, submitting all current render objects.
         * @param window The window to draw the frame on.
         * @returns 0 if successful, otherwise an error code.
         */
        int zen_draw_frame(ZEN_Window* window);

        /**
         * Gets the current frames-per-second (FPS) for the specified window.
         * @param window The window for which to get the FPS.
         * @returns The current FPS as a floating point value.
         */
        float zen_get_fps(ZEN_Window* window);

        /**
         * Gets the time elapsed between the last two frames (delta time) for the specified window.
         * @param window The window for which to get the delta time.
         * @returns The delta time in seconds.
         */
        float zen_get_delta_time(ZEN_Window* window);

        /**
         * Scales the provided vec3 by the specified amount per axis.
         * @param a The vector you want to scale.
         * @param x_scale The scale modifier for the X axis.
         * @param y_scale The scale modifier for the Y axis.
         * @param z_scale The scale modifier for the Z axis.
         * @param dest A vec3 where the modified vector will be stored.
         */
        void zen_scale_vec3(vec3 a, float x_scale, float y_scale, float z_scale, vec3 dest);

        /**
         * Creates a model matrix based on a transform using 2D order of operations.
         * @param window The window in which these transformations will occur.
         * @param coord The coordinate system the transform is using.
         * @param transform The transform which the model will be based off.
         * @param model The matrix in which the transformation will be stored.
         */
        void zen_make_model_from_transform_2d(ZEN_Window* window, ZEN_CoordinateSystem coord, ZEN_Transform* transform, vec4* model);

    #pragma endregion // Rendering
    #pragma region Vulkan

        // vulkan_draw.c
        int zen_vk_draw_frame(size_t context_index);

        // vulkan_utils.c
        const char** zen_vk_get_instance_extensions(size_t* count);
        const char ** zen_vk_get_device_extensions(size_t *count);
        bool zen_vk_find_queue_families(VkPhysicalDevice device, size_t context_index);
        bool zen_vk_query_swapchain_support(VkPhysicalDevice device, size_t context_index);
        bool zen_vk_device_has_extensions(VkPhysicalDevice device);
        bool zen_vk_check_device_api_version(VkPhysicalDevice device);
        bool zen_vk_device_is_suitable(VkPhysicalDevice device, size_t context_index);
        VkSurfaceFormatKHR zen_vk_choose_swap_surface_format(size_t context_index);
        VkPresentModeKHR zen_vk_choose_swap_present_mode(size_t context_index);
        VkExtent2D zen_vk_choose_swap_extent(size_t context_index);
        VkShaderModule zen_vk_create_shader_module(const char* code, size_t code_size);
        VkVertexInputBindingDescription zen_vk_get_vertex_binding_description();
        VkVertexInputAttributeDescription* zen_vk_get_vertex_attribute_descriptions();
        uint32_t zen_vk_find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        int zen_vk_cleanup_swapchain(size_t context_index);
        int zen_vk_recreate_swapchain(size_t context_index);
        int zen_vk_resize_vertex_buffer(void);
        int zen_vk_append_graphics_pipeline(size_t shader_index);
        int zen_vk_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* buffer_memory);
        int zen_vk_copy_buffer(VkBuffer src_buffer, VkBuffer dest_buffer, VkDeviceSize size);

        // vulkan_init.c
        int zen_init_vulkan(ZEN_Window* window, uint32_t api_version);
        void zen_destroy_vulkan(bool is_last, size_t context_index);

        int zen_vk_create_instance(void);
        int zen_vk_create_surface(size_t context_index);
        int zen_vk_pick_physical_device(size_t context_index);
        int zen_vk_create_logical_device(void);
        int zen_vk_create_swap_chain(size_t context_index);
        int zen_vk_create_image_views(size_t context_index);
        int zen_vk_create_render_pass(size_t context_index);
        int zen_vk_create_graphics_pipelines(void);
        int zen_vk_create_graphics_pipeline(ZEN_Shader* shader);
        int zen_vk_create_framebuffers(size_t context_index);
        int zen_vk_create_command_pool(void);
        int zen_vk_create_vertex_buffer(void);
        int zen_vk_create_index_buffer(void);
        int zen_vk_create_command_buffers(size_t context_index);
        int zen_vk_create_sync_objects(size_t context_index);

    #pragma endregion // Vulkan

    #if defined(ZEN_STRIP_PREFIX)

       

    #endif // ZEN_STRIP_PREFIX

#endif // ZENLIB_CORE_H