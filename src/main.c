#include "include/zenlib_core.h"

int main(int argc, char** argv) {

    (void)argc;
    (void)argv;

    ZEN_Window* window = zen_create_window("Hello World", 400, 400, (ZEN_WindowStyle){});

    while(!zen_window_should_close(window)) {
        if (zen_get_key_up(window, Z_KEY_Escape) || zen_get_mouse_up(window, 0))
            break;
    }

    zen_destroy_window(window);
    return 0;

}