#define GLFW_INCLUDE_NONE
#include "vulkan-renderer/dummy.hpp"
#include <GLFW/glfw3.h>
#include <assert.h>
#include <vulkan/vulkan_core.h>
#include <yacsl/debug/assert.hpp>
#include <yacsl/debug/log.hpp>

void dummy() {
  yacsl::debug::log_info("vkCreateInstance: {}",
                         reinterpret_cast<void*>(vkCreateInstance));
  yacsl::debug::log_info("glfwCreateWindow: {}",
                         reinterpret_cast<void*>(glfwCreateWindow));
}
