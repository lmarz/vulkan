#include "vulkan.h"

#include "vk/device.h"
#include "vk/swapchain.h"
#include "vk/pipeline.h"
#include "vk/command.h"
#include "vk/buffer.h"
#include "vk/texture.h"

void init(Context* context, void (*initFunc)(Context*));
void mainLoop(Context* context, void (*gameLoopFunc)(Context*), void (*renderLoopFunc)(Context*));
void clean(Context* context, void (*cleanFunc)(Context*));