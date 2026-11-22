#include "vulkan-renderer/context.hpp"
#include "glfw.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/memory/arena.hpp"
#include "yacsl/type_aliases.hpp"
#include <vulkan/vulkan_core.h>

namespace vulkan_renderer {

using yacsl::u32, yacsl::usize;
using yacsl::containers::Vector, yacsl::containers::StackArray;
using yacsl::debug::assert_eq, yacsl::debug::assert_;
using yacsl::memory::Alignment;

struct Context::VulkanLoaderAllocator::Callbacks {
  // NOLINTBEGIN(bugprone-easily-swappable-parameters)
  static VKAPI_ATTR void* VKAPI_CALL
  alloc(void* pself, yacsl::usize size, yacsl::usize alignment,
        VkSystemAllocationScope /*scope*/) noexcept {
    auto& self = *static_cast<VulkanLoaderAllocator*>(pself);
    void* ptr =
        self.arena.allocate(size, yacsl::memory::Alignment{alignment}).data();
    self.vulkan_loader_allocations.insert(ptr, size);
    return ptr;
  }
  static VKAPI_ATTR void* VKAPI_CALL
  realloc(void* pself, void* original, yacsl::usize size,
          yacsl::usize alignment, VkSystemAllocationScope /*scope*/) noexcept {
    auto& self = *static_cast<VulkanLoaderAllocator*>(pself);
    void* ptr =
        self.arena.allocate(size, yacsl::memory::Alignment{alignment}).data();
    memcpy(ptr, original, self.vulkan_loader_allocations.get(original));
    self.vulkan_loader_allocations.insert(ptr, size);
    return ptr;
  }
  static VKAPI_ATTR void VKAPI_CALL free(void* pself, void* ptr) noexcept {
    auto& self = *static_cast<VulkanLoaderAllocator*>(pself);
    self.vulkan_loader_allocations.remove(ptr);
  }
  static VKAPI_ATTR void VKAPI_CALL internal_alloc(
      void* /*pself*/, usize /*size*/, VkInternalAllocationType /*type*/,
      VkSystemAllocationScope /*scope*/) noexcept {}
  static VKAPI_ATTR void VKAPI_CALL internal_free(
      void* /*pself*/, usize /*size*/, VkInternalAllocationType /*type*/,
      VkSystemAllocationScope /*scope*/) noexcept {}
  // NOLINTEND(bugprone-easily-swappable-parameters)
};

void Context::create_instance(this Context& self) noexcept {
#ifdef VULKAN_RENDERER_VALIDATION_LAYERS
  StackArray<char const*, 1> layers = {"VK_LAYER_KHRONOS_validation"};
#else
  StackArray<char const*, 0> layers = {};
#endif
  Vector<char const*, 10, yacsl::memory::MainArenaAllocator, u32> extensions = {
      "VK_KHR_portability_enumeration",
  };
  detail::glfw_retain();

  u32          glfw_extension_count = 0;
  char const** glfw_extensions =
      glfwGetRequiredInstanceExtensions(&glfw_extension_count);
  for (u32 i = 0; i < glfw_extension_count; ++i) {
    extensions.push(glfw_extensions[i]);
  }
  VkInstanceCreateInfo info = {
      .sType                 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .flags                 = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
      .pApplicationInfo      = nullptr,
      .enabledLayerCount     = layers.len(),
      .ppEnabledLayerNames   = layers.data(),
      .enabledExtensionCount = extensions.len(),
      .ppEnabledExtensionNames = extensions.data(),
  };
  assert_eq(vkCreateInstance(&info, &self.vk_loader_alloc_callbacks_,
                             &self.instance_),
            VK_SUCCESS);

  detail::glfw_release();
}

void Context::pick_physical_device(this Context& self) noexcept {
  u32 count = 0;
  assert_eq(vkEnumeratePhysicalDevices(self.instance_, &count, nullptr),
            VK_SUCCESS);
  auto devices = yacsl::containers::Vector<VkPhysicalDevice, 4>(count);
  assert_eq(vkEnumeratePhysicalDevices(self.instance_, &count, devices.data()),
            VK_SUCCESS);
  self.physical_device_ = devices[0];
}

void Context::get_queue_family_index(this Context& self) noexcept {
  u32 count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(self.physical_device_, &count,
                                           nullptr);
  auto properties_vec = Vector<VkQueueFamilyProperties, 4,
                               yacsl::memory::MainArenaAllocator, u32>(count);
  vkGetPhysicalDeviceQueueFamilyProperties(self.physical_device_, &count,
                                           properties_vec.data());

  bool found = false;

  for (u32 index : properties_vec.indices()) {
    if ((properties_vec[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) > 0) {
      self.queue_family_index_ = index;
      found                    = true;
      break;
    }
  }
  assert_(found, "No queue families were found!");
}

void Context::create_device(this Context& self) noexcept {
  float                                  priority     = 1;
  StackArray<VkDeviceQueueCreateInfo, 1> queue_infoes = {{{
      .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = self.queue_family_index_,
      .queueCount       = 1,
      .pQueuePriorities = &priority,
  }}};
  StackArray<char const*, 1>             extensions   = {{"VK_KHR_swapchain"}};
  VkDeviceCreateInfo                     info         = {
      .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount    = queue_infoes.len(),
      .pQueueCreateInfos       = queue_infoes.data(),
      .enabledExtensionCount   = extensions.len(),
      .ppEnabledExtensionNames = extensions.data(),
  };
  assert_eq(vkCreateDevice(self.physical_device_, &info,
                           &self.vk_loader_alloc_callbacks_, &self.device_),
            VK_SUCCESS);
}

Context::Context(InitTag /*tag*/) noexcept
    : vk_loader_alloc_callbacks_{
          .pUserData       = &vk_loader_alloc_,
          .pfnAllocation   = VulkanLoaderAllocator::Callbacks::alloc,
          .pfnReallocation = VulkanLoaderAllocator::Callbacks::realloc,
          .pfnFree         = VulkanLoaderAllocator::Callbacks::free,
          .pfnInternalAllocation =
              VulkanLoaderAllocator::Callbacks::internal_alloc,
          .pfnInternalFree = VulkanLoaderAllocator::Callbacks::internal_free,
      } {
  create_instance();
  pick_physical_device();
  get_queue_family_index();
  create_device();
  vkGetDeviceQueue(device_, queue_family_index_, 0, &queue_);
}

Context::~Context() noexcept {
  vkDestroyDevice(device_, &vk_loader_alloc_callbacks_);
  vkDestroyInstance(instance_, &vk_loader_alloc_callbacks_);
}

} // namespace vulkan_renderer
