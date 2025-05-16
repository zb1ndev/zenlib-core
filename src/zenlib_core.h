/* zenlib - v0.0.1 - MIT License - https://github.com/zb1ndev/zenlib

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

    Table of Contents :
    [1] - Platforms
    [2] - Platform Specific Includes
    [3] - Types
    [4] - Event System
    [5] - Vulkan or OpenGL or DirectX
    [6] - Window Management
    [7] - ZEN Main

*/

#if !defined(ZENLIB_CORE_H)
#define ZENLIB_CORE_H

    #include <stdlib.h>

    #include <stdio.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <assert.h>

    #include <stdatomic.h>

    #include "zstring.h"
    
    #define ZEN_MAX_WINDOWS 10

    #pragma region Platforms

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

    #pragma endregion // Platform Definitions
    #pragma region Platform Specific Includes

        #if defined(ZEN_OS_WINDOWS)
            #include <windows.h>
        #endif // ZEN_OS_WINDOWS

    #pragma endregion // Platform Specific Includes
    #pragma region Event System

        typedef enum ZEN_KeyCode {
            
            Key_Escape

        } ZEN_KeyCode;

        #define ZEN_KEY_ON_DOWN 1
        #define ZEN_KEY_PRESSED 2
        #define ZEN_KEY_ON_UP 3

        typedef struct ZEN_MouseState {

            ssize_t x;
            ssize_t y;
        
            bool left_button;
            bool right_button;
            bool middle_button;
        
        } ZEN_MouseState;

        typedef struct ZEN_EventHandler {
            
            bool should_close;
            uint8_t key_states[1];
            ZEN_MouseState mouse_state;
            
            void (*resize_callback)(void* data, ssize_t new_width, ssize_t new_height);
            void (*minimize_callback)(void* data);
            bool (*close_callback)(void* data);

            float delta_time;

        } ZEN_EventHandler; 

        #if defined(ZEN_OS_WINDOWS)
            LRESULT CALLBACK ZEN_WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        #endif // ZEN_OS_WINDOWS

    #pragma endregion // Event System
    #pragma region Vulkan or OpenGL or DirectX
        
        
        
    #pragma endregion // Vulkan or OpenGL or DirectX
    #pragma region Window Management
        
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

        /** A function that creates a window.
         * @param title The title of the window.
         * @param width The width of the window.
         * @param height The height of the window.
         * @returns A reference to the window.
         */
        ZEN_Window* zen_create_window(const char* title, size_t width, size_t height, ZEN_WindowStyle style); 

        /** A function that shows a window.
         * @param window The window you want to show.
         */
        void zen_show_window(ZEN_Window* window);
        
        /** A function that determines if a window should close.
         * @param window The window you want to watch.
         * @returns If said window should close.
         */
        bool zen_window_should_close(ZEN_Window* window);
        
        /** A function that destoys a window.
         * @param window The window you want to destroy.
         */
        void zen_destroy_window(ZEN_Window* window);

    #pragma endregion // Window Management
    #pragma region ZEN Main

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

    #pragma endregion // ZEN_MAIN

    #if defined(ZEN_STRIP_PREFIX)

        #define KeyCode ZEN_KeyCode
        #define MouseState ZEN_MouseState
        #define EventHandler ZEN_EventHandler
        #define WindowStyle ZEN_WindowStyle
        #define Window ZEN_Window

        #define create_window zen_create_window
        #define show_window zen_show_window
        #define hide_window zen_hide_window
        #define update_window zen_update_window
        #define destroy_window zen_destroy_window

    #endif // ZEN_STRIP_PREFIX

#endif // ZENLIB_CORE_H