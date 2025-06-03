#include "../../include/zenlib_core.h"

int zen_vk_draw_frame(size_t context_index) {
    
    ZEN_VulkanContext* context = &__zencore_context__.vk_context;
    ZEN_VulkanSurfaceInfo* info = &context->surfaces[context_index];
    size_t current_frame = context->current_frame;

    // Calculate Delta Time
    info->window->current_frame = clock();
    info->window->delta_time = (double)(info->window->current_frame - info->window->last_frame) / CLOCKS_PER_SEC;

    // Calculate Frame Rate
    if (zen_stopwatch_elapsed_seconds(&info->window->frame_timer) >= 1) {
        zen_stopwatch_stop(&info->window->frame_timer);
        info->window->frame_rate = info->window->frame_count;
        info->window->frame_count = 0;
    }

    if (info->window->frame_count == 0)
        zen_stopwatch_start(&info->window->frame_timer);
    
    vkQueueWaitIdle(__zencore_context__.vk_context.present_queue);

    if (info->window->event_handler.resized) {
        info->window->event_handler.resized = false;
        zen_vk_recreate_swapchain(context_index);
        return 0;
    }

    vkWaitForFences(context->device, 1, &info->in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR (
        context->device,
        info->swap_chain,
        UINT64_MAX,
        info->image_available_semaphores[current_frame],
        VK_NULL_HANDLE,
        &image_index
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        zen_vk_recreate_swapchain(context_index);
        return 0;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        log_error("Failed to acquire swap chain image.");
        exit(1);
    }

    vkResetFences(context->device, 1, &info->in_flight_fences[current_frame]);
    vkResetCommandBuffer(info->command_buffers[current_frame], 0);

    VkCommandBuffer cmd = info->command_buffers[current_frame];

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    vkBeginCommandBuffer(cmd, &begin_info);

    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = context->render_pass,
        .framebuffer = info->frame_buffers[image_index],
        .renderArea = { {0, 0}, info->swap_chain_extent },
        .clearValueCount = 1,
        .pClearValues = (VkClearValue[]){{.color = {{0.0f, 0.0f, 0.0f, 1.0f}}}},
    };

    vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)info->swap_chain_extent.width,
        .height = (float)info->swap_chain_extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = info->swap_chain_extent
    };
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    for (size_t i = 0; i < __zencore_context__.render_object_count; i++) {

        ZEN_RenderObject* obj = &__zencore_context__.render_objects[i];
        if (!obj->enabled || obj->vertex_count == 0)
            continue;

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, context->graphics_pipelines[context->shaders[obj->shader].pipeline].graphics_pipeline);
        VkBuffer vertex_buffers[] = { context->vertex_buffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, vertex_buffers, offsets);
        vkCmdDraw(cmd, (uint32_t)obj->vertex_count, 1, zen_get_vertex_count_at_index(obj->index), 0);
    
    }

    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    VkSemaphore wait_semaphores[] = { info->image_available_semaphores[current_frame] };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signal_semaphores[] = { info->render_finished_semaphores[current_frame] };

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signal_semaphores,
    };

    if (vkQueueSubmit(context->graphics_queue, 1, &submit_info, info->in_flight_fences[current_frame]) != VK_SUCCESS) {
        log_error("Failed to submit draw command buffer.");
        return -1;
    }

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = 1,
        .pSwapchains = &info->swap_chain,
        .pImageIndices = &image_index,
    };

    result = vkQueuePresentKHR(context->present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || info->window->event_handler.resized) {
        info->window->event_handler.resized = false;
        zen_vk_recreate_swapchain(context_index);
    } else if (result != VK_SUCCESS) {
        log_error("Failed to acquire swap chain image.");
        exit(1);
    }

    info->window->frame_count++;
    info->window->last_frame = info->window->current_frame;
    context->current_frame = (current_frame + 1) % ZEN_MAX_FRAMES_IN_FLIGHT;
    return 0;

}