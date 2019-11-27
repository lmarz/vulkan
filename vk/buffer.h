#include "vulkan.h"
#include "command.h"

uint32_t selectMemoryType(VkPhysicalDeviceMemoryProperties properties, uint32_t memoryTypeBits, VkMemoryPropertyFlags flags);
Buffer createBuffer(Context* context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);
void uploadBuffer(Context* context, Buffer scratch, Buffer buf, void* data, size_t size);
void destroyBuffer(Context* context, Buffer buffer);