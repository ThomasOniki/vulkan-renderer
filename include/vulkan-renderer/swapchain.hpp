#pragma once

#include "vulkan-renderer/compiler/export.hpp"
#include "vulkan-renderer/context.hpp"
#include "vulkan-renderer/render_target.hpp"
#include "vulkan-renderer/window.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/containers/tuple.hpp"
#include "yacsl/math/matrix.hpp"
#include "yacsl/type_aliases.hpp"
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

  [[nodiscard]] yacsl::containers::Tuple<RenderTarget, VkSemaphore, yacsl::u32>
  next_image(this Swapchain& self) noexcept VULKAN_RENDERER_API;

  [[nodiscard]] auto surface(this Swapchain const& self) noexcept {
    return self.surface_;
  }
  [[nodiscard]] auto format(this Swapchain const& self) noexcept {
    return self.format_;
  }
  [[nodiscard]] auto swapchain(this Swapchain const& self) noexcept {
    return self.swapchain_;
  }
  [[nodiscard]] auto render_pass(this Swapchain const& self) noexcept {
    return self.render_pass_;
  }

  static constexpr yacsl::usize FRAMES_IN_FLIGHT = 2;

private:
  Context*       context_;
  VkSurfaceKHR   surface_;
  VkFormat       format_;
  VkSwapchainKHR swapchain_   = VK_NULL_HANDLE;
  VkRenderPass   render_pass_ = VK_NULL_HANDLE;

  yacsl::math::Vec2<yacsl::u32> fb_size_;

  yacsl::containers::Vector<VkImage, 4>       images_;
  yacsl::containers::Vector<VkImageView, 4>   image_views_;
  yacsl::containers::Vector<VkFramebuffer, 4> framebuffers_;

  yacsl::u32 frame_index_ = 0;
  yacsl::containers::StackArray<VkSemaphore, FRAMES_IN_FLIGHT>
      image_acquired_semaphores_ = {};
};

} // namespace vulkan_renderer
