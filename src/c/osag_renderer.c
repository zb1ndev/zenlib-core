#include "../include/zenlib_core.h"

#pragma region Renderer

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
        
        bool is_last = __zencore_context__.window_count == 1;

        if (api == ZEN_RAPI_Vulkan)
            zen_destroy_vulkan(is_last, window->renderer_context_index);
        
        return 0;

    }

    int zen_draw_frame(ZEN_Window* window) {

        if (window == NULL) return -1;
        if (window->api == ZEN_RAPI_Vulkan)
            if (zen_vk_draw_frame(window->renderer_context_index) < 0)
                return -1;
        
        return 0;

    }

    float zen_get_fps(ZEN_Window* window) {
        
        (void)window;
        return .0f;

    }

    float zen_get_delta_time(ZEN_Window* window) {
        return window->event_handler.delta_time;
    }

#pragma endregion // Renderer
#pragma region Shaders

    size_t zen_append_shader(ZEN_Shader shader) {

        size_t new_count = (__zencore_context__.vk_context.shader_count + 1);
        if (new_count >= __zencore_context__.vk_context.shader_capacity) {

            __zencore_context__.vk_context.shader_capacity += (new_count * 2);
            ZEN_Shader* temp = (ZEN_Shader*)realloc(__zencore_context__.vk_context.shaders, sizeof(ZEN_Shader) * __zencore_context__.vk_context.shader_capacity); 
            if (temp == NULL) {
                log_error("Failed to allocate space for shaders.");
                return 0;
            }
            __zencore_context__.vk_context.shaders = temp;

        }

        ZEN_Shader* result = &__zencore_context__.vk_context.shaders[__zencore_context__.vk_context.shader_count++];

        result->name = malloc(strlen(shader.name) + 1);
        strcpy(result->name, shader.name);

        result->vertex_shader_path = malloc(strlen(shader.vertex_shader_path) + 1);
        strcpy(result->vertex_shader_path, shader.vertex_shader_path);

        result->fragment_shader_path = malloc(strlen(shader.fragment_shader_path) + 1);
        strcpy(result->fragment_shader_path, shader.fragment_shader_path);

        if (__zencore_context__.vk_context.info.initialized)
            zen_vk_append_graphics_pipeline(__zencore_context__.vk_context.shader_count - 1);
        return __zencore_context__.vk_context.shader_count - 1;

    }

#pragma endregion // Shaders
#pragma region Render Objects

    size_t zen_append_render_object(ZEN_RenderObject object) {

        size_t index = __zencore_context__.render_object_count;
        size_t new_count = (__zencore_context__.render_object_count + 1);
        if (new_count >= __zencore_context__.render_object_capacity) {

            __zencore_context__.render_object_capacity += (new_count * 2);
            ZEN_RenderObject* temp = (ZEN_RenderObject*)realloc(__zencore_context__.render_objects, sizeof(ZEN_RenderObject) * __zencore_context__.render_object_capacity); 
            if (temp == NULL)
                return 0;
            __zencore_context__.render_objects = temp;

        }
    
        __zencore_context__.render_objects[__zencore_context__.render_object_count] = (ZEN_RenderObject) {
            .enabled = object.enabled,
            .shader = object.shader,
            .vertex_count = object.vertex_count,
            .index_count = object.index_count,
            .index = index
        };

        __zencore_context__.render_objects[__zencore_context__.render_object_count].vertices = malloc(sizeof(ZEN_Vertex) * object.vertex_count);
        if (__zencore_context__.render_objects[__zencore_context__.render_object_count].vertices == NULL) {
            log_error("Failed to allocate space for verticies.");
            return SIZE_MAX;
        }

        memcpy (
            __zencore_context__.render_objects[__zencore_context__.render_object_count].vertices,
            object.vertices, sizeof(ZEN_Vertex) * object.vertex_count
        );

        __zencore_context__.render_object_count++;
        if (__zencore_context__.render_object_last_count <= __zencore_context__.render_object_count) 
            zen_vk_resize_vertex_buffer();

        return index;
            
    }

    int zen_remove_render_object(ZEN_RenderObject* object) {

        if (object->index >= __zencore_context__.render_object_count)
            return -1;

        size_t count = __zencore_context__.render_object_count;
        if (object->index < count - 1) {
            memmove(
                &__zencore_context__.render_objects[object->index],
                &__zencore_context__.render_objects[object->index + 1],
                (count - object->index - 1) * sizeof(ZEN_RenderObject)
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

#pragma endregion // Render Objects
#pragma region Vertices

    uint64_t zen_get_vertex_count(void)  {
        
        uint64_t result = 0;
        for (size_t i = 0; i < __zencore_context__.render_object_count; i++)
            result += __zencore_context__.render_objects[i].vertex_count;
        return result;

    }

    uint64_t zen_get_vertex_count_at_index(size_t index) {

        if (index > __zencore_context__.render_object_count) {
            log_error("Index is out of range at function \"zen_get_vertex_count_at_index\".");
            return 0;
        }

        uint64_t result = 0;
        for (size_t i = 0; i < index; i++)
            result += __zencore_context__.render_objects[i].vertex_count;
        return result;

    }

    ZEN_Vertex* zen_get_vertices() {

        ZEN_Vertex* result = malloc(sizeof(ZEN_Vertex) * zen_get_vertex_count());
        if (result == NULL) {
            log_error("Failed to allocate space for vertices.");
            return NULL;
        }

        size_t result_index = 0;
        for (size_t i = 0; i < __zencore_context__.render_object_count; i++) {
            memcpy(&result[result_index], __zencore_context__.render_objects[i].vertices, sizeof(ZEN_Vertex) * __zencore_context__.render_objects[i].vertex_count);
            result_index += __zencore_context__.render_objects[i].vertex_count;
        }
        
        return result;

    }

#pragma endregion // Vertices
