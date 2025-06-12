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

    size_t index = zen_append_render_object((ZEN_RenderObject) {
        .enabled = true,
        .shader = default_shader,

        .vertices = (ZEN_Vertex[]) {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
        },
        .vertex_count = 4,
        .index_count = 6,
        .indices = (uint16_t[]) { 0, 1, 2, 2, 3, 0 }
    });

    ZEN_Window* window = zen_create_window("Game Window", 1280, 720, ZEN_RAPI_Vulkan);
    
    zen_set_view_mode(ZEN_VIEW_MODE_2D);
    while (!zen_window_should_close(window)) {
        
        if (zen_get_key_down(window, Z_KEY_Escape))
            break;
        zen_draw_frame(window);
    
    }
    
    zen_destroy_window(window);

    return 0;

}