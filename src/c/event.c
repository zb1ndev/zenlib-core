#include "../zenlib_core.h"

#if defined(ZEN_OS_WINDOWS)

    LRESULT CALLBACK ZEN_WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        
        ZEN_Window* window = NULL;
        for (size_t i = 0; i < __zencore_context__.window_count; i++)
            if (__zencore_context__.windows[i].handle == hwnd)
                window = &__zencore_context__.windows[i];
        if (window == NULL)
            return DefWindowProc(hwnd, uMsg, wParam, lParam);

        switch (uMsg) {

            case WM_KEYDOWN:
                return 0;
                
            case WM_KEYUP:
                return 0;

            case WM_SIZE:
                if (wParam == SIZE_MINIMIZED && window->event_handler.minimize_callback != NULL)
                    window->event_handler.minimize_callback(window);
                return 0;
            case WM_SIZING:
                RECT* rect = (RECT*)lParam;
                if (window->event_handler.resize_callback != NULL)
                    window->event_handler.resize_callback(window, rect->right - rect->left, rect->bottom - rect->top);
                return TRUE;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            case WM_CLOSE:
                if (window->event_handler.close_callback == NULL || 
                    window->event_handler.close_callback(window))
                        window->event_handler.should_close = true;
                return 0;
            default:
                break;
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    }

#endif // ZEN_OS_WINDOWS