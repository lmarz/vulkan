#include "swapchain.h"

VkFormat getFormat(Context* context) {
    uint32_t formatCount;
    ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(context->physicalDevice, context->surface, &formatCount, NULL), "formats");
    VkSurfaceFormatKHR formats[formatCount];
    ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(context->physicalDevice, context->surface, &formatCount, formats), "formats");

    if(formats[0].format == VK_FORMAT_UNDEFINED) {
        return VK_FORMAT_B8G8R8_UNORM;
    } else {
        return formats[0].format;
    }
}

VkFormat getDepthFormat(Context* context) {
    VkFormat formats[] = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };

    for(int i = 0; i < ARRAYSIZE(formats); i++) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(context->physicalDevice, formats[i], &properties);
        if(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return formats[i];
        }
    }
    return 0;
}

VkSwapchainKHR createSwapchain(Context* context) {
    VkBool32 supported;
    ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(context->physicalDevice, context->familyIndex, context->surface, &supported), "supported");

    VkSurfaceCapabilitiesKHR capabilities;
    ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physicalDevice, context->surface, &capabilities), "capabilities");

    VkCompositeAlphaFlagBitsKHR surfaceComposite = (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR :\
                                                   (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) ? VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR :\
                                                   (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) ? VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR :\
                                                   VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

    VkExtent2D extent = { context->width, context->height };

    VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    createInfo.surface = context->surface;
    createInfo.minImageCount = capabilities.minImageCount;
    createInfo.imageFormat = context->format;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 1;
    createInfo.pQueueFamilyIndices = &context->familyIndex;
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = surfaceComposite;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_FALSE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain;
    ASSERT(vkCreateSwapchainKHR(context->device, &createInfo, NULL, &swapchain), "swapchain");
    return swapchain;
}

DepthStencil createDepthStencil(Context* context) {
    VkExtent3D extent = { context->width, context->height, 1 };

    VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = context->depthFormat;
    imageInfo.extent = extent;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image;
    ASSERT(vkCreateImage(context->device, &imageInfo, NULL, &image), "depthImage");

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(context->device, image, &requirements);

    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(context->physicalDevice, &properties);

    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = selectMemoryType(properties, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory memory;
    ASSERT(vkAllocateMemory(context->device, &allocInfo, NULL, &memory), "depthMemory");
    ASSERT(vkBindImageMemory(context->device, image, memory, 0), "depthBind");

    VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = context->depthFormat;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    ASSERT(vkCreateImageView(context->device, &createInfo, NULL, &imageView), "depthView");

    DepthStencil depthStencil;
    depthStencil.image = image;
    depthStencil.memory = memory;
    depthStencil.imageView = imageView;

    return depthStencil;
}

VkRenderPass createRenderPass(Context* context) {
    VkAttachmentDescription attachments[2] = {};
    attachments[0].format = context->format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[1].format = context->depthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachments[2] = {};
    colorAttachments[0].attachment = 0;
    colorAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachments[1].attachment = 1;
    colorAttachments[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachments[0];
    subpass.pDepthStencilAttachment = &colorAttachments[1];

    VkSubpassDependency dependencies[2];
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    createInfo.attachmentCount = ARRAYSIZE(attachments);
    createInfo.pAttachments = attachments;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    createInfo.dependencyCount = ARRAYSIZE(dependencies);
    createInfo.pDependencies = dependencies;

    VkRenderPass renderPass;
    ASSERT(vkCreateRenderPass(context->device, &createInfo, NULL, &renderPass), "renderPass");
    return renderPass;
}

uint32_t getSwapchainImageCount(Context* context) {
    uint32_t imageCount;
    ASSERT(vkGetSwapchainImagesKHR(context->device, context->swapchain, &imageCount, NULL), "imageCount");
    return imageCount;
}

VkImage* getSwapchainImages(Context* context) {
    VkImage* images = malloc(sizeof(VkImage) * context->swapchainImageCount);
    ASSERT(vkGetSwapchainImagesKHR(context->device, context->swapchain, &context->swapchainImageCount, images), "swapcainImages");
    return images;
}

VkImageView* createSwapchainImageViews(Context* context) {
    VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = context->format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.layerCount = 1;
    createInfo.subresourceRange.levelCount = 1;

    VkImageView* imageViews = malloc(sizeof(VkImageView) * context->swapchainImageCount);

    for(int i = 0; i < context->swapchainImageCount; i++) {
        createInfo.image = context->swapchainImages[i];
        ASSERT(vkCreateImageView(context->device, &createInfo, NULL, &imageViews[i]), "swapchainImageViews");
    }
    return imageViews;
}

VkFramebuffer* createFramebuffers(Context* context) {

    VkImageView attachments[2];
    attachments[1] = context->depthStencil.imageView;

    VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    createInfo.renderPass = context->renderPass;
    createInfo.attachmentCount = 2;
    createInfo.pAttachments = attachments;
    createInfo.width = context->width;
    createInfo.height = context->height;
    createInfo.layers = 1;

    VkFramebuffer* framebuffers = malloc(sizeof(VkFramebuffer) * context->swapchainImageCount);

    for(int i = 0; i < context->swapchainImageCount; i++) {
        attachments[0] = context->swapchainImageViews[i];
        // Maybe? createInfo.pAttachments = attachments;
        ASSERT(vkCreateFramebuffer(context->device, &createInfo, NULL, &framebuffers[i]), "framebuffers");
    }
    return framebuffers;
}

void destroyDepthStencil(Context* context) {
    vkDestroyImageView(context->device, context->depthStencil.imageView, NULL);
    vkFreeMemory(context->device, context->depthStencil.memory, NULL);
    vkDestroyImage(context->device, context->depthStencil.image, NULL);
}

void destroySwapchainStuff(Context* context) {

    for(int i = 0; i < context->swapchainImageCount; i++) {
        vkDestroyFramebuffer(context->device, context->framebuffers[i], NULL);
        vkDestroyImageView(context->device, context->swapchainImageViews[i], NULL);
    }

    free(context->framebuffers);
    free(context->swapchainImageViews);
    free(context->swapchainImages);
}