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

typedef struct Pipeline {
    VkDescriptorSetLayout setLayout;
    VkDescriptorSet descriptorSet;
    VkPipelineLayout layout;
    VkPipeline pipeline;
} Pipeline;

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
    Buffer vertexBuffer;

    size_t indicesCount;
    uint16_t* indices;
    size_t indicesSize;
    Buffer indexBuffer;

    Buffer uniformBuffer;
} Model;

typedef struct Entity {
    Model model;
    Texture texture;
    Pipeline pipeline;
    mat4 modelMatrix;
} Entity;

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

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkDescriptorPool descriptorPool;

    Buffer stagingBuffer;

    VkSemaphore acquireSemaphore;
    VkSemaphore releaseSemaphore;

    Uniform uniform;
} Context;

// User functions
void initialize(Context* context);
void gameLoop(Context* context);
void renderLoop(Context* context);
void cleanUp(Context* context);