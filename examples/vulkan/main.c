#include "../../src/include/zenlib_core.h"

int main(int argc, char** argv) {
    
    (void)argc;
    (void)argv;

    size_t default_shader = zen_append_shader((ZEN_Shader) {
        .name = "default",
        .vertex_shader_path = "./examples/vulkan/shaders/compiled/vert.spv",
        .fragment_shader_path = "./examples/vulkan/shaders/compiled/frag.spv"
    });

    ZEN_Window* window = zen_create_window("Game Window", 500, 500, ZEN_RAPI_Vulkan);

    while (!zen_window_should_close(window)) {
        
        if (zen_get_key_down(window, Z_KEY_Escape))
            break;
        
        zen_draw_frame(window);
        printf("FPS : %f\nDelta Time : %f\n", zen_get_fps(window), zen_get_delta_time(window));

    }
    
    zen_destroy_window(window);

    return 0;

}