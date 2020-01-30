#include "vk/renderer.h"

#include "entity.h"

Pipeline pipeline;
Entity entity;

void initFunc(Context* context) {
    pipeline = createPipeline(context, "res/shaders/vert.spv", "res/shaders/frag.spv");
    entity = createEntity(context, "res/models/cube.gltf", "res/textures/cube.png", pipeline);
}

void gameLoopFunc(Context* context) {
    entity = rotateEntity(entity, 0.05f, (vec3){0, 1, 0});
    prepareEntity(context, entity);
}

void renderLoopFunc(Context* context) {
    renderEntity(context, entity);
}

void cleanFunc(Context* context) {
    destroyEntity(context, entity);
    destroyPipeline(context, pipeline);
}

int main(int argc, char const *argv[]) {
    Context* context = malloc(sizeof(Context));
    context->width = 800;
    context->height = 600;

    init(context, initFunc);
    mainLoop(context, gameLoopFunc, renderLoopFunc);
    clean(context, cleanFunc);

    free(context);
    return 0;
}