#include "../../src/include/zenlib_core.h"

int main(int argc, char** argv) {
    
    (void)argc;
    (void)argv;

    ZEN_Window* window = zen_create_window("Game Window", 500, 500, ZEN_RAPI_None);

    while (!zen_window_should_close(window)) {
        
        if (zen_get_key_down(window, Z_KEY_Escape))
            break;
        
    }
    
    zen_destroy_window(window);

    return 0;

}