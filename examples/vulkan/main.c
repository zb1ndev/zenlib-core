#include "../../src/zenlib_core.h"

size_t draw_square(const vec3 position, const vec3 scale, const vec4 rotation, size_t shader) {
    
    return zen_append_render_object((ZEN_RenderObject) {

        .enabled = true,
        .shader = shader,
        .coord_system = ZEN_SCREEN_SPACE,

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
            .position = {position[0], position[1], position[2]},
            .scale = {scale[0], scale[1], scale[2]},
            .rotation = {rotation[0], rotation[1], rotation[2], rotation[3]}
        }

    });

}

void move_player(ZEN_Window* window, ssize_t player_object) {

    if (player_object < 0)
        return;

    const float speed = 200.0f;
    int x_dir = 0, y_dir = 0;

    if (zen_get_key_pressed(window, Z_KEY_W))
        y_dir = -1;
    if (zen_get_key_pressed(window, Z_KEY_S))
        y_dir = 1;
    if (zen_get_key_pressed(window, Z_KEY_D))
        x_dir = 1;
    if (zen_get_key_pressed(window, Z_KEY_A))
        x_dir = -1;

    ZEN_RenderObject* player = zen_get_render_object((size_t)player_object);
    player->transform.position[0] += x_dir * speed * zen_get_delta_time(window);
    player->transform.position[1] += y_dir * speed * zen_get_delta_time(window);

}

int main(int argc, char** argv) {
    
    (void)argc;
    (void)argv;

    size_t default_shader = zen_append_shader((ZEN_Shader) {
        
        .name = "default",
        .vertex_shader_path = "./examples/vulkan/shaders/compiled/vert.spv",
        .fragment_shader_path = "./examples/vulkan/shaders/compiled/frag.spv"

    });

    ssize_t square_1 = -1;
    ZEN_Window* window = zen_create_window("Game Window", 1280, 720, ZEN_RAPI_Vulkan);
    
    zen_set_view_mode(window, ZEN_VIEW_MODE_2D);
    zen_set_clear_color(window, (vec4){0.02f,0.02f,0.02f,1.0f});
    zen_set_window_icon(window, "./examples/vulkan/resources/logo.ico");

    while (!zen_window_should_close(window)) {
        
        float fps = zen_get_fps(window);
        String title = string_from_format("Test Window | FPS : %f", fps);
        zen_set_window_title(window, title.content);
        string_drop(&title);

        if (zen_get_key_down(window, Z_KEY_Escape))
            break;

        if (zen_get_key_down(window, Z_KEY_Space))
            square_1 = draw_square((vec3){.0f,.0f,.0f}, (vec3){200.0f,200.0f,.0f}, (vec4){.0f,.0f,0.0f,.0f}, default_shader);
            
        move_player(window, square_1);
        zen_draw_frame(window);

    }

    zen_destroy_window(window);

    return 0;

}