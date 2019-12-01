#include "vulkan.h"

VkShaderModule loadShader(Context* context, const char* path);
VkDescriptorSetLayout createSetLayout(Context* context, uint32_t bindingCount, VkDescriptorSetLayoutBinding* bindings);
VkPipelineLayout createPipelineLayout(Context* context, VkDescriptorSetLayout setLayout);
VkPipeline createGraphicsPipeline(Context* context, VkShaderModule vertexShader, VkShaderModule fragmentShader, VkPipelineLayout layout);