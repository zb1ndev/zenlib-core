# zenlib-core API Reference

## Table of Contents
- [Types & Enums](#types--enums)
  - [ZEN_RendererAPI](#zen_rendererapi)
  - [ZEN_KeyCode](#zen_keycode)
  - [ZEN_MouseState](#zen_mousestate)
  - [ZEN_EventHandler](#zen_eventhandler)
  - [ZEN_WindowStyle](#zen_windowstyle)
  - [ZEN_Window](#zen_window)
- [Core Functions](#core-functions)
  - [Window Management](#window-management)
  - [Input Handling](#input-handling)
  - [Renderer Initialization](#renderer-initialization)
  - [Windows-Specific Functions](#windows-specific-functions)
- [Macros & Prefix Stripping](#macros--prefix-stripping)
- [Example Usage](#example-usage)

zenlib-core is a lightweight C library for window management, input handling, and renderer abstraction, designed for cross-platform applications. Below is an overview of its main types, enums, and functions.

---

## Types & Enums

### ZEN_RendererAPI
Renderer backend selection:
- `ZEN_RAPI_None`
- `ZEN_RAPI_OpenGL`
- `ZEN_RAPI_Vulkan`
- `ZEN_RAPI_DirectX`

### ZEN_KeyCode
Enumeration of keyboard keys (letters, numbers, function keys, control keys, arrows, navigation, numpad, symbols, modifiers). Used for input handling.

### ZEN_MouseState
Represents mouse position and button states:
- `ssize_t x, y` — Mouse coordinates
- `bool button_states[3]` — LMB, MMB, RMB
- `bool prev_button_states[3]`

### ZEN_EventHandler
Handles input and window events:
- `bool key_states[107]`, `prev_key_states[107]`
- `ZEN_MouseState mouse_state`
- Callback pointers for key/mouse/window events
- `bool should_close`, `float delta_time`

### ZEN_WindowStyle
Window appearance customization:
- `double border_raduis`
- `void (*custom_toolbar)(void*, size_t, size_t)`

### ZEN_Window
Represents a window:
- `char* class_name`, `const char* title`
- `size_t width, height`
- Platform-specific handles (e.g., HWND)
- `ZEN_RendererAPI api`
- `ZEN_WindowStyle style`
- `ZEN_EventHandler event_handler`
- `size_t context_index`

---

## Core Functions

### Window Management
- `ZEN_Window* zen_create_window(const char* title, size_t width, size_t height, ZEN_RendererAPI api)`
  - Create a new window.
- `bool zen_window_should_close(ZEN_Window* window)`
  - Check if a window should close.
- `void zen_destroy_window(ZEN_Window* window)`
  - Destroy a window and free resources.

### Input Handling
- `bool zen_get_key_up(ZEN_Window* window, ZEN_KeyCode code)`
  - Check if a key was released.
- `bool zen_get_key_down(ZEN_Window* window, ZEN_KeyCode code)`
  - Check if a key was pressed.
- `bool zen_get_key_pressed(ZEN_Window* window, ZEN_KeyCode code)`
  - Check if a key is being held down.
- `bool zen_get_mouse_up(ZEN_Window* window, size_t button)`
  - Check if a mouse button was released (0=LMB, 1=MMB, 2=RMB).
- `bool zen_get_mouse_down(ZEN_Window* window, size_t button)`
  - Check if a mouse button was pressed.
- `bool zen_get_mouse_pressed(ZEN_Window* window, size_t button)`
  - Check if a mouse button is being held down.

### Renderer Initialization
- `int zen_initialize_renderer(ZEN_Window* window, ZEN_RendererAPI api)`
  - Initialize the selected renderer API for a window.

### Windows-Specific Functions
- `void zen_update_key_states(ZEN_Window* window, UINT code, bool value)`
- `void zen_set_key_state(ZEN_Window* window, int index, bool value)`
- `LRESULT CALLBACK ZEN_WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)`

---

## Macros & Prefix Stripping
- Define `ZEN_STRIP_PREFIX` to use short names (e.g., `Window` instead of `ZEN_Window`).
- Define `ZEN_MAX_WINDOWS` to set the maximum number of windows (default: 10).

---

## Example Usage
```c
ZEN_Window* window = zen_create_window("My App", 800, 600, ZEN_RAPI_OpenGL);
while (!zen_window_should_close(window)) {
    // Handle input, rendering, etc.
}
zen_destroy_window(window);
```

---

For more details, see the source code and comments in `zenlib_core.h`.
