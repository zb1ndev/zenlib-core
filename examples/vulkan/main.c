#include "../../src/include/zenlib_core.h"

int main(int argc, char** argv) {
    
    (void)argc;
    (void)argv;

    ZEN_RenderObject triangle = (ZEN_RenderObject){
        .vertex_count = 3,
        .vertices = (ZEN_Vertex[]){
            { .pos = {  0.0f, -0.5f }, .color = { 1.0f, 0.0f, 0.0f } }, // Red
            { .pos = {  0.5f,  0.5f }, .color = { 0.0f, 1.0f, 0.0f } }, // Green
            { .pos = { -0.5f,  0.5f }, .color = { 0.0f, 0.0f, 1.0f } }, // Blue
        },
        .index_count = 3,
        .indices = (int[]){ 0, 1, 2 },
    };

    ZEN_RenderObjectRef tri_ref = zen_append_render_object(triangle);
    zen_remove_render_object(tri_ref);
    zen_clear_render_objects();

    // ZEN_Window* window = zen_create_window("Game Window", 1280, 720, ZEN_RAPI_Vulkan);

    // while(!zen_window_should_close(window)) {
        
    //     if (zen_get_key_down(window, Z_KEY_Escape))
    //         break;

    //     ZEN_RenderObject triangle = (ZEN_RenderObject){
    //         .vertex_count = 3,
    //         .vertices = (ZEN_Vertex[]){
    //             { .pos = {  0.0f, -0.5f }, .color = { 1.0f, 0.0f, 0.0f } }, // Red
    //             { .pos = {  0.5f,  0.5f }, .color = { 0.0f, 1.0f, 0.0f } }, // Green
    //             { .pos = { -0.5f,  0.5f }, .color = { 0.0f, 0.0f, 1.0f } }, // Blue
    //         },
    //         .index_count = 3,
    //         .indices = (int[]){ 0, 1, 2 },
    //     };

    //     zen_append_render_object(triangle);

    //     zen_draw_frame(window);
        

    // }
    
    // zen_destroy_window(window);

    return 0;

}