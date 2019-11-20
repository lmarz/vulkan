#pragma once
#include "vulkan.h"

Buffer createBuffer(Context* context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags) {
    VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    createInfo.size = size;
    createInfo.usage = usage;

    VkBuffer buffer;
    ASSERT(vkCreateBuffer(context->device, &createInfo, NULL, &buffer), "buffer");

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(context->device, buffer, &requirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(context->physicalDevice, &memProperties);
    uint32_t memoryTypeIndex = selectMemoryType(memProperties, requirements.memoryTypeBits, flags);

    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    VkDeviceMemory memory;
    ASSERT(vkAllocateMemory(context->device, &allocInfo, NULL, &memory), "allocateMemory");
    ASSERT(vkBindBufferMemory(context->device, buffer, memory, 0), "bindMemory");

    void* data;
    if(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        ASSERT(vkMapMemory(context->device, memory, 0, size, 0, &data), "mapMem");

    Buffer buf;
    buf.buffer = buffer;
    buf.memory = memory;
    buf.size = size;
    buf.data = data;

    return buf;
}

void uploadBuffer(Context* context, Buffer scratch, Buffer buf, void* data, size_t size) {
    memcpy(scratch.data, data, size);

    ASSERT(vkResetCommandPool(context->device, context->commandPool, 0), "reset");

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    ASSERT(vkBeginCommandBuffer(context->commandBuffer, &beginInfo), "begin");

    VkBufferCopy region = {0, 0, size};
    vkCmdCopyBuffer(context->commandBuffer, scratch.buffer, buf.buffer, 1, &region);

    ASSERT(vkEndCommandBuffer(context->commandBuffer), "end");

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &context->commandBuffer;

    ASSERT(vkQueueSubmit(context->queue, 1, &submitInfo, VK_NULL_HANDLE), "submit");

    ASSERT(vkDeviceWaitIdle(context->device), "wait");
}

void destroyBuffer(Context* context, Buffer buffer) {
    vkDestroyBuffer(context->device, buffer.buffer, NULL);
    vkFreeMemory(context->device, buffer.memory, NULL);
}