#include "../zenlib_core.h"

#if defined(ZEN_OS_WINDOWS)

    ZEN_Window* zen_create_window(const char* title, size_t width, size_t height, ZEN_RendererAPI api, bool show_title_bar) {

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
            show_title_bar ? WS_OVERLAPPEDWINDOW : (WS_POPUP | WS_THICKFRAME | WS_VISIBLE),            
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
            __zencore_context__.windows[0].handle, NULL, __zencore_context__.h_instance, window        
        );

        if (window->handle == NULL) {
            printf(ERRORF "CreateWindowEx failed.\n");
            return NULL;
        }

        window->class_name = class_name.content;
        window->show_title_bar = show_title_bar;

        if (!show_title_bar) {
            SetWindowLong(window->handle, GWL_STYLE, WS_POPUP | WS_VISIBLE);
            SetWindowLong(window->handle, GWL_EXSTYLE, 0);
            SetWindowPos(window->handle, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }

        if (zen_initialize_renderer(window, api) < 0) {
            printf(ERRORF "Failed to initialize renderer on window.\n");
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
            printf(ERRORF "Failed to destroy renderer.\n");

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

    void zen_minimize_window(ZEN_Window* window) { 
        ShowWindow(window->handle, SW_MINIMIZE); 
    }
    
    void zen_restore_window(ZEN_Window* window)  { 
        ShowWindow(window->handle, SW_RESTORE);  
    }
    
    void zen_maximize_window(ZEN_Window* window) { 
        ShowWindow(window->handle, SW_MAXIMIZE); 
    }

    void zen_set_window_title(ZEN_Window* window, const char* title) { 
        SetWindowTextA(window->handle, title); 
    }

    void zen_set_window_icon(ZEN_Window* window, const char* path) { 
    
        HICON icon = (HICON)LoadImage(NULL, path, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
        SendMessage(window->handle, WM_SETICON, ICON_BIG, (LPARAM)icon);
        SendMessage(window->handle, WM_SETICON, ICON_SMALL, (LPARAM)icon);

        return;
    
    }
    void zen_set_window_size(ZEN_Window* window, size_t width, size_t height) {

        ivec2 position; 
        zen_get_window_position(window, position);
        SetWindowPos(window->handle, NULL, position[0], position[1], width, height, SWP_SHOWWINDOW); 

    }

    void zen_get_window_position(ZEN_Window* window, ivec2 dest) {

        RECT rect;
        GetWindowRect(window->handle, &rect);

        dest[0] = rect.left;
        dest[1] = rect.top;
        
    }
    void zen_get_window_size(ZEN_Window* window, ivec2 dest) {
        
        dest[0] = window->width;
        dest[1] = window->height;
        
    }

    void zen_set_window_position(ZEN_Window* window, size_t x, size_t y, bool on_top) { 
        SetWindowPos(window->handle, on_top ? HWND_TOPMOST : NULL, x, y, window->width, window->height, SWP_SHOWWINDOW);
    }
    
    void zen_set_window_focused(ZEN_Window* window, bool on_top) { 
        SetFocus(window->handle);
        if (on_top) 
            BringWindowToTop(window->handle);
    }

#endif // ZEN_OS_WINDOWS