#include "vulkan.h"
#include "modelloader.h"
#include "vk/texture.h"

typedef struct Entity {
    Model model;
    Texture texture;
    Pipeline pipeline;
} Entity;

Pipeline createPipeline(Context* context, const char* vertexShaderPath, const char* fragmentShaderPath) {
    VkShaderModule vertexShader = loadShader(context, vertexShaderPath);
    VkShaderModule fragmentShader = loadShader(context, fragmentShaderPath);

    VkDescriptorSetLayoutBinding bindings[3] = {};
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    Pipeline pipeline;
    pipeline.setLayout = createSetLayout(context, ARRAYSIZE(bindings), bindings);
    pipeline.layout = createPipelineLayout(context);
    pipeline.pipeline = createGraphicsPipeline(context);
    return pipeline;
}

Entity createEntity(Context* context, const char* modelPath, const char* texturePath, Pipeline pipeline) {
    
    Model model = loadModel(modelPath);
    model.vertexBuffer = createBuffer(context, model.verticesSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    model.indexBuffer = createBuffer(context, model.indicesSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    model.uniformBuffer = createBuffer(context, sizeof(Uniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    uploadBuffer(context, context->buffers[0], model.vertexBuffer, model.vertices, model.verticesSize);
    uploadBuffer(context, context->buffers[0], model.indexBuffer, model.indices, model.indicesSize);

    Texture texture = createTexture(context, texturePath);
    
    Entity entity;
    entity.model = model;
    entity.texture = texture;
    entity.pipeline = pipeline;
    return entity;
}

void prepareEntity(Context* context, Entity entity) {
    Uniform uniform = context->uniform;
    glm_mat4_identity(uniform.model);
    uploadBuffer(context, context->buffers[0], entity.model.uniformBuffer, &uniform, sizeof(Uniform));

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = entity.model.uniformBuffer.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = entity.model.uniformBuffer.size;

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.sampler = entity.texture.sampler;
    imageInfo.imageView = entity.texture.imageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writes[2] = {};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = context->descriptorSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo = &bufferInfo;
    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = context->descriptorSet;
    writes[1].dstBinding = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(context->device, ARRAYSIZE(writes), writes, 0, NULL);
}

void renderEntity(Context* context, Entity entity) {
    vkCmdBindPipeline(context->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, entity.pipeline.pipeline);
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(context->commandBuffer, 0, 1, &entity.model.vertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(context->commandBuffer, entity.model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(context->commandBuffer, entity.model.indicesCount, 1, 0, 0, 1);
}

void destroyEntity(Context* context, Entity entity) {
    destroyBuffer(context, entity.model.indexBuffer);
    destroyBuffer(context, entity.model.vertexBuffer);
    destroyModel(entity.model);
    destroyTexture(context, entity.texture);
}

void destroyPipeline(Context* context, Pipeline pipeline) {
    vkDestroyPipeline(context->device, pipeline.pipeline, NULL);
    vkDestroyPipelineLayout(context->device, pipeline.layout, NULL);
    vkDestroyDescriptorSetLayout(context->device, pipeline.setLayout, NULL);
}