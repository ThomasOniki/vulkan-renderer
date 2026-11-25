#pragma once

#include "yacsl/debug/panic.hpp"
#include "yacsl/threads/atomic.hpp"
#include "yacsl/type_aliases.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h> // IWYU pragma: export

namespace vulkan_renderer::detail {

using yacsl::threads::Atomic, yacsl::threads::MemoryOrder, yacsl::u32;

inline constinit Atomic<u32> glfw_user_count = 0;

inline void glfw_retain() {
  if (glfw_user_count.fetch_add(1, MemoryOrder::AcqRel) == 0) {
    yacsl::debug::assert_(glfwInit() == GLFW_TRUE,
                          "glfwInit returned GLFW_FALSE!");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwSetErrorCallback([](int code, char const* desc) {
      yacsl::debug::panic("[GLFW] {}: {}", code, desc);
    });
  }
}
inline void glfw_release() {
  if (glfw_user_count.sub_fetch(1, MemoryOrder::AcqRel) == 0) {
    glfwTerminate();
  }
}

} // namespace vulkan_renderer::detail
