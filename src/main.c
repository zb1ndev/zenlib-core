#include "include/zenlib_core.h"

int main(int argc, char** argv) {

    (void)argc;
    (void)argv;

    ZEN_Window* window = zen_create_window("Diva World", 400, 400, ZEN_RAPI_None);
    ZEN_Window* window2 = zen_create_window("Julia is short", 400, 400, ZEN_RAPI_None);


    while(!zen_window_should_close(window)) {
        if (zen_get_key_down(window, Z_KEY_A))
            printf("I pressed A!\n");
    }
    
    zen_destroy_window(window);
    zen_destroy_window(window2);

    return 0;

}