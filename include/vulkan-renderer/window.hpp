#pragma once

#include "vulkan-renderer/compiler/export.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/math/matrix.hpp"
#include "yacsl/type_aliases.hpp"
#include <vulkan/vulkan_core.h>

using GLFWwindow = struct GLFWwindow;

namespace vulkan_renderer {

class Context;

class Window {
public:
  explicit Window(yacsl::math::Vec2<yacsl::u32> size,
                  char const* title) noexcept VULKAN_RENDERER_API;

  explicit Window(yacsl::math::Vec2<yacsl::u32> size,
                  yacsl::containers::String     title)
      : Window(size, title.c_str()) {}

  VkSurfaceKHR create_surface(this Window& self,
                              Context& context) noexcept VULKAN_RENDERER_API;

  ~Window() noexcept VULKAN_RENDERER_API;

  Window(Window const&)            = delete;
  Window(Window&& other)           = delete;
  Window& operator=(Window const&) = delete;
  Window& operator=(Window&&)      = delete;

  [[nodiscard]] bool
  should_close(this Window const& self) noexcept VULKAN_RENDERER_API;

  void update(this Window const& self) noexcept VULKAN_RENDERER_API;

  [[nodiscard]] constexpr auto handle(this Window const& self) noexcept {
    return self.handle_;
  }
  [[nodiscard]] constexpr auto
  framebuffer_size(this Window const& self) noexcept {
    return self.framebuffer_size_;
  }

private:
  GLFWwindow*                   handle_           = nullptr;
  yacsl::math::Vec2<yacsl::u32> framebuffer_size_ = {0, 0};
};

} // namespace vulkan_renderer
