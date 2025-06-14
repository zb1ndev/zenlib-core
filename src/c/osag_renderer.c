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

    void zen_set_clear_color(ZEN_Window* window, vec4 color) { 
        glm_vec4_dup(color, window->clear_color); 
    }

    void zen_set_view_mode(ZEN_Window* window, ZEN_ViewMode mode) {
        window->view_mode = mode;
    }

    int zen_draw_frame(ZEN_Window* window) {

        ZEN_RendererContext* context = &__zencore_context__.renderer_context;

        // Calculate Delta Time
        window->current_frame = clock();
        window->delta_time = (double)(window->current_frame - window->last_frame) / CLOCKS_PER_SEC;

        // Calculate Frame Rate
        if (zen_stopwatch_elapsed_seconds(&window->frame_timer) >= 1) {
            zen_stopwatch_stop(&window->frame_timer);
            window->frame_rate = window->frame_count;
            window->frame_count = 0;
        }

        // Start Timing Frame
        if (window->frame_count == 0)
            zen_stopwatch_start(&window->frame_timer);

        // Draw Frame ( includes resizing vertex buffer )
        if (window == NULL) return -1;
        if (window->api == ZEN_RAPI_Vulkan) {
            if (context->render_object_last_count < context->render_object_count && context->render_object_count > 0) {
                context->render_object_last_count = context->render_object_count;
                zen_vk_resize_vertex_buffer();
            }
            if (zen_vk_draw_frame(window->renderer_context_index) < 0)
                return -1;
        }

        window->frame_count++;
        window->last_frame = window->current_frame;
        return 0;

    }

    float zen_get_fps(ZEN_Window* window) { 
        return (float)window->frame_rate; 
    }

    float zen_get_delta_time(ZEN_Window* window) { 
        return window->delta_time; 
    }

#pragma endregion // Renderer
#pragma region Shaders

    size_t zen_append_shader(ZEN_Shader shader) {

        size_t new_count = (__zencore_context__.renderer_context.shader_count + 1);
        if (new_count >= __zencore_context__.renderer_context.shader_capacity) {

            __zencore_context__.renderer_context.shader_capacity += (new_count * 2);
            ZEN_Shader* temp = (ZEN_Shader*)realloc(__zencore_context__.renderer_context.shaders, sizeof(ZEN_Shader) * __zencore_context__.renderer_context.shader_capacity); 
            if (temp == NULL) {
                log_error("Failed to allocate space for shaders.");
                return 0;
            }
            __zencore_context__.renderer_context.shaders = temp;

        }

        ZEN_Shader* result = &__zencore_context__.renderer_context.shaders[__zencore_context__.renderer_context.shader_count++];

        result->name = malloc(strlen(shader.name) + 1);
        strcpy(result->name, shader.name);

        result->vertex_shader_path = malloc(strlen(shader.vertex_shader_path) + 1);
        strcpy(result->vertex_shader_path, shader.vertex_shader_path);

        result->fragment_shader_path = malloc(strlen(shader.fragment_shader_path) + 1);
        strcpy(result->fragment_shader_path, shader.fragment_shader_path);

        if (__zencore_context__.vk_context.info.initialized)
            zen_vk_append_graphics_pipeline(__zencore_context__.renderer_context.shader_count - 1);
        return __zencore_context__.renderer_context.shader_count - 1;

    }

    ZEN_Shader* zen_get_shader(size_t index) {
        return &__zencore_context__.renderer_context.shaders[index];
    }

#pragma endregion // Shaders
#pragma region Render Objects

    size_t zen_append_render_object(ZEN_RenderObject object) {

        size_t index = __zencore_context__.renderer_context.render_object_count;
        size_t new_count = (__zencore_context__.renderer_context.render_object_count + 1);
        if (new_count >= __zencore_context__.renderer_context.render_object_capacity) {

            __zencore_context__.renderer_context.render_object_capacity += (new_count * 2);
            ZEN_RenderObject* temp = (ZEN_RenderObject*)realloc (
                __zencore_context__.renderer_context.render_objects, 
                sizeof(ZEN_RenderObject) * __zencore_context__.renderer_context.render_object_capacity
            ); 
            if (temp == NULL)
                return 0;
            __zencore_context__.renderer_context.render_objects = temp;

        }
        
        ZEN_RenderObject* obj = &__zencore_context__.renderer_context.render_objects[__zencore_context__.renderer_context.render_object_count];

        *obj = (ZEN_RenderObject) {
            .enabled = object.enabled,
            .shader = object.shader,
            .vertex_count = object.vertex_count,
            .index_count = object.index_count,
            .index = index
        };

        obj->vertices = malloc(sizeof(ZEN_Vertex) * object.vertex_count);
        if (obj->vertices == NULL) {
            log_error("Failed to allocate space for verticies.");
            return SIZE_MAX;
        }
        memcpy(obj->vertices, object.vertices, sizeof(ZEN_Vertex) * object.vertex_count);

        obj->indices = malloc(sizeof(ZEN_Vertex) * object.index_count);
        if (obj->indices == NULL) {
            log_error("Failed to allocate space for verticies.");
            return SIZE_MAX;
        }
        memcpy(obj->indices, object.indices, sizeof(uint16_t) * object.index_count);


        glm_vec3_copy(object.transform.position, obj->transform.position);
        glm_vec3_copy(object.transform.scale, obj->transform.scale);
        glm_vec4_copy(object.transform.rotation, obj->transform.rotation);

        __zencore_context__.renderer_context.render_object_count++;
        return index;
            
    }

    ZEN_RenderObject* zen_get_render_object(size_t index) {
        return &__zencore_context__.renderer_context.render_objects[index];
    }

    int zen_remove_render_object(ZEN_RenderObject* object) {

        if (object->index >= __zencore_context__.renderer_context.render_object_count)
            return -1;

        size_t count = __zencore_context__.renderer_context.render_object_count;
        if (object->index < count - 1) {
            memmove(
                &__zencore_context__.renderer_context.render_objects[object->index],
                &__zencore_context__.renderer_context.render_objects[object->index + 1],
                (count - object->index - 1) * sizeof(ZEN_RenderObject)
            );
        }

        __zencore_context__.renderer_context.render_object_count--;
        return 0;

    }

    int zen_clear_render_objects(void) {

        __zencore_context__.renderer_context.render_object_count = 0;
        memset(__zencore_context__.renderer_context.render_objects, 0, __zencore_context__.renderer_context.render_object_capacity * sizeof(ZEN_RenderObject));
        return 0;

    }

