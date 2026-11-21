#include "vulkan-renderer/context.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/type_aliases.hpp"
#include <alloca.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

namespace vulkan_renderer {

using yacsl::u32, yacsl::usize;
using yacsl::containers::Vector, yacsl::containers::StackArray;
using yacsl::debug::assert_eq;
using yacsl::memory::Alignment;

struct Context::VulkanLoaderAllocator::Callbacks {
  // NOLINTBEGIN(bugprone-easily-swappable-parameters)
  static VKAPI_ATTR void* VKAPI_CALL
  alloc(void* pself, yacsl::usize size, yacsl::usize alignment,
        VkSystemAllocationScope /*scope*/) noexcept {
    VulkanLoaderAllocator& self = *static_cast<VulkanLoaderAllocator*>(pself);
    void*                  ptr =
        self.arena.allocate(size, yacsl::memory::Alignment{alignment}).data();
    self.vulkan_loader_allocations.insert(ptr, size);
    return ptr;
  }
  static VKAPI_ATTR void* VKAPI_CALL
  realloc(void* pself, void* original, yacsl::usize size,
          yacsl::usize alignment, VkSystemAllocationScope /*scope*/) noexcept {
    VulkanLoaderAllocator& self = *static_cast<VulkanLoaderAllocator*>(pself);
    void*                  ptr =
        self.arena.allocate(size, yacsl::memory::Alignment{alignment}).data();
    memcpy(ptr, original, self.vulkan_loader_allocations.get(original));
    self.vulkan_loader_allocations.insert(ptr, size);
    return ptr;
  }
  static VKAPI_ATTR void VKAPI_CALL free(void* pself, void* ptr) noexcept {
    VulkanLoaderAllocator& self = *static_cast<VulkanLoaderAllocator*>(pself);
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

static VkInstance
create_instance(VkAllocationCallbacks const& allocation_callbacks) noexcept {
  VkApplicationInfo app_info = {
      .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName   = "app",
      .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
      .pEngineName        = "no engine",
      .engineVersion      = VK_MAKE_VERSION(0, 0, 0),
      .apiVersion         = VK_API_VERSION_1_1,
  };
#ifdef VULKAN_RENDERER_VALIDATION_LAYERS
  StackArray<char const*, 1> layers = {"VK_LAYER_KHRONOS_validation"};
#else
  StackArray<char const*, 0> layers = {};
#endif
  StackArray<char const*, 1> extensions = {
      "VK_KHR_portability_enumeration",
  };
  VkInstanceCreateInfo info = {
      .sType                 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .flags                 = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
      .pApplicationInfo      = &app_info,
      .enabledLayerCount     = layers.len(),
      .ppEnabledLayerNames   = layers.data(),
      .enabledExtensionCount = extensions.len(),
      .ppEnabledExtensionNames = extensions.data(),
  };
  VkInstance instance = VK_NULL_HANDLE;
  assert_eq(vkCreateInstance(&info, &allocation_callbacks, &instance),
            VK_SUCCESS);
  return instance;
}

static VkPhysicalDevice pick_physical_device(VkInstance instance) noexcept {
  u32 count = 0;
  assert_eq(vkEnumeratePhysicalDevices(instance, &count, nullptr), VK_SUCCESS);
  auto devices = yacsl::containers::Vector<VkPhysicalDevice, 4>(count);
  assert_eq(vkEnumeratePhysicalDevices(instance, &count, devices.data()),
            VK_SUCCESS);
  return devices[0];
}

static VkDevice create_device(VkAllocationCallbacks const& allocation_callbacks,
                              VkPhysicalDevice physical_device) noexcept {
  VkDeviceCreateInfo info   = {};
  VkDevice           device = VK_NULL_HANDLE;
  assert_eq(
      vkCreateDevice(physical_device, &info, &allocation_callbacks, &device),
      VK_SUCCESS);
  return device;
}

Context Context::create() noexcept {
  VulkanLoaderAllocator alloc;
  VkAllocationCallbacks allocation_callbacks = {
      // NOLINTNEXTLINE
      .pUserData             = const_cast<VulkanLoaderAllocator*>(&alloc),
      .pfnAllocation         = VulkanLoaderAllocator::Callbacks::alloc,
      .pfnReallocation       = VulkanLoaderAllocator::Callbacks::realloc,
      .pfnFree               = VulkanLoaderAllocator::Callbacks::free,
      .pfnInternalAllocation = VulkanLoaderAllocator::Callbacks::internal_alloc,
      .pfnInternalFree       = VulkanLoaderAllocator::Callbacks::internal_free,
  };

  VkInstance       instance        = create_instance(allocation_callbacks);
  VkPhysicalDevice physical_device = pick_physical_device(instance);
  VkDevice device = create_device(allocation_callbacks, physical_device);
  return {yacsl::memory::move(alloc), allocation_callbacks, instance, device};
}

Context::~Context() noexcept {
  vkDestroyDevice(device_, &vk_loader_alloc_callbacks_);
  vkDestroyInstance(instance_, &vk_loader_alloc_callbacks_);
}

} // namespace vulkan_renderer
