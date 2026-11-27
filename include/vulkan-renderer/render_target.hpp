#pragma once

#include "yacsl/math/matrix.hpp"
#include "yacsl/type_aliases.hpp"
#include <vulkan/vulkan_core.h>

namespace vulkan_renderer {

// Non owning
struct RenderTarget {
  VkImage                       image;
  VkImageView                   image_view;
  VkFramebuffer                 framebuffer;
  VkRenderPass                  render_pass;
  VkSwapchainKHR                swapchain;
  yacsl::math::Vec2<yacsl::u32> size;
};

} // namespace vulkan_renderer
