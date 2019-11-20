#include "vulkan.h"

VkPhysicalDevice getPhysicalDevice(Context* context) {
    uint32_t gpuCount;
    vkEnumeratePhysicalDevices(context->instance, &gpuCount, NULL);
    VkPhysicalDevice gpus[gpuCount];
    vkEnumeratePhysicalDevices(context->instance, &gpuCount, gpus);

    for(int i = 0; i < gpuCount; i++) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(gpus[i], &properties);
        if(properties.apiVersion >= VK_API_VERSION_1_1 && properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            return gpus[i];
        }
    }
    return gpus[0];
}

uint32_t getFamilyIndex(Context* context) {
    uint32_t familyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(context->physicalDevice, &familyCount, NULL);
    VkQueueFamilyProperties families[familyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(context->physicalDevice, &familyCount, families);

    for(int i = 0; i < familyCount; i++) {
        int presentationSupport = glfwGetPhysicalDevicePresentationSupport(context->instance, context->physicalDevice, i);
        if(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentationSupport) {
            return i;
        }
    }
    return 0;
}

VkDevice createDevice(Context* context) {

    float queuePriorities = 1.0f;

    VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueInfo.queueFamilyIndex = context->familyIndex;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &queuePriorities;

    const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueInfo;
    createInfo.enabledExtensionCount = ARRAYSIZE(extensions);
    createInfo.ppEnabledExtensionNames = extensions;

    VkDevice device;
    ASSERT(vkCreateDevice(context->physicalDevice, &createInfo, NULL, &device), "device");
    return device;
}

VkQueue getQueue(Context* context) {
    VkQueue queue;
    vkGetDeviceQueue(context->device, context->familyIndex, 0, &queue);
    return queue;
}