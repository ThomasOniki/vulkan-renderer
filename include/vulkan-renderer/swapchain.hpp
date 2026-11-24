#pragma once

#include "vulkan-renderer/compiler/export.hpp"
#include "vulkan-renderer/context.hpp"
#include "vulkan-renderer/window.hpp"
#include <vulkan/vulkan_core.h>

namespace vulkan_renderer {

class Swapchain {
public:
  explicit Swapchain(Context& context,
                     Window&  window) noexcept VULKAN_RENDERER_API;
  ~Swapchain() noexcept VULKAN_RENDERER_API;

  Swapchain(Swapchain const&)                                  = delete;
  Swapchain(Swapchain&&)                                       = delete;
  Swapchain& operator=(this Swapchain& self, Swapchain const&) = delete;
  Swapchain& operator=(this Swapchain& self, Swapchain&&)      = delete;

  [[nodiscard]] auto surface(this Swapchain const& self) noexcept {
    return self.surface_;
  }
  [[nodiscard]] auto format(this Swapchain const& self) noexcept {
    return self.format_;
  }
  [[nodiscard]] auto swapchain(this Swapchain const& self) noexcept {
    return self.swapchain_;
  }

private:
  Context*       context_;
  VkSurfaceKHR   surface_;
  VkFormat       format_;
  VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
};

} // namespace vulkan_renderer
