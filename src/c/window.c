#include "../include/zenlib_core.h"

#if defined(ZEN_OS_WINDOWS)

    ZEN_Window* zen_create_window(const char* title, size_t width, size_t height, ZEN_WindowStyle style) {

        if (__zencore_context__.window_count >= ZEN_MAX_WINDOWS)
            return NULL;

        String class_name = string_from_format("%s_%d", title, __zencore_context__.window_count);
        WNDCLASS window_class = { 
            .lpszClassName = class_name.content,
            .lpfnWndProc = ZEN_WindowProcedure,
            .hInstance = __zencore_context__.h_instance,
        };
        RegisterClass(&window_class);

        ZEN_Window window = (ZEN_Window) {
            .title = title,
            .width = width,
            .height = height,
            .style = style,
            .context_index = __zencore_context__.window_count
        };

        window.handle = CreateWindowEx(
            0, class_name.content, title,    
            WS_OVERLAPPEDWINDOW,            
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
            __zencore_context__.windows[0].handle, NULL, __zencore_context__.h_instance, &window        
        );

        if (window.handle == NULL) {
            DWORD error = GetLastError();
            printf("CreateWindowEx failed with error code: %lu\n", error);
            return NULL;        
        }

        window.class_name = class_name.content;
        __zencore_context__.windows[__zencore_context__.window_count] = window;
        
        ShowWindow(window.handle, __zencore_context__.show_cmd);
        UpdateWindow(window.handle);

        return &__zencore_context__.windows[__zencore_context__.window_count++];

    }

    bool zen_window_should_close(ZEN_Window* window) {
        
        GetMessage(&window->msg, NULL, 0, 0);
        TranslateMessage(&window->msg);
        DispatchMessage(&window->msg);

        return window->event_handler.should_close;
        
    }

    void zen_destroy_window(ZEN_Window* window) {

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