#include "vulkan.h"

VkCommandPool createCommandPool(Context* context);
VkCommandBuffer createCommandBuffer(Context* context);
VkDescriptorPool createDescriptorPool(Context* context, VkDescriptorPoolSize* poolSizes);
VkDescriptorSet createDescriptorSet(Context* context);
VkSemaphore createSemaphore(Context* context);
void startRecording(Context* context);
void endRecording(Context* context);