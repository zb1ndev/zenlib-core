#include "../../src/include/zenlib_core.h"

int main(int argc, char** argv) {
    
    (void)argc;
    (void)argv;

    zen_set_clear_color((vec4){0.02f,0.02f,0.02f,1.0f});
    size_t default_shader = zen_append_shader((ZEN_Shader) {
        .name = "default",
        .vertex_shader_path = "./examples/vulkan/shaders/compiled/vert.spv",
        .fragment_shader_path = "./examples/vulkan/shaders/compiled/frag.spv"
    });

    ZEN_Window* window = zen_create_window("Game Window", 1280, 720, ZEN_RAPI_Vulkan);

    while (!zen_window_should_close(window)) {
        
        if (zen_get_key_down(window, Z_KEY_Escape))
            break;
        zen_draw_frame(window);
    
    }
    
    zen_destroy_window(window);

    return 0;

}