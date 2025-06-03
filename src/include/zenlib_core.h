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

    #define ZEN_DEBUG // Remove for Release

    #if defined(ZEN_DEBUG)
        #define debug_log(msg) printf("[LOG] "msg"\n")
        #define debug_log_va(msg, ...) printf("[LOG] "msg"\n", __VA_ARGS__)
    #endif // ZEN_DEBUG

    #define log_error(msg) printf("[ERROR] "msg"\n")
    #define log_error_va(msg, ...) printf("[ERROR] "msg"\n", __VA_ARGS__)

    // Windows
    #if defined(_WIN32) || defined(_WIN64)
        #define ZEN_OS_WINDOWS
    #endif

    // macOS
    #if defined(__APPLE__) && defined(__MACH__)
        #define ZEN_OS_MAC
    #endif

    // Linux
    #if defined(__linux__)
        #define ZEN_OS_LINUX
    #endif

    // Unix (includes macOS and Linux)
    #if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
        #define ZEN_OS_UNIX
    #endif

    #include "zstring.h"

    #include <stdlib.h>
    #include <stdio.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <assert.h>
    
    #include "vulkan/vulkan.h"
    #include "../../deps/include/cglm/cglm.h"

    #if defined(ZEN_OS_WINDOWS)
        #include <windows.h>
        #include <windowsx.h>
        #include <vulkan/vulkan_win32.h>
    #endif // ZEN_OS_WINDOWS

    #define ZEN_VULKAN_VERSION VK_MAKE_VERSION(1,0,0)
    #define ZEN_MAX_WINDOWS 10
    #define ZEN_MAX_FRAMES_IN_FLIGHT 2

    #define zen_vk_offset_of(s, m) (long)(&(((s*)0)->m))

    typedef enum ZEN_RendererAPI {

        ZEN_RAPI_None, ZEN_RAPI_OpenGL, 
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
        void (*on_minimize_callback)(void* data);
        void (*on_restore_callback)(void* data);
        bool (*on_close_callback)(void* data);
        
        bool should_close;
        bool minimized, resized;
        float delta_time;

    } ZEN_EventHandler;

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

    } ZEN_Window;

    typedef struct ZEN_RenderPipline {

        VkPipeline graphics_pipeline;
        VkPipelineLayout pipeline_layout;
    
    } ZEN_RenderPipline;

    typedef struct ZEN_Vertex {
        
        vec2 pos;
        vec3 color;

    } ZEN_Vertex;

    typedef struct ZEN_Shader {

        const char* name;

        const char* vertex_shader_path;
        const char* fragment_shader_path;

        ZEN_RenderPipline* pipline;

    } ZEN_Shader;

    typedef struct ZEN_RenderObject {
        
        bool enabled;
        size_t index;

        size_t vertex_count;
        ZEN_Vertex* vertices;

        size_t index_count;
        int* indices;

        size_t shader;

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
        size_t current_frame;

        size_t surface_count;
        ZEN_VulkanSurfaceInfo* surfaces;

        VkInstance instance;
        VkBuffer vertex_buffer;
        VkDeviceMemory vertex_buffer_memory;

        VkDevice device;
        VkPhysicalDevice physical_device;

        uint32_t graphics_family;
        uint32_t present_family;

        VkQueue graphics_queue;
        VkQueue present_queue;

        VkCommandPool command_pool;

        size_t graphics_pipline_count;
        ZEN_RenderPipline* graphics_pipelines;

        size_t shader_count;
        size_t shader_capacity;
        ZEN_Shader* shaders;
        
        VkRenderPass render_pass;

    } ZEN_VulkanContext;

    #if defined(ZEN_OS_WINDOWS)
    
        typedef struct ZEN_CoreContext {
                
            HINSTANCE h_instance;
            HINSTANCE h_prev_instance; 
            LPSTR lp_cmd_line; 
            int show_cmd;

            ZEN_Window windows[ZEN_MAX_WINDOWS];
            size_t window_count;

            ZEN_VulkanContext vk_context;

            size_t render_object_count;
            size_t render_object_last_count;

            size_t render_object_capacity;
            ZEN_RenderObject* render_objects;

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

    char* zen_read_file_contents(const char* file_path, size_t* file_size);
    
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
     * Appends a shader to the internal shader list.
     * @param shader The shader object to add.
     * @returns The index of the appended shader in the shader list.
     */
    size_t zen_append_shader(ZEN_Shader shader);

    /**
     * Appends a render object to the internal render object list.
     * @param object The render object to add.
     * @returns The index of the appended render object.
     */
    size_t zen_append_render_object(ZEN_RenderObject object);

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
        int zen_vk_recreate_swapchain(size_t context_index);
        int zen_vk_resize_vertex_buffer(void);
        int zen_vk_append_graphics_pipeline(size_t shader_index);

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
        int zen_vk_create_command_buffers(size_t context_index);
        int zen_vk_create_sync_objects(size_t context_index);

    #pragma endregion // Vulkan

    #if defined(ZEN_STRIP_PREFIX)

        #define KeyCode ZEN_KeyCode
        #define MouseState ZEN_MouseState
        #define EventHandler ZEN_EventHandler
        #define WindowStyle ZEN_WindowStyle
        #define Window ZEN_Window
        
        #define create_window zen_create_window
        #define window_should_close zen_window_should_close
        #define destroy_window zen_destroy_window

        #define get_key_up zen_get_key_up
        #define get_key_down zen_get_key_down
        #define get_key_pressed zen_get_key_pressed

        #define get_mouse_up zen_get_mouse_up
        #define get_mouse_down zen_get_mouse_down
        #define get_mouse_pressed zen_get_mouse_pressed

        #define initialize_renderer zen_initialize_renderer

    #endif // ZEN_STRIP_PREFIX

#endif // ZENLIB_CORE_H