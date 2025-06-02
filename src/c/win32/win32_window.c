#include "../../include/zenlib_core.h"

#if defined(ZEN_OS_WINDOWS)

    ZEN_Window* zen_create_window(const char* title, size_t width, size_t height, ZEN_RendererAPI api) {

        if (__zencore_context__.window_count >= ZEN_MAX_WINDOWS)
            return NULL;

        String class_name = string_from_format("%s_%d", title, __zencore_context__.window_count);
        WNDCLASS window_class = {
            .lpszClassName = class_name.content,
            .lpfnWndProc = ZEN_WindowProcedure,
            .hInstance = __zencore_context__.h_instance,
        };
        RegisterClass(&window_class);

        ZEN_Window* window = &__zencore_context__.windows[__zencore_context__.window_count];

        *window = (ZEN_Window) {
            .title = title,
            .width = width,
            .height = height,
            .api = api,
            .context_index = __zencore_context__.window_count
        };

        window->handle = CreateWindowEx(
            0, class_name.content, title,    
            WS_OVERLAPPEDWINDOW,            
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
            __zencore_context__.windows[0].handle, NULL, __zencore_context__.h_instance, window        
        );

        if (window->handle == NULL) {
            log_error("CreateWindowEx failed.");
            return NULL;        
        }

        window->class_name = class_name.content;

        if (zen_initialize_renderer(window, api) < 0) {
            log_error("Failed to initialize renderer on window.");
            __zencore_context__.window_count++;
            zen_destroy_window(window);
            return NULL;
        }

        ShowWindow(__zencore_context__.windows[__zencore_context__.window_count].handle, __zencore_context__.show_cmd);
        UpdateWindow(__zencore_context__.windows[__zencore_context__.window_count].handle);

        return &__zencore_context__.windows[__zencore_context__.window_count++];

    }

    bool zen_window_should_close(ZEN_Window* window) {

        if (window == NULL)
            return true;
        
        PeekMessage(&window->msg, NULL, 0, 0, PM_REMOVE);
        TranslateMessage(&window->msg);
        DispatchMessage(&window->msg);

        return window->event_handler.should_close;
        
    }

    void zen_destroy_window(ZEN_Window* window) {

        if (window == NULL)
            return;

        if (zen_destroy_renderer(window, window->api) < 0)
            log_error("Failed to destroy renderer.");

        DestroyWindow(window->handle);
        free(window->class_name);

        size_t destroyed_index = window->context_index;

        ZEN_Window temp[ZEN_MAX_WINDOWS];
        memcpy(temp, __zencore_context__.windows, sizeof(temp));
        
        for (size_t i = 0, j = 0; i < __zencore_context__.window_count; i++) {
            if (i != destroyed_index) j++;
            __zencore_context__.windows[j] = temp[i];
        }

        __zencore_context__.window_count--;
        __zencore_context__.windows[window->context_index] = (ZEN_Window){0};

    }

#endif // ZEN_OS_WINDOWS