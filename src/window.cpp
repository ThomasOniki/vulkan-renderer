#include "vulkan-renderer/window.hpp"
#include "yacsl/math/matrix.hpp"
#include "yacsl/threads/atomic.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace vulkan_renderer {

using yacsl::u32, yacsl::threads::Atomic, yacsl::threads::MemoryOrder,
    yacsl::math::Vec2;

static Atomic<u32> window_count = 0;

Window Window::open(Vec2<u32> size, char const* title) noexcept {
  if (window_count.fetch_add(1, MemoryOrder::AcqRel) == 0) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    yacsl::debug::assert_(glfwInit() == GLFW_TRUE,
                          "glfwInit returned GLFW_FALSE!");
  }

  GLFWwindow* handle =
      glfwCreateWindow(static_cast<int>(size[0]), static_cast<int>(size[0]),
                       title, nullptr, nullptr);

  yacsl::debug::assert_(handle != nullptr, "glfwCreateWindow returned NULL!");

  return {handle};
}

Window::~Window() noexcept {
  glfwDestroyWindow(handle_);
  if (window_count.sub_fetch(1, MemoryOrder::AcqRel) == 0) {
    glfwTerminate();
  }
}

bool Window::should_close(this Window const& self) noexcept {
  return glfwWindowShouldClose(self.handle_) == GLFW_TRUE;
}

void Window::update(this Window const& /*self*/) noexcept { glfwPollEvents(); }

} // namespace vulkan_renderer
