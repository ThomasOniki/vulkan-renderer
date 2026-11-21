#pragma once

#include "vulkan-renderer/compiler/export.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/math/matrix.hpp"
#include "yacsl/type_aliases.hpp"

using GLFWwindow = struct GLFWwindow;

namespace vulkan_renderer {

class Window {
public:
  static Window open(yacsl::math::Vec2<yacsl::u32> size,
                     char const* title) noexcept VULKAN_RENDERER_API;

  static Window open(yacsl::math::Vec2<yacsl::u32> size,
                     yacsl::containers::String     title) noexcept {
    return open(size, title.c_str());
  }

  ~Window() noexcept VULKAN_RENDERER_API;

  Window(GLFWwindow* handle) noexcept : handle_(handle) {}
  Window(Window const&)            = delete;
  Window(Window&& other)           = delete;
  Window& operator=(Window const&) = delete;
  Window& operator=(Window&&)      = delete;

  [[nodiscard]] bool should_close(this Window const& self) noexcept;

  void update(this Window const& self) noexcept;

  [[nodiscard]] constexpr auto handle(this Window const& self) noexcept {
    return self.handle_;
  }

private:
  GLFWwindow* handle_ = nullptr;
};

} // namespace vulkan_renderer
