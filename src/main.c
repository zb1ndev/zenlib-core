#include "zenlib_core.h"

int main(int argc, char** argv) {

    (void)argc;
    (void)argv;

    ZEN_Window* window = zen_create_window("Hello World", 400, 400, (ZEN_WindowStyle){});

    zen_show_window(window);
    while(!zen_window_should_close(window)) {

    }

    zen_destroy_window(window);
    return 0;

}