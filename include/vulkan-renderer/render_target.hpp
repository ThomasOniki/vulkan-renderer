#pragma once

#include <vulkan/vulkan_core.h>

namespace vulkan_renderer {

// Non owning
struct RenderTarget {
  VkImage     image;
  VkImageView image_view;
  VkSemaphore ready_semaphore;
};

} // namespace vulkan_renderer
