#include "vulkan.h"

VkPhysicalDevice getPhysicalDevice(Context* context);
uint32_t getFamilyIndex(Context* context);
VkDevice createDevice(Context* context);
VkQueue getQueue(Context* context);