#pragma endregion // Render Objects
#pragma region Vertices

    uint64_t zen_get_vertex_count(void)  {
        
        uint64_t result = 0;
        for (size_t i = 0; i < __zencore_context__.renderer_context.render_object_count; i++)
            result += __zencore_context__.renderer_context.render_objects[i].vertex_count;
        return result;

    }

    uint64_t zen_get_vertex_count_at_index(size_t index) {

        if (index > __zencore_context__.renderer_context.render_object_count) {
            log_error("Index is out of range at function \"zen_get_vertex_count_at_index\".");
            return 0;
        }

        uint64_t result = 0;
        for (size_t i = 0; i < index; i++)
            result += __zencore_context__.renderer_context.render_objects[i].vertex_count;
        return result;

    }

    ZEN_Vertex* zen_get_vertices() {

        ZEN_Vertex* result = malloc(sizeof(ZEN_Vertex) * zen_get_vertex_count());
        if (result == NULL) {
            log_error("Failed to allocate space for vertices.");
            return NULL;
        }

        size_t result_index = 0;
        for (size_t i = 0; i < __zencore_context__.renderer_context.render_object_count; i++) {
            memcpy (
                &result[result_index], 
                __zencore_context__.renderer_context.render_objects[i].vertices, 
                sizeof(ZEN_Vertex) * __zencore_context__.renderer_context.render_objects[i].vertex_count
            );
            result_index += __zencore_context__.renderer_context.render_objects[i].vertex_count;
        }
        
        return result;

    }

#pragma endregion // Vertices
#pragma region Indices

    uint64_t zen_get_index_count(void)  {
        
        uint64_t result = 0;
        for (size_t i = 0; i < __zencore_context__.renderer_context.render_object_count; i++)
            result += __zencore_context__.renderer_context.render_objects[i].index_count;
        return result;

    }

    uint64_t zen_get_index_count_at_index(size_t index) {

        if (index > __zencore_context__.renderer_context.render_object_count) {
            log_error("Index is out of range at function \"zen_get_vertex_count_at_index\".");
            return 0;
        }

        uint64_t result = 0;
        for (size_t i = 0; i < index; i++)
            result += __zencore_context__.renderer_context.render_objects[i].index_count;
        return result;

    }

    uint16_t* zen_get_indices() {

        uint16_t* result = malloc(sizeof(uint16_t) * zen_get_index_count());
        if (result == NULL) {
            log_error("Failed to allocate space for indices.");
            return NULL;
        }

        size_t result_index = 0;
        for (size_t o = 0; o < __zencore_context__.renderer_context.render_object_count; o++) {
            for (size_t i = 0; i < __zencore_context__.renderer_context.render_objects[o].index_count; i++) {
                result[result_index++] = __zencore_context__.renderer_context.render_objects[o].indices[i];
            }
        }
        
        return result;

    }

#pragma endregion // Indices
#pragma region Transform

    void zen_scale_vec3(vec3 a, float x_scale, float y_scale, float z_scale, vec3 dest) {
        
        dest[0] = a[0] * x_scale;
        dest[1] = a[1] * y_scale;
        dest[2] = a[2] * z_scale;

    }

    // 2D Order of ops
    void zen_make_model_from_transform_2d(ZEN_Window* window, ZEN_Transform* transform, vec4* model) {

        size_t* size = zen_get_window_size(window);
        float x_scaling = 1.0f / size[0];
        float y_scaling = 1.0f / size[1];

        vec3 scaled_position;
        vec3 scaled_scale;

        zen_scale_vec3(transform->position, x_scaling, y_scaling, .0f, scaled_position);
        zen_scale_vec3(transform->scale, x_scaling, y_scaling, .0f, scaled_scale);
    
        glm_mat4_identity(model);

        glm_translate(model, scaled_position);
        glm_scale(model, scaled_scale);

        float angle = transform->rotation[2];
        glm_make_rad(&angle);
        glm_rotate(model, angle, (vec3){0.0f, 0.0f, 1.0f});

    }

#pragma endregion // Transform