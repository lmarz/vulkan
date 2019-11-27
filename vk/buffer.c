#include "buffer.h"

uint32_t selectMemoryType(VkPhysicalDeviceMemoryProperties properties, uint32_t memoryTypeBits, VkMemoryPropertyFlags flags) {
    for(uint32_t i = 0; i < properties.memoryTypeCount; i++) {
        if((memoryTypeBits & (1 << i)) != 0 && (properties.memoryTypes[i].propertyFlags & flags) == flags) {
            return i;
        }
    }
    return ~0u;
}

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

    startRecording(context);

    VkBufferCopy region = {0, 0, size};
    vkCmdCopyBuffer(context->commandBuffer, scratch.buffer, buf.buffer, 1, &region);

    endRecording(context);
}

void destroyBuffer(Context* context, Buffer buffer) {
    vkDestroyBuffer(context->device, buffer.buffer, NULL);
    vkFreeMemory(context->device, buffer.memory, NULL);
}