#include "vulkan.h"
#include "buffer.h"

void uploadTexture(Context* context, VkImage image, int x, int y, Buffer staging);
Texture createTexture(Context* context, const char* path);
void destroyTexture(Context* context, Texture texture);