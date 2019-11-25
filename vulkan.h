#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include <cgltf.h>

#include <stb_image.h>

#define ARRAYSIZE(array) (sizeof(array) / sizeof(array[0]))
#define ASSERT(val, pos) if(val != VK_SUCCESS) { printf("Error: %d | %s\n", val, pos); exit(-1); }

typedef struct DepthStencil {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView imageView;
} DepthStencil;

typedef struct Vertex {
    float position[3];
    float normal[3];
    float uv[2];
} Vertex;

typedef struct Buffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    size_t size;
    void* data;
} Buffer;

typedef struct Uniform {
    mat4 projection;
    mat4 view;
    mat4 model;
} Uniform;

typedef struct Texture {
    VkImage image;
    VkDeviceMemory memory;
    VkSampler sampler;
    VkImageView imageView;
} Texture;

typedef struct Model {
    size_t verticesCount;
    Vertex* vertices;
    size_t verticesSize;
    size_t indicesCount;
    uint16_t* indices;
    size_t indicesSize;
} Model;

typedef struct Context {
    int width;
    int height;

    GLFWwindow* window;

    VkInstance instance;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    uint32_t familyIndex;
    VkDevice device;
    VkQueue queue;

    VkFormat format;
    VkFormat depthFormat;
    DepthStencil depthStencil;
    VkSwapchainKHR swapchain;
    VkRenderPass renderPass;

    uint32_t swapchainImageCount;
    VkImage* swapchainImages;
    VkImageView* swapchainImageViews;
    VkFramebuffer* framebuffers;

    VkShaderModule vertexShader;
    VkShaderModule fragmentShader;
    VkDescriptorSetLayout setLayout;
    VkPipelineLayout layout;
    VkPipeline pipeline;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;

    Buffer* buffers;

    VkSemaphore acquireSemaphore;
    VkSemaphore releaseSemaphore;

    Uniform uniform;
    Model model;
    Texture texture;
} Context;