#include "entity.h"

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
    bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    Pipeline pipeline;
    pipeline.setLayout = createSetLayout(context, ARRAYSIZE(bindings), bindings);
    pipeline.descriptorSet = createDescriptorSet(context, pipeline.setLayout);
    pipeline.layout = createPipelineLayout(context, pipeline.setLayout);
    pipeline.pipeline = createGraphicsPipeline(context, vertexShader, fragmentShader, pipeline.layout);
    return pipeline;
}

Entity createEntity(Context* context, const char* modelPath, const char* texturePath, Pipeline pipeline) {
    
    Model model = loadModel(modelPath);
    model.vertexBuffer = createBuffer(context, model.verticesSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    model.indexBuffer = createBuffer(context, model.indicesSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    model.uniformBuffer = createBuffer(context, sizeof(Uniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    uploadBuffer(context, context->stagingBuffer, model.vertexBuffer, model.vertices, model.verticesSize);
    uploadBuffer(context, context->stagingBuffer, model.indexBuffer, model.indices, model.indicesSize);
    Texture texture = createTexture(context, texturePath);
    
    Entity entity;
    entity.model = model;
    entity.texture = texture;
    entity.pipeline = pipeline;
    glm_mat4_identity(entity.modelMatrix);
    return entity;
}

Light createLight(Context* context, vec3 lightPos, vec3 lightColor) {
    Light light;
    light.buffer = createBuffer(context, sizeof(LightObject), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    glm_vec3_copy(lightPos, light.lightObject.lightPos);
    glm_vec3_copy(lightColor, light.lightObject.lightColor);
    uploadBuffer(context, context->stagingBuffer, light.buffer, &light.lightObject, sizeof(LightObject));
    return light;
}

void prepareEntity(Context* context, Entity entity, Light light) {
    Uniform uniform = context->uniform;
    glm_mat4_copy(entity.modelMatrix, uniform.model);

    uploadBuffer(context, context->stagingBuffer, entity.model.uniformBuffer, &uniform, sizeof(Uniform));

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = entity.model.uniformBuffer.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = entity.model.uniformBuffer.size;

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.sampler = entity.texture.sampler;
    imageInfo.imageView = entity.texture.imageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorBufferInfo lightInfo = {};
    lightInfo.buffer = light.buffer.buffer;
    lightInfo.offset = 0;
    lightInfo.range = sizeof(LightObject);

    VkWriteDescriptorSet writes[3] = {};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = entity.pipeline.descriptorSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo = &bufferInfo;
    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = entity.pipeline.descriptorSet;
    writes[1].dstBinding = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo = &imageInfo;
    writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[2].dstSet = entity.pipeline.descriptorSet;
    writes[2].dstBinding = 2;
    writes[2].descriptorCount = 1;
    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writes[2].pBufferInfo = &lightInfo;

    vkUpdateDescriptorSets(context->device, ARRAYSIZE(writes), writes, 0, NULL);
}

void renderEntity(Context* context, Entity entity) {
    vkCmdBindDescriptorSets(context->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, entity.pipeline.layout, 0, 1, &entity.pipeline.descriptorSet, 0, NULL);
    vkCmdBindPipeline(context->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, entity.pipeline.pipeline);
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(context->commandBuffer, 0, 1, &entity.model.vertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(context->commandBuffer, entity.model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(context->commandBuffer, entity.model.indicesCount, 1, 0, 0, 1);
}

Entity rotateEntity(Entity entity, float angle, vec3 axis) {
    glm_rotate(entity.modelMatrix, angle, axis);
    return entity;
}

void destroyLight(Context* context, Light light) {
    destroyBuffer(context, light.buffer);
}

void destroyEntity(Context* context, Entity entity) {
    destroyBuffer(context, entity.model.uniformBuffer);
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