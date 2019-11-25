#include "vulkan.h"
#include "buffer.h"

VkFormat getFormat(Context* context);
VkFormat getDepthFormat(Context* context);
VkSwapchainKHR createSwapchain(Context* context);
DepthStencil createDepthStencil(Context* context);
VkRenderPass createRenderPass(Context* context);
uint32_t getSwapchainImageCount(Context* context);
VkImage* getSwapchainImages(Context* context);
VkImageView* createSwapchainImageViews(Context* context);
VkFramebuffer* createFramebuffers(Context* context);
void destroyDepthStencil(Context* context);
void destroySwapchainStuff(Context* context);