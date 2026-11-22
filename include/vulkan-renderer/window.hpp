#pragma once

#include "vulkan-renderer/compiler/export.hpp"
#include "vulkan-renderer/swapchain.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/math/matrix.hpp"
#include "yacsl/type_aliases.hpp"

using GLFWwindow = struct GLFWwindow;

namespace vulkan_renderer {

class Context;

class Window {
public:
  static Window open(yacsl::math::Vec2<yacsl::u32> size,
                     char const* title) noexcept VULKAN_RENDERER_API;

  static Window open(yacsl::math::Vec2<yacsl::u32> size,
                     yacsl::containers::String     title) noexcept {
    return open(size, title.c_str());
  }

  Swapchain create_swapchain(this Window& self,
                             Context&     context) noexcept VULKAN_RENDERER_API;

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
  Window(GLFWwindow*                   handle,
         yacsl::math::Vec2<yacsl::u32> framebuffer_size) noexcept
      : handle_(handle), framebuffer_size_(framebuffer_size) {}

  GLFWwindow*                   handle_ = nullptr;
  yacsl::math::Vec2<yacsl::u32> framebuffer_size_;
};

} // namespace vulkan_renderer
