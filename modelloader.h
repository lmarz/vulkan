#include "vulkan.h"

#include <cgltf.h>

Model loadModel(const char* path);
void destroyModel(Model model);