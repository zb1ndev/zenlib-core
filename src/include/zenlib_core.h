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

    #include "platform.h"
    #include "zstring.h"

    #include <stdlib.h>
    #include <stdio.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <assert.h>
    #include <stdatomic.h>

    #if defined(ZEN_OS_WINDOWS)
        #include <windows.h>
        #include <windowsx.h>
    #endif // ZEN_OS_WINDOWS

    #define ZEN_MAX_WINDOWS 10


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
        void (*on_mouse_move_down)(void* data);
        void (*on_mouse_move_up)(void* data);
        
        void (*resize_callback)(void* data, ssize_t new_width, ssize_t new_height);
        void (*minimize_callback)(void* data);
        bool (*close_callback)(void* data);
        
        bool should_close;
        float delta_time;

    } ZEN_EventHandler;

    typedef struct ZEN_WindowStyle {

        double border_raduis;
        void (*custom_toolbar)(void* data, size_t width, size_t height); // If not NULL then expand viewport over the default toolbar.

    } ZEN_WindowStyle;

    typedef struct ZEN_Window {

        char* class_name;
        const char* title; 
        size_t width; 
        size_t height;

        #if defined(ZEN_OS_WINDOWS)
            MSG msg;
            HWND handle;
        #endif // ZEN_OS_WINDOWS

        ZEN_WindowStyle style;
        ZEN_EventHandler event_handler;

        size_t context_index;

    } ZEN_Window;

    #if defined(ZEN_OS_WINDOWS)
    
        typedef struct ZEN_CoreContext {
                
            HINSTANCE h_instance;
            HINSTANCE h_prev_instance; 
            LPSTR lp_cmd_line; 
            int show_cmd;

            ZEN_Window windows[ZEN_MAX_WINDOWS];
            size_t window_count;

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
    ZEN_Window* zen_create_window(const char* title, size_t width, size_t height, ZEN_WindowStyle style); 
    
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

    #endif // ZEN_STRIP_PREFIX

#endif // ZENLIB_CORE_H