#include "vulkan-renderer/swapchain.hpp"
#include "glfw.hpp"
#include "vulkan-renderer/context.hpp"
#include "vulkan-renderer/window.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/debug/panic.hpp"
#include "yacsl/type_aliases.hpp"
#include <vulkan/vulkan_core.h>

using yacsl::u32, yacsl::containers::Vector;
using yacsl::debug::assert_eq, yacsl::debug::panic;

namespace vulkan_renderer {

static VkFormat pick_format(VkPhysicalDevice physical_device,
                            VkSurfaceKHR     surface) noexcept {
  u32 count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count,
                                       nullptr);
  auto formats = Vector<VkSurfaceFormatKHR>(count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count,
                                       formats.data());
  for (auto format : formats) {
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format.format;
    }
  }
  panic("No supported surface format found!");
}

static VkPresentModeKHR pick_present_mode(VkPhysicalDevice physical_device,
                                          VkSurfaceKHR     surface) noexcept {
  u32 count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count,
                                            nullptr);
  auto modes = Vector<VkPresentModeKHR>(count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count,
                                            modes.data());
  for (auto mode : modes) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return mode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

Swapchain Window::create_swapchain(this Window& self,
                                   Context&     context) noexcept {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  assert_eq(glfwCreateWindowSurface(context.instance(), self.handle_,
                                    &context.vk_loader_allocation_callbacks(),
                                    &surface),
            VK_SUCCESS);

  VkSurfaceCapabilitiesKHR capabilities;
  assert_eq(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physical_device(),
                                                      surface, &capabilities),
            VK_SUCCESS);

  VkFormat const format = pick_format(context.physical_device(), surface);
  VkPresentModeKHR const present_mode =
      pick_present_mode(context.physical_device(), surface);

  VkSwapchainCreateInfoKHR info = {
      .sType         = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface       = surface,
      .minImageCount = capabilities.minImageCount,
      .imageFormat   = format,
      .imageExtent =
          {
              .width  = self.framebuffer_size_[0],
              .height = self.framebuffer_size_[1],
          },
      .imageArrayLayers = 1,
      .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform     = capabilities.currentTransform,
      .compositeAlpha   = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
      .presentMode      = present_mode,
  };
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;

  yacsl::debug::assert_eq(
      vkCreateSwapchainKHR(context.device(), &info,
                           &context.vk_loader_allocation_callbacks(),
                           &swapchain),
      VK_SUCCESS);

  return {context, surface, format, swapchain};
}

Swapchain::~Swapchain() noexcept {
  vkDestroySwapchainKHR(context_->device(), swapchain_,
                        &context_->vk_loader_allocation_callbacks());
  vkDestroySurfaceKHR(context_->instance(), surface_,
                      &context_->vk_loader_allocation_callbacks());
}

} // namespace vulkan_renderer
