#include "../../include/zenlib_core.h"

#if defined(ZEN_OS_WINDOWS)

    LRESULT CALLBACK ZEN_WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        
        ZEN_Window* window = NULL;
        for (size_t i = 0; i < __zencore_context__.window_count; i++)
            if (__zencore_context__.windows[i].handle == hwnd)
                window = &__zencore_context__.windows[i];
        if (window == NULL)
            return DefWindowProc(hwnd, uMsg, wParam, lParam);

        if (window->win_proc_extension != NULL)
            window->win_proc_extension(window, hwnd, uMsg, wParam, lParam);

        switch (uMsg) {

            case WM_MOUSEMOVE: {

                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                window->event_handler.mouse_state.x = x;
                window->event_handler.mouse_state.y = y;

                if (window->event_handler.on_mouse_move_callback != NULL)
                    window->event_handler.on_mouse_move_callback(window);

            } return 0;

            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP: {

                ZEN_MouseState* mouse = &window->event_handler.mouse_state;
                mouse->prev_button_states[0] = mouse->button_states[0];
                mouse->button_states[0] = (uMsg == WM_LBUTTONDOWN);

                if (window->event_handler.on_mouse_button != NULL)
                    window->event_handler.on_mouse_button(window);

            } return 0;

            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP: {

                ZEN_MouseState* mouse = &window->event_handler.mouse_state;
                mouse->prev_button_states[2] = mouse->button_states[2];
                mouse->button_states[2] = (uMsg == WM_RBUTTONDOWN);
                
                if (window->event_handler.on_mouse_button != NULL)
                    window->event_handler.on_mouse_button(window);
            
            } return 0;

            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP: {
                ZEN_MouseState* mouse = &window->event_handler.mouse_state;
                mouse->prev_button_states[1] = mouse->button_states[1];
                mouse->button_states[1] = (uMsg == WM_MBUTTONDOWN);

                if (window->event_handler.on_mouse_button != NULL)
                    window->event_handler.on_mouse_button(window);

            } return 0;

            case WM_KEYDOWN: {
                UINT vkCode = (UINT)wParam;
                zen_update_key_states(window, vkCode, true);
                if (window->event_handler.on_key_down_callback != NULL)
                    window->event_handler.on_key_down_callback(window);

            } return 0;

            case WM_KEYUP: {
                UINT vkCode = (UINT)wParam;
                zen_update_key_states(window, vkCode, false);
                if (window->event_handler.on_key_up_callback != NULL)
                    window->event_handler.on_key_up_callback(window);
            } return 0;
            
            case WM_SIZE: {
                if (wParam == SIZE_MINIMIZED) {
                    window->event_handler.minimized = true;
                    if (window->event_handler.on_minimize_callback != NULL)
                        window->event_handler.on_minimize_callback(window);
                }
                if (wParam == SIZE_RESTORED) {
                    window->event_handler.minimized = false;
                    if(window->event_handler.on_restore_callback != NULL)
                        window->event_handler.on_restore_callback(window);
                }
            } return 0;

            case WM_SIZING:{
                RECT* rect = (RECT*)lParam;
                if (window->event_handler.on_resize_callback != NULL)
                    window->event_handler.on_resize_callback(window, rect->right - rect->left, rect->bottom - rect->top);
                window->event_handler.resized = true;
                window->width = rect->right - rect->left;
                window->height = rect->bottom - rect->top;

            } return TRUE;

            case WM_DESTROY:{
                PostQuitMessage(0);
            } return 0;

            case WM_CLOSE: {
                if (window->event_handler.on_close_callback == NULL || window->event_handler.on_close_callback(window))
                    window->event_handler.should_close = true;
            }  return 0;

            default:
                break;

        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    }

    void zen_set_key_state(ZEN_Window* window, int index, bool value) {
        
        if (index < 0 || index >= Z_KEY_Count) return;
        window->event_handler.prev_key_states[index] = window->event_handler.key_states[index];
        window->event_handler.key_states[index] = value;
    
    }

    void zen_update_key_states(ZEN_Window* window, UINT code, bool value) {

        if (code >= 'A' && code <= 'Z') {
            zen_set_key_state(window, code - 'A', value);
            return;
        }
        
        if (code >= '0' && code <= '9') {
            zen_set_key_state(window, 26 + (code - '0'), value);
            return;
        }

        switch (code) {

            // Function Keys
            case VK_F1:        zen_set_key_state(window, 36, value); break;
            case VK_F2:        zen_set_key_state(window, 37, value); break;
            case VK_F3:        zen_set_key_state(window, 38, value); break;
            case VK_F4:        zen_set_key_state(window, 39, value); break;
            case VK_F5:        zen_set_key_state(window, 40, value); break;
            case VK_F6:        zen_set_key_state(window, 41, value); break;
            case VK_F7:        zen_set_key_state(window, 42, value); break;
            case VK_F8:        zen_set_key_state(window, 43, value); break;
            case VK_F9:        zen_set_key_state(window, 44, value); break;
            case VK_F10:       zen_set_key_state(window, 45, value); break;
            case VK_F11:       zen_set_key_state(window, 46, value); break;
            case VK_F12:       zen_set_key_state(window, 47, value); break;

            // Control / Special Keys
            case VK_ESCAPE:    zen_set_key_state(window, 48, value); break;
            case VK_TAB:       zen_set_key_state(window, 49, value); break;
            case VK_CAPITAL:   zen_set_key_state(window, 50, value); break;
            case VK_SHIFT:     zen_set_key_state(window, 51, value); break;
            case VK_CONTROL:   zen_set_key_state(window, 52, value); break;
            case VK_MENU:      zen_set_key_state(window, 53, value); break;
            case VK_SPACE:     zen_set_key_state(window, 54, value); break;
            case VK_RETURN:    zen_set_key_state(window, 55, value); break;
            case VK_BACK:      zen_set_key_state(window, 56, value); break;

            // Arrows
            case VK_LEFT:      zen_set_key_state(window, 57, value); break;
            case VK_UP:        zen_set_key_state(window, 58, value); break;
            case VK_RIGHT:     zen_set_key_state(window, 59, value); break;
            case VK_DOWN:      zen_set_key_state(window, 60, value); break;

            // Navigation
            case VK_INSERT:    zen_set_key_state(window, 61, value); break;
            case VK_DELETE:    zen_set_key_state(window, 62, value); break;
            case VK_HOME:      zen_set_key_state(window, 63, value); break;
            case VK_END:       zen_set_key_state(window, 64, value); break;
            case VK_PRIOR:     zen_set_key_state(window, 65, value); break;
            case VK_NEXT:      zen_set_key_state(window, 66, value); break;

            // Numpad
            case VK_NUMLOCK:   zen_set_key_state(window, 67, value); break;
            case VK_NUMPAD0:   zen_set_key_state(window, 68, value); break;
            case VK_NUMPAD1:   zen_set_key_state(window, 69, value); break;
            case VK_NUMPAD2:   zen_set_key_state(window, 70, value); break;
            case VK_NUMPAD3:   zen_set_key_state(window, 71, value); break;
            case VK_NUMPAD4:   zen_set_key_state(window, 72, value); break;
            case VK_NUMPAD5:   zen_set_key_state(window, 73, value); break;
            case VK_NUMPAD6:   zen_set_key_state(window, 74, value); break;
            case VK_NUMPAD7:   zen_set_key_state(window, 75, value); break;
            case VK_NUMPAD8:   zen_set_key_state(window, 76, value); break;
            case VK_NUMPAD9:   zen_set_key_state(window, 77, value); break;
            case VK_DIVIDE:    zen_set_key_state(window, 78, value); break;
            case VK_MULTIPLY:  zen_set_key_state(window, 79, value); break;
            case VK_SUBTRACT:  zen_set_key_state(window, 80, value); break;
            case VK_ADD:       zen_set_key_state(window, 81, value); break;
            case VK_DECIMAL:   zen_set_key_state(window, 82, value); break;
            case VK_SEPARATOR: zen_set_key_state(window, 83, value); break;

            // Left/Right Modifiers
            case VK_LSHIFT:    zen_set_key_state(window, 84, value); break;
            case VK_RSHIFT:    zen_set_key_state(window, 85, value); break;
            case VK_LCONTROL:  zen_set_key_state(window, 86, value); break;
            case VK_RCONTROL:  zen_set_key_state(window, 87, value); break;
            case VK_LMENU:     zen_set_key_state(window, 88, value); break;
            case VK_RMENU:     zen_set_key_state(window, 89, value); break;
            case VK_LWIN:      zen_set_key_state(window, 90, value); break;
            case VK_RWIN:      zen_set_key_state(window, 91, value); break;

            // Misc
            case VK_PAUSE:     zen_set_key_state(window, 92, value); break;
            case VK_SNAPSHOT:  zen_set_key_state(window, 93, value); break;
            case VK_SCROLL:    zen_set_key_state(window, 94, value); break;

            default:
                printf("[WARNING] Key Not Implemented: code=%u (index=%d)\n", code, code - 65);
                break;

        }
    }   

#endif // ZEN_OS_WINDOWS