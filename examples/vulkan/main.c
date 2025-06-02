#include "../../src/include/zenlib_core.h"

ZEN_RenderObjectRef draw_triangle(size_t shader, vec3 color) {

    return zen_append_render_object((ZEN_RenderObject) {
        .enabled = true,
        .vertex_count = 3,
        .vertices = (ZEN_Vertex[]) {
            { .pos = {  0.0f, -0.5f }, .color = {color[0], color[1], color[2]} }, // Bottom center (red)
            { .pos = {  0.5f,  0.5f }, .color = {color[0], color[1], color[2]} }, // Top right (green)
            { .pos = { -.0f,  0.5f }, .color = {color[0], color[1], color[2]} }, // Top left (blue)
        },
        .index_count = 0,  // Not using indexed drawing atm
        .indices = NULL,
        .shader = shader
    });
    
}

ZEN_RenderObjectRef draw_square(size_t shader, vec3 color) {

    return zen_append_render_object((ZEN_RenderObject) {
        .enabled = true,
        .vertex_count = 6,
        .vertices = (ZEN_Vertex[]) {
            // First triangle
            { .pos = { -0.5f, -0.5f }, .color = { color[0], color[1], color[2] } }, // Bottom left
            { .pos = {  .0f, -0.5f }, .color = { color[0], color[1], color[2] } }, // Bottom right
            { .pos = {  .0f,  0.5f }, .color = { color[0], color[1], color[2] } }, // Top right

            // Second triangle
            { .pos = {  .0f,  0.5f }, .color = { color[0], color[1], color[2] } }, // Top right
            { .pos = { -0.5f,  0.5f }, .color = { color[0], color[1], color[2] } }, // Top left
            { .pos = { -0.5f, -0.5f }, .color = { color[0], color[1], color[2] } }, // Bottom left
        },
        .index_count = 0,  // Not using indexed drawing atm
        .indices = NULL,
        .shader = shader
    });
    
}

int main(int argc, char** argv) {
    
    (void)argc;
    (void)argv;

    size_t index = zen_append_shader((ZEN_Shader) {
        .name = "main",
        .fragment_shader_path = "./examples/vulkan/shaders/compiled/frag.spv",
        .vertex_shader_path = "./examples/vulkan/shaders/compiled/vert.spv"
    });

    size_t index2 = zen_append_shader((ZEN_Shader) {
        .name = "opacity",
        .fragment_shader_path = "./examples/vulkan/shaders/compiled/frag2.spv",
        .vertex_shader_path = "./examples/vulkan/shaders/compiled/vert.spv"
    });
    
    ZEN_RenderObjectRef ref = draw_triangle(index, (vec3){ 1.0f, 1.0f, 1.0f });
    ZEN_RenderObjectRef ref2 = draw_triangle(index2, (vec3){ 1.0f, 1.0f, 1.0f });
    ZEN_RenderObjectRef ref3 = draw_square(index, (vec3){ 1.0f, 1.0f, 1.0f });

    ZEN_Window* window = zen_create_window("Game Window", 1280, 720, ZEN_RAPI_Vulkan);
    ZEN_Window* window2 = zen_create_window("Game Window2", 1280, 720, ZEN_RAPI_Vulkan);
    ZEN_Window* window3 = zen_create_window("Game Window2", 1280, 720, ZEN_RAPI_Vulkan);

    while (!zen_window_should_close(window)) {
        
        if (zen_get_key_down(window, Z_KEY_Escape))
            break;
        // if (zen_window_should_close(window2))
            // break;

        ref2.object->enabled = false;
        ref.object->enabled = true;
        zen_draw_frame(window);

        // ref2.object->enabled = true;
        // ref.object->enabled = false;
        zen_draw_frame(window2);
        zen_draw_frame(window3);
        

    }
    
    
    zen_destroy_window(window3);
    zen_destroy_window(window2);
    zen_destroy_window(window);

    return 0;

}