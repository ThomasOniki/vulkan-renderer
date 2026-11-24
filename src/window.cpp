#include "vulkan-renderer/window.hpp"
#include "glfw.hpp"
#include "vulkan-renderer/context.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/math/matrix.hpp"

using yacsl::u32, yacsl::math::Vec2, yacsl::debug::assert_eq;

namespace vulkan_renderer {

Window::Window(Vec2<u32> size, char const* title) noexcept {
  detail::glfw_retain();

  {
    handle_ =
        glfwCreateWindow(static_cast<int>(size[0]), static_cast<int>(size[0]),
                         title, nullptr, nullptr);

    yacsl::debug::assert_(handle_ != nullptr,
                          "glfwCreateWindow returned NULL!");
  }

  int width  = 0;
  int height = 0;
  glfwGetFramebufferSize(handle_, &width, &height);
  framebuffer_size_ = {static_cast<u32>(width), static_cast<u32>(height)};
}

Window::~Window() noexcept {
  glfwDestroyWindow(handle_);
  detail::glfw_release();
}

bool Window::should_close(this Window const& self) noexcept {
  return glfwWindowShouldClose(self.handle_) == GLFW_TRUE;
}

void Window::update(this Window const& /*self*/) noexcept { glfwPollEvents(); }

VkSurfaceKHR Window::create_surface(this Window& self,
                                    Context&     context) noexcept {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  assert_eq(glfwCreateWindowSurface(context.instance(), self.handle_,
                                    &context.vk_loader_allocation_callbacks(),
                                    &surface),
            VK_SUCCESS);
  return surface;
}

} // namespace vulkan_renderer
