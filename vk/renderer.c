#include "renderer.h"

void init(Context* context, void (*initFunc)(Context*)) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    context->width = 800;
    context->height = 600;

    context->window = glfwCreateWindow(context->width, context->height, "Suzanne", NULL, NULL);

    context->instance = createInstance();
    context->surface = createSurface(context);

    context->physicalDevice = getPhysicalDevice(context);
    context->familyIndex = getFamilyIndex(context);
    context->device = createDevice(context);
    context->queue = getQueue(context);

    context->format = getFormat(context);
    context->depthFormat = getDepthFormat(context);
    context->depthStencil = createDepthStencil(context);
    context->swapchain = createSwapchain(context);
    context->renderPass = createRenderPass(context);

    context->swapchainImageCount = getSwapchainImageCount(context);
    context->swapchainImages = getSwapchainImages(context);
    context->swapchainImageViews = createSwapchainImageViews(context);
    context->framebuffers = createFramebuffers(context);

    context->commandPool = createCommandPool(context);
    context->commandBuffer = createCommandBuffer(context);
    VkDescriptorPoolSize sizes[3];
    sizes[0].descriptorCount = 1;
    sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    sizes[1].descriptorCount = 1;
    sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sizes[2].descriptorCount = 1;
    sizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    context->descriptorPool = createDescriptorPool(context, sizes);

    context->acquireSemaphore = createSemaphore(context);
    context->releaseSemaphore = createSemaphore(context);

    context->stagingBuffer = createBuffer(context, 128 * 1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    glm_perspective_default((float)context->width / (float)context->height, context->uniform.projection);
    vec3 eye = {0, -2, -5};
    vec3 center = {0, 0, 0};
    vec3 up = {0, 1, 0};
    glm_lookat(eye, center, up, context->uniform.view);
    glm_mat4_identity(context->uniform.model);

    initFunc(context);
}

void mainLoop(Context* context, void (*gameLoopFunc)(Context*), void (*renderLoopFunc)(Context*)) {
    int newWidth, newHeight;
    while(!glfwWindowShouldClose(context->window)) {
        glfwPollEvents();

        glfwGetWindowSize(context->window, &newWidth, &newHeight);
        if(newWidth != context->width || newHeight != context->height) {
            context->width = newWidth;
            context->height = newHeight;

            destroySwapchainStuff(context);
            destroyDepthStencil(context);
            context->depthStencil = createDepthStencil(context);

            vkDestroySwapchainKHR(context->device, context->swapchain, NULL);
            context->swapchain = createSwapchain(context);

            context->swapchainImageCount = getSwapchainImageCount(context);
            context->swapchainImages = getSwapchainImages(context);
            context->swapchainImageViews = createSwapchainImageViews(context);
            context->framebuffers = createFramebuffers(context);
            // Weird hack
            // continue;
        }

        gameLoopFunc(context);

        uint32_t imageIndex;
        ASSERT(vkAcquireNextImageKHR(context->device, context->swapchain, UINT64_MAX, context->acquireSemaphore, VK_NULL_HANDLE, &imageIndex));

        startRecording(context);

        VkClearColorValue colorValue = {0.0f, 0.0f, 0.0f, 1.0f};
        VkClearDepthStencilValue depthValue = {1.0f, 0.0f};

        VkClearValue clearValues[2];
        clearValues[0].color = colorValue;
        clearValues[1].depthStencil = depthValue;

        VkRenderPassBeginInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderPassInfo.renderPass = context->renderPass;
        renderPassInfo.framebuffer = context->framebuffers[imageIndex];
        renderPassInfo.renderArea.extent.width = context->width;
        renderPassInfo.renderArea.extent.height = context->height;
        renderPassInfo.renderArea.offset.x = 0;
        renderPassInfo.renderArea.offset.y = 0;
        renderPassInfo.clearValueCount = ARRAYSIZE(clearValues);
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(context->commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.width = (float)context->width;
        // viewport.y = (float)context->height;
        viewport.height = (float)context->height;
        //viewport.minDepth = 0.0f;
        //viewport.maxDepth = 1.0f;

        vkCmdSetViewport(context->commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.extent.width = context->width;
        scissor.extent.height = context->height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;

        vkCmdSetScissor(context->commandBuffer, 0, 1, &scissor);

        renderLoopFunc(context);

        vkCmdEndRenderPass(context->commandBuffer);

        ASSERT(vkEndCommandBuffer(context->commandBuffer));

        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &context->acquireSemaphore;
        submitInfo.pWaitDstStageMask = &waitStageMask;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &context->commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &context->releaseSemaphore;

        ASSERT(vkQueueSubmit(context->queue, 1, &submitInfo, VK_NULL_HANDLE));

        VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &context->releaseSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &context->swapchain;
        presentInfo.pImageIndices = &imageIndex;

        ASSERT(vkQueuePresentKHR(context->queue, &presentInfo));

        ASSERT(vkDeviceWaitIdle(context->device));
    }
}

void clean(Context* context, void (*cleanFunc)(Context*)) {
    cleanFunc(context);

    destroyBuffer(context, context->stagingBuffer);

    vkDestroySemaphore(context->device, context->releaseSemaphore, NULL);
    vkDestroySemaphore(context->device, context->acquireSemaphore, NULL);
    vkDestroyDescriptorPool(context->device, context->descriptorPool, NULL);
    vkDestroyCommandPool(context->device, context->commandPool, NULL);

    destroySwapchainStuff(context);
    vkDestroyRenderPass(context->device, context->renderPass, NULL);
    vkDestroySwapchainKHR(context->device, context->swapchain, NULL);
    destroyDepthStencil(context);

    vkDestroyDevice(context->device, NULL);
    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
    vkDestroyInstance(context->instance, NULL);

    glfwTerminate();
}