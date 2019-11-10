#include "triangle.h"

VkInstance createInstance() {
    VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.apiVersion = VK_API_VERSION_1_1;

    uint32_t extensionCount;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
    
    VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    createInfo.pApplicationInfo = &appInfo;
#ifdef DEBUG
    printf("DEBUG Mode enabled\n");
    createInfo.enabledLayerCount = ARRAYSIZE(layers);
    createInfo.ppEnabledLayerNames = layers;
#endif
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;

    VkInstance instance;
    ASSERT(vkCreateInstance(&createInfo, NULL, &instance), "instance");
    return instance;
}

VkSurfaceKHR createSurface(Context* context) {
    VkSurfaceKHR surface;
    glfwCreateWindowSurface(context->instance, context->window, NULL, &surface);
    return surface;
}