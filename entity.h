#include "vulkan.h"
#include "modelloader.h"
#include "vk/pipeline.h"
#include "vk/texture.h"

Pipeline createPipeline(Context* context, const char* vertexShaderPath, const char* fragmentShaderPath);
Entity createEntity(Context* context, const char* modelPath, const char* texturePath, Pipeline pipeline);
void prepareEntity(Context* context, Entity entity);
void renderEntity(Context* context, Entity entity);
Entity rotateEntity(Entity entity, float angle, vec3 axis);
void destroyEntity(Context* context, Entity entity);
void destroyPipeline(Context* context, Pipeline pipeline);