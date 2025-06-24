#include "../../src/zenlib_core.h"

size_t draw_square(const vec3 position, const vec3 scale, const vec3 color, size_t shader, size_t texture) {
     
    return zen_append_render_object((ZEN_RenderObject) {

        .enabled = true,
        .shader = shader,
        .coord_system = ZEN_SCREEN_SPACE,

        .vertex_count = 4,
        .vertices = (ZEN_Vertex[]) {
            {{.0f, .0f},     {color[0], color[1], color[2]}, {1.0f, 0.0f}},
            {{1.0f, .0f},    {color[0], color[1], color[2]}, {0.0f, 0.0f}},
            {{1.0f, -1.0f},  {color[0], color[1], color[2]}, {0.0f, 1.0f}},
            {{.0f, -1.0f},   {color[0], color[1], color[2]}, {1.0f, 1.0f}},
           
        },
        
        .index_count = 6,
        .indices = (uint16_t[]) { 0, 1, 2, 2, 3, 0 },

        .texture = texture,

        .transform = (ZEN_Transform) {
            .position = {position[0], position[1], position[2]},
            .scale = {scale[0], scale[1], scale[2]},
            .rotation = {.0f, .0f, .0f, .0f}
        }

    });

}

size_t load_circle(const char* path) {

    int width, height, channels;
    stbi_uc* data = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
    return zen_vk_upload_image_data(data, width, height);

}

int main(int argc, char** argv) {
    
    (void)argc;
    (void)argv; 
    size_t default_shader = zen_append_shader((ZEN_Shader) {
        
        .name = "default",
        .vertex_shader_path = "./examples/vulkan/shaders/compiled/vert.spv",
        .fragment_shader_path = "./examples/vulkan/shaders/compiled/frag.spv"

    });

    ZEN_Window* window = zen_create_window("Game Window", 1280, 720, ZEN_RAPI_Vulkan, false);
    
    zen_set_view_mode(window, ZEN_VIEW_MODE_2D);
    zen_set_clear_color(window, (vec4){0.01f,0.01f,0.01f,0.0f});
    zen_set_window_icon(window, "./examples/vulkan/resources/logo.ico");
    
    draw_square((vec3){5.0f,  5.0f, .0f}, (vec3){20.0f,20.0f,.0f}, (vec3){1.0f, .0f , .0f}, default_shader, 1);
    draw_square((vec3){20.0f, 5.0f, .0f}, (vec3){20.0f,20.0f,.0f}, (vec3){.0f , 1.0f, .0f}, default_shader, 1);
    draw_square((vec3){35.0f, 5.0f, .0f}, (vec3){20.0f,20.0f,.0f}, (vec3){1.0f, 1.0f, .0f}, default_shader, 1);

    bool loaded_resources = false;

    while (!zen_window_should_close(window)) {

        if (!loaded_resources) {
            load_circle("./examples/vulkan/resources/circle.png");
            loaded_resources = true;
        }

        if (zen_get_key_down(window, Z_KEY_Escape))
            break;

        
        
        zen_draw_frame(window);

    }

    zen_destroy_window(window);
    return 0;

}