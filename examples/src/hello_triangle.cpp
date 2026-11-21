#include "vulkan-renderer/window.hpp"
#include "yacsl/debug/log.hpp"

int main() {
  auto window = vulkan_renderer::Window::open({800, 400}, "hello triangle");
  while (!window.should_close()) {
    window.update();
  }
  yacsl::debug::flush_logger();
}
