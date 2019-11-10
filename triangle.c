#include "triangle.h"

#include "vk/instance.h"
#include "vk/device.h"
#include "vk/swapchain.h"
#include "vk/pipeline.h"
#include "vk/command.h"
#include "vk/buffer.h"

void init(Context* context) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    context->window = glfwCreateWindow(context->width, context->height, "Triangle", NULL, NULL);

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

    context->vertexShader = loadShader(context, "res/shaders/vert.spv");
    context->fragmentShader = loadShader(context, "res/shaders/frag.spv");
    context->setLayout = createSetLayout(context, NULL);
    context->layout = createPipelineLayout(context);
    context->pipeline = createGraphicsPipeline(context);

    context->commandPool = createCommandPool(context);
    context->commandBuffer = createCommandBuffer(context);
    VkDescriptorPoolSize size = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 };
    context->descriptorPool = createDescriptorPool(context, &size);
    // context->descriptorSet = createDescriptorSet(context);

    context->buffers = malloc(sizeof(Buffer) * 3);
    context->buffers[0] = createBuffer(context, 128 * 1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    context->buffers[1] = createBuffer(context, 128 * 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    context->buffers[2] = createBuffer(context, 128 * 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Vertex vertices[] = {
        {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{0.0f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };

    uint16_t indices[] = {0, 1, 2};

    uploadBuffer(context, context->buffers[0], context->buffers[1], vertices, sizeof(vertices));
    uploadBuffer(context, context->buffers[0], context->buffers[2], indices, sizeof(indices));

    context->acquireSemaphore = createSemaphore(context);
    context->releaseSemaphore = createSemaphore(context);
}

void mainLoop(Context* context) {
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

        uint32_t imageIndex;
        vkAcquireNextImageKHR(context->device, context->swapchain, 0, context->acquireSemaphore, VK_NULL_HANDLE, &imageIndex);

        ASSERT(vkResetCommandPool(context->device, context->commandPool, 0), "reset");

        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        ASSERT(vkBeginCommandBuffer(context->commandBuffer, &beginInfo), "begin");

        VkClearColorValue colorValue = {0.0f, 0.0f, 0.2f, 1.0f};
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
        viewport.height = (float)context->height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vkCmdSetViewport(context->commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.extent.width = context->width;
        scissor.extent.height = context->height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;

        vkCmdSetScissor(context->commandBuffer, 0, 1, &scissor);

        // vkCmdBindDescriptorSets(context->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->layout, 0, 1, &context->descriptorSet, 0, NULL);
        vkCmdBindPipeline(context->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->pipeline);
        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(context->commandBuffer, 0, 1, &context->buffers[1].buffer, offsets);
        vkCmdBindIndexBuffer(context->commandBuffer, context->buffers[2].buffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(context->commandBuffer, 3, 1, 0, 0, 1);

        vkCmdEndRenderPass(context->commandBuffer);

        ASSERT(vkEndCommandBuffer(context->commandBuffer), "end");

        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &context->acquireSemaphore;
        submitInfo.pWaitDstStageMask = &waitStageMask;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &context->commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &context->releaseSemaphore;

        ASSERT(vkQueueSubmit(context->queue, 1, &submitInfo, VK_NULL_HANDLE), "submit");

        VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &context->releaseSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &context->swapchain;
        presentInfo.pImageIndices = &imageIndex;

        ASSERT(vkQueuePresentKHR(context->queue, &presentInfo), "present");

        ASSERT(vkDeviceWaitIdle(context->device), "wait");
    }
}

void clean(Context* context) {
    vkDestroySemaphore(context->device, context->releaseSemaphore, NULL);
    vkDestroySemaphore(context->device, context->acquireSemaphore, NULL);

    destroyBuffer(context, context->buffers[2]);
    destroyBuffer(context, context->buffers[1]);
    destroyBuffer(context, context->buffers[0]);
    free(context->buffers);

    vkDestroyDescriptorPool(context->device, context->descriptorPool, NULL);
    vkDestroyCommandPool(context->device, context->commandPool, NULL);

    vkDestroyPipeline(context->device, context->pipeline, NULL);
    vkDestroyPipelineLayout(context->device, context->layout, NULL);
    vkDestroyDescriptorSetLayout(context->device, context->setLayout, NULL);

    destroySwapchainStuff(context);
    vkDestroyRenderPass(context->device, context->renderPass, NULL);
    vkDestroySwapchainKHR(context->device, context->swapchain, NULL);
    destroyDepthStencil(context);

    vkDestroyDevice(context->device, NULL);
    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
    vkDestroyInstance(context->instance, NULL);

    glfwTerminate();
}

int main(int argc, char const *argv[]) {
    Context* context = malloc(sizeof(Context));
    context->width = 800;
    context->height = 600;

    init(context);
    mainLoop(context);
    clean(context);

    free(context);
    return 0;
}
