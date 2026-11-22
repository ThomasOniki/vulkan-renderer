#include "vulkan-renderer/window.hpp"
#include "glfw.hpp"
#include "yacsl/math/matrix.hpp"

namespace vulkan_renderer {

using yacsl::u32, yacsl::math::Vec2;

Window Window::open(Vec2<u32> size, char const* title) noexcept {
  detail::glfw_retain();

  GLFWwindow* handle =
      glfwCreateWindow(static_cast<int>(size[0]), static_cast<int>(size[0]),
                       title, nullptr, nullptr);

  yacsl::debug::assert_(handle != nullptr, "glfwCreateWindow returned NULL!");

  int width  = 0;
  int height = 0;
  glfwGetFramebufferSize(handle, &width, &height);

  return {
      handle,
      {static_cast<u32>(width), static_cast<u32>(height)},
  };
}

Window::~Window() noexcept {
  glfwDestroyWindow(handle_);
  detail::glfw_release();
}

bool Window::should_close(this Window const& self) noexcept {
  return glfwWindowShouldClose(self.handle_) == GLFW_TRUE;
}

void Window::update(this Window const& /*self*/) noexcept { glfwPollEvents(); }

} // namespace vulkan_renderer
