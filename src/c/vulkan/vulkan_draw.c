#include "../../include/zenlib_core.h"

int zen_vk_draw_frame(size_t context_index) {
    
    ZEN_VulkanContext* context = &__zencore_context__.vk_context;
    ZEN_VulkanSurfaceInfo* surface = &context->surfaces[context_index];
    size_t current_frame = context->current_frame;

    vkQueueWaitIdle(__zencore_context__.vk_context.present_queue);
    vkWaitForFences(context->device, 1, &surface->in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(
        context->device,
        surface->swap_chain,
        UINT64_MAX,
        surface->image_available_semaphores[current_frame],
        VK_NULL_HANDLE,
        &image_index
    );

    if (result != VK_SUCCESS) {
        log_error("Failed to acquire swapchain image.");
        return -1;
    }

    vkResetFences(context->device, 1, &surface->in_flight_fences[current_frame]);
    vkResetCommandBuffer(surface->command_buffers[current_frame], 0);

    VkCommandBuffer cmd = surface->command_buffers[current_frame];

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    vkBeginCommandBuffer(cmd, &begin_info);

    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = context->render_pass,
        .framebuffer = surface->frame_buffers[image_index],
        .renderArea = { {0, 0}, surface->swap_chain_extent },
        .clearValueCount = 1,
        .pClearValues = (VkClearValue[]){{.color = {{0.0f, 0.0f, 0.0f, 1.0f}}}},
    };

    vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)surface->swap_chain_extent.width,
        .height = (float)surface->swap_chain_extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = surface->swap_chain_extent
    };
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    for (size_t i = 0; i < __zencore_context__.render_object_count; i++) {

        ZEN_RenderObject* obj = &__zencore_context__.render_objects[i];
        if (!obj->enabled || obj->vertex_count == 0 || __zencore_context__.vk_context.shaders[obj->shader].pipline == NULL)
            continue;

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, __zencore_context__.vk_context.shaders[obj->shader].pipline->graphics_pipeline);
        VkBuffer vertex_buffers[] = { context->vertex_buffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, vertex_buffers, offsets);
        vkCmdDraw(cmd, (uint32_t)obj->vertex_count, 1, zen_get_vertex_count_at_index(obj->index), 0);
    
    }

    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    VkSemaphore wait_semaphores[] = { surface->image_available_semaphores[current_frame] };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signal_semaphores[] = { surface->render_finished_semaphores[current_frame] };

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

    if (vkQueueSubmit(context->graphics_queue, 1, &submit_info, surface->in_flight_fences[current_frame]) != VK_SUCCESS) {
        log_error("Failed to submit draw command buffer.");
        return -1;
    }

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = 1,
        .pSwapchains = &surface->swap_chain,
        .pImageIndices = &image_index,
    };

    result = vkQueuePresentKHR(context->present_queue, &present_info);
    if (result != VK_SUCCESS) {
        log_error("Failed to present swapchain image.");
        return -1;
    }

    context->current_frame = (current_frame + 1) % ZEN_MAX_FRAMES_IN_FLIGHT;
    return 0;

}