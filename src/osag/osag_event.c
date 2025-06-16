#include "../zenlib_core.h"

bool zen_get_key_up(ZEN_Window* window, ZEN_KeyCode code) {

    if (window->event_handler.prev_key_states[code] && !window->event_handler.key_states[code]) {
        window->event_handler.prev_key_states[code] = window->event_handler.key_states[code];
        return true; 
    }

    return false;

}

bool zen_get_key_down(ZEN_Window* window, ZEN_KeyCode code) {

    if (!window->event_handler.prev_key_states[code] && window->event_handler.key_states[code]) {
        window->event_handler.prev_key_states[code] = window->event_handler.key_states[code];
        return true;
    }

    return false;

}

bool zen_get_key_pressed(ZEN_Window* window, ZEN_KeyCode code) {
    return window->event_handler.key_states[code];
}

bool zen_get_mouse_up(ZEN_Window* window, size_t button) {

    if (window->event_handler.mouse_state.prev_button_states[button] && !window->event_handler.mouse_state.button_states[button]) {
        window->event_handler.mouse_state.prev_button_states[button] = window->event_handler.mouse_state.button_states[button];
        return true;
    }

    return false;

}

bool zen_get_mouse_down(ZEN_Window* window, size_t button) {

    if (!window->event_handler.mouse_state.prev_button_states[button] && window->event_handler.mouse_state.button_states[button]) {
        window->event_handler.mouse_state.prev_button_states[button] = window->event_handler.mouse_state.button_states[button];
        return true;
    }

    return false;

}

bool zen_get_mouse_pressed(ZEN_Window* window, size_t button) {
    return window->event_handler.mouse_state.button_states[button];
}