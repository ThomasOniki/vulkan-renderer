#pragma once

#include "yacsl/type_aliases.hpp"
#include <string.h>
#include <vulkan/vulkan_core.h>
#include <yacsl/containers/hash_map.hpp>
#include <yacsl/memory/arena.hpp>
#include <yacsl/memory/lifetimes.hpp>

namespace vulkan_renderer {

class Context {
public:
  static Context create() noexcept;
  ~Context() noexcept;

  Context(Context const&)            = delete;
  Context(Context&&)                 = delete;
  Context& operator=(Context const&) = delete;
  Context& operator=(Context&&)      = delete;

  [[nodiscard]] VkDevice device(this Context const& self) noexcept {
    return self.device_;
  }
  [[nodiscard]] VkInstance instance(this Context const& self) noexcept {
    return self.instance_;
  }
  [[nodiscard]] VkAllocationCallbacks
  vk_loader_allocation_callbacks(this Context const& self) noexcept {
    return self.vk_loader_alloc_callbacks_;
  }

private:
  struct VulkanLoaderAllocator {
    yacsl::memory::Arena arena;

    using Allocations = yacsl::containers::HashMap<void*, yacsl::usize, 16>;
    Allocations vulkan_loader_allocations = Allocations(16);

    struct Callbacks;
  };

  Context(VulkanLoaderAllocator&& allocator, VkAllocationCallbacks callbacks,
          VkInstance instance, VkDevice device) noexcept
      : vk_loader_alloc_(yacsl::memory::move(allocator)),
        vk_loader_alloc_callbacks_(callbacks), instance_(instance),
        device_(device) {
    vk_loader_alloc_callbacks_.pUserData = &vk_loader_alloc_;
  }

private:
  VulkanLoaderAllocator vk_loader_alloc_;
  VkAllocationCallbacks vk_loader_alloc_callbacks_ = {};
  VkInstance            instance_;
  VkDevice              device_;
};

} // namespace vulkan_renderer
