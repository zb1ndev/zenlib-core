#include "../../src/include/zenlib_core.h"

int main(int argc, char** argv) {
    
    (void)argc;
    (void)argv;

    size_t default_shader = zen_append_shader((ZEN_Shader) {
        
        .name = "default",
        .vertex_shader_path = "./examples/vulkan/shaders/compiled/vert.spv",
        .fragment_shader_path = "./examples/vulkan/shaders/compiled/frag.spv"

    });

    size_t index = zen_append_render_object((ZEN_RenderObject) {

        .enabled = true,
        .shader = default_shader,

        .vertex_count = 4,
        .vertices = (ZEN_Vertex[]) {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
        },
        
        .index_count = 6,
        .indices = (uint16_t[]) { 0, 1, 2, 2, 3, 0 },

        .transform = (ZEN_Transform) {
            .position = {.0f,.0f,.0f},
            .scale = {.5f,.2f,.2f},
            .rotation = {.0f,.0f,45.0f,.0f}
        }

    });

    debug_log_va("Created object %zu", index);

    ZEN_Window* window = zen_create_window("Game Window", 1280, 720, ZEN_RAPI_Vulkan);
    
    zen_set_view_mode(window, ZEN_VIEW_MODE_2D);
    zen_set_clear_color(window, (vec4){0.02f,0.02f,0.02f,1.0f});
    zen_set_window_icon(window, "./examples/vulkan/resources/logo.ico");

    ZEN_Window* window2 = zen_create_window("Game Window 2", 1280, 720, ZEN_RAPI_Vulkan);
    
    zen_set_view_mode(window2, ZEN_VIEW_MODE_2D);
    zen_set_clear_color(window2, (vec4){1.0f,1.0f,1.0f,1.0f});
    zen_set_window_icon(window2, "./examples/vulkan/resources/logo.ico");

    while (!zen_window_should_close(window)) {
        
        if (zen_get_key_down(window, Z_KEY_Escape))
            break;
        zen_draw_frame(window);
        zen_draw_frame(window2);

    }
    
    zen_destroy_window(window2);
    zen_destroy_window(window);

    return 0;

}