#include "command.h"

VkCommandPool createCommandPool(Context* context) {
    VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = context->familyIndex;

    VkCommandPool commandPool;
    ASSERT(vkCreateCommandPool(context->device, &createInfo, NULL, &commandPool));
    return commandPool;
}

VkCommandBuffer createCommandBuffer(Context* context) {
    VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = context->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    ASSERT(vkAllocateCommandBuffers(context->device, &allocInfo, &commandBuffer));
    return commandBuffer;
}

VkDescriptorPool createDescriptorPool(Context* context, VkDescriptorPoolSize* poolSizes) {
    VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    createInfo.maxSets = 128;
    createInfo.poolSizeCount = ARRAYSIZE(poolSizes);
    createInfo.pPoolSizes = poolSizes;

    VkDescriptorPool descriptorPool;
    ASSERT(vkCreateDescriptorPool(context->device, &createInfo, NULL, &descriptorPool));
    return descriptorPool;
}

VkDescriptorSet createDescriptorSet(Context* context, VkDescriptorSetLayout setLayout) {
    VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.descriptorPool = context->descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &setLayout;

    VkDescriptorSet descriptorSet;
    ASSERT(vkAllocateDescriptorSets(context->device, &allocInfo, &descriptorSet));
    return descriptorSet;
}

VkSemaphore createSemaphore(Context* context) {
    VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

    VkSemaphore semaphore;
    ASSERT(vkCreateSemaphore(context->device, &createInfo, NULL, &semaphore));
    return semaphore;
}

void startRecording(Context* context) {
    ASSERT(vkResetCommandPool(context->device, context->commandPool, 0));

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    ASSERT(vkBeginCommandBuffer(context->commandBuffer, &beginInfo));
}

void endRecording(Context* context) {
    ASSERT(vkEndCommandBuffer(context->commandBuffer));

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &context->commandBuffer;

    ASSERT(vkQueueSubmit(context->queue, 1, &submitInfo, VK_NULL_HANDLE));

    ASSERT(vkDeviceWaitIdle(context->device));
}