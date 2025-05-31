#include "../include/zenlib_core.h"

int zen_initialize_renderer(ZEN_Window* window, ZEN_RendererAPI api) {

    if (window == NULL) return -1;

    if (api == ZEN_RAPI_Vulkan) {
        if (zen_init_vulkan(window, ZEN_VULKAN_VERSION) < 0)
            return -1;
        debug_log_va("Vulkan Initialized for window %llu", __zencore_context__.window_count);
    }
    
    return 0;

}

int zen_destroy_renderer(ZEN_Window* window, ZEN_RendererAPI api) {
    
    if (api == ZEN_RAPI_None) return 0;
    if (api == ZEN_RAPI_Vulkan) zen_destroy_vulkan(window->renderer_context_index);
    
    return 0;

}

int zen_append_shader(ZEN_Shader object) {

    (void)object;
    return;

}

void zen_destroy_shader(void) {

    return;

}

ZEN_RenderObjectRef zen_append_render_object(ZEN_RenderObject object) {

    size_t new_count = (__zencore_context__.render_object_count + 1);
    if (new_count >= __zencore_context__.render_object_capacity) {

        __zencore_context__.render_object_capacity += (new_count * 2);
        ZEN_RenderObject* temp = (ZEN_RenderObject*)realloc(__zencore_context__.render_objects, sizeof(ZEN_RenderObject) * __zencore_context__.render_object_capacity); 
        if (temp == NULL) {
            return (ZEN_RenderObjectRef) {
                .index = -1,
                .object = NULL
            };
        }
        __zencore_context__.render_objects = temp;

    }

    __zencore_context__.render_objects[__zencore_context__.render_object_count++] = object;
    return (ZEN_RenderObjectRef) {
        .index = __zencore_context__.render_object_count - 1,
        .object = &__zencore_context__.render_objects[__zencore_context__.render_object_count - 1]
    };
        
}

int zen_remove_render_object(ZEN_RenderObjectRef object) {

    if (object.index >= __zencore_context__.render_object_count)
        return -1;

    size_t count = __zencore_context__.render_object_count;
    if (object.index < count - 1) {
        memmove(
            &__zencore_context__.render_objects[object.index],
            &__zencore_context__.render_objects[object.index + 1],
            (count - object.index - 1) * sizeof(ZEN_RenderObject)
        );
    }
    __zencore_context__.render_object_count--;

    return 0;

}

int zen_clear_render_objects(void) {

    __zencore_context__.render_object_count = 0;
    memset(__zencore_context__.render_objects, 0, __zencore_context__.render_object_capacity * sizeof(ZEN_RenderObject));

    return 0;

}