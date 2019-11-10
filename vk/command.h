#include "triangle.h"

VkCommandPool createCommandPool(Context* context) {
    VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = context->familyIndex;

    VkCommandPool commandPool;
    ASSERT(vkCreateCommandPool(context->device, &createInfo, NULL, &commandPool), "commandPool");
    return commandPool;
}

VkCommandBuffer createCommandBuffer(Context* context) {
    VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = context->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    ASSERT(vkAllocateCommandBuffers(context->device, &allocInfo, &commandBuffer), "commandBuffer");
    return commandBuffer;
}

VkDescriptorPool createDescriptorPool(Context* context, VkDescriptorPoolSize* poolSizes) {
    VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    createInfo.maxSets = 128;
    createInfo.poolSizeCount = ARRAYSIZE(poolSizes);
    createInfo.pPoolSizes = poolSizes;

    VkDescriptorPool descriptorPool;
    ASSERT(vkCreateDescriptorPool(context->device, &createInfo, NULL, &descriptorPool), "descriptorPool");
    return descriptorPool;
}

VkDescriptorSet createDescriptorSet(Context* context) {
    VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.descriptorPool = context->descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &context->setLayout;

    VkDescriptorSet descriptorSet;
    ASSERT(vkAllocateDescriptorSets(context->device, NULL, &descriptorSet), "descriptorSet");
    return descriptorSet;
}

VkSemaphore createSemaphore(Context* context) {
    VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

    VkSemaphore semaphore;
    ASSERT(vkCreateSemaphore(context->device, &createInfo, NULL, &semaphore), "semaphore");
    return semaphore;
}