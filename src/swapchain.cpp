#include "vulkan-renderer/swapchain.hpp"
#include "vulkan-renderer/context.hpp"
#include "vulkan-renderer/render_target.hpp"
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

static VkRenderPass create_render_pass(Context& context,
                                       VkFormat format) noexcept {
  VkSubpassDescription    subpass    = {};
  VkAttachmentDescription attachment = {
      .format         = format,
      .samples        = VK_SAMPLE_COUNT_1_BIT,
      .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };
  VkRenderPassCreateInfo info = {
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments    = &attachment,
      .subpassCount    = 1,
      .pSubpasses      = &subpass,
  };
  VkRenderPass render_pass = VK_NULL_HANDLE;
  assert_eq(vkCreateRenderPass(context.device(), &info,
                               &context.vk_cpu_alloc_callbacks(), &render_pass),
            VK_SUCCESS);
  return render_pass;
}

static VkImageView create_image_view(Context& context, VkImage image,
                                     VkFormat format) noexcept {
  VkImageViewCreateInfo info = {
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image    = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format   = format,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .levelCount = 1,
              .layerCount = 1,
          },
  };
  VkImageView image_view = VK_NULL_HANDLE;
  assert_eq(vkCreateImageView(context.device(), &info,
                              &context.vk_cpu_alloc_callbacks(), &image_view),
            VK_SUCCESS);
  return image_view;
}

static VkSemaphore create_semaphore(Context& context) noexcept {
  VkSemaphoreCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VkSemaphore semaphore = VK_NULL_HANDLE;
  assert_eq(vkCreateSemaphore(context.device(), &info,
                              &context.vk_cpu_alloc_callbacks(), &semaphore),
            VK_SUCCESS);
  return semaphore;
}

Swapchain::Swapchain(Context& context, Window& window) noexcept
    : context_(&context), surface_(window.create_surface(context)),
      format_(pick_format(context.physical_device(), surface_)) {
  VkSurfaceCapabilitiesKHR capabilities;
  assert_eq(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physical_device(),
                                                      surface_, &capabilities),
            VK_SUCCESS);

  VkFormat const format = pick_format(context.physical_device(), surface_);
  VkPresentModeKHR const present_mode =
      pick_present_mode(context.physical_device(), surface_);

  VkSwapchainCreateInfoKHR info = {
      .sType         = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface       = surface_,
      .minImageCount = capabilities.minImageCount,
      .imageFormat   = format,
      .imageExtent =
          {
              .width  = window.framebuffer_size()[0],
              .height = window.framebuffer_size()[1],
          },
      .imageArrayLayers = 1,
      .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform     = capabilities.currentTransform,
      .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode      = present_mode,
  };
  yacsl::debug::assert_eq(
      vkCreateSwapchainKHR(context.device(), &info,
                           &context.vk_cpu_alloc_callbacks(), &swapchain_),
      VK_SUCCESS);

  render_pass_ = create_render_pass(context, format);

  u32 image_count = 0;
  assert_eq(vkGetSwapchainImagesKHR(context.device(), swapchain_, &image_count,
                                    nullptr),
            VK_SUCCESS);
  images_.resize(image_count);
  assert_eq(vkGetSwapchainImagesKHR(context.device(), swapchain_, &image_count,
                                    images_.data()),
            VK_SUCCESS);
  for (VkImage const image : images_) {
    image_views_.push(create_image_view(context, image, format_));
  }

  for (VkSemaphore& semaphore : image_acquired_semaphores_) {
    semaphore = create_semaphore(context);
  }
}

Swapchain::~Swapchain() noexcept {
  for (VkSemaphore semaphore : image_acquired_semaphores_) {
    vkDestroySemaphore(context_->device(), semaphore,
                       &context_->vk_cpu_alloc_callbacks());
  }

  for (yacsl::u32 i = 0; i < images_.len(); ++i) {
    vkDestroyImageView(context_->device(), image_views_[i],
                       &context_->vk_cpu_alloc_callbacks());
  }

  vkDestroyRenderPass(context_->device(), render_pass_,
                      &context_->vk_cpu_alloc_callbacks());

  vkDestroySwapchainKHR(context_->device(), swapchain_,
                        &context_->vk_cpu_alloc_callbacks());
  vkDestroySurfaceKHR(context_->instance(), surface_,
                      &context_->vk_cpu_alloc_callbacks());
}

yacsl::containers::Tuple<RenderTarget, VkSemaphore>
Swapchain::next_image(this Swapchain& self) noexcept {
  u32         image_index = 0;
  VkSemaphore semaphore   = self.image_acquired_semaphores_[self.frame_index_];
  self.frame_index_       = (self.frame_index_ + 1) % FRAMES_IN_FLIGHT;
  vkAcquireNextImageKHR(self.context_->device(), self.swapchain_, 0, semaphore,
                        VK_NULL_HANDLE, &image_index);
  return {
      RenderTarget{
          .image      = self.images_[image_index],
          .image_view = self.image_views_[image_index],
      },
      semaphore,
  };
}

} // namespace vulkan_renderer
