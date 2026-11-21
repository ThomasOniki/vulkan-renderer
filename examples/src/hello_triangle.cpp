#include "vulkan-renderer/context.hpp"
#include "vulkan-renderer/window.hpp"
#include "yacsl/debug/log.hpp"

using vulkan_renderer::Context, vulkan_renderer::Window;

int main() {
  auto context = Context::create();

  auto window = Window::open({800, 400}, "hello triangle");
  while (!window.should_close()) {
    window.update();
  }
  yacsl::debug::flush_logger();
}
