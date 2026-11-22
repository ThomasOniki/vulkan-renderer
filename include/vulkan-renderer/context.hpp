#pragma once

#include "yacsl/type_aliases.hpp"
#include <vulkan/vulkan_core.h>
#include <yacsl/containers/hash_map.hpp>
#include <yacsl/memory/arena.hpp>
#include <yacsl/memory/lifetimes.hpp>

namespace vulkan_renderer {

class Context {
public:
  // Initialization has to take place in the constructor because
  // allocation_callbacks.pUserData needs to point to the arena member.
  // Changing pUserData doesn't work.
  static auto create() noexcept { return Context(InitTag{}); }
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
  [[nodiscard]] VkAllocationCallbacks const&
  vk_loader_allocation_callbacks(this Context const& self) noexcept {
    return self.vk_loader_alloc_callbacks_;
  }
  [[nodiscard]] yacsl::u32
  queue_family_index(this Context const& self) noexcept {
    return self.queue_family_index_;
  }
  [[nodiscard]] VkQueue queue(this Context const& self) noexcept {
    return self.queue_;
  }

private:
  struct VulkanLoaderAllocator {
    yacsl::memory::Arena arena;

    using Allocations = yacsl::containers::HashMap<void*, yacsl::usize, 16>;
    Allocations vulkan_loader_allocations = Allocations(16);

    struct Callbacks;
  };

  VulkanLoaderAllocator vk_loader_alloc_;
  VkAllocationCallbacks vk_loader_alloc_callbacks_ = {};
  VkInstance            instance_                  = VK_NULL_HANDLE;
  VkPhysicalDevice      physical_device_           = VK_NULL_HANDLE;
  VkDevice              device_                    = VK_NULL_HANDLE;
  yacsl::u32            queue_family_index_        = 0;
  VkQueue               queue_                     = VK_NULL_HANDLE;

private:
  struct InitTag {};

  // Initialization has to take place in the constructor because
  // allocation_callbacks.pUserData needs to point to the arena member.
  // Changing pUserData doesn't work.
  Context(InitTag /*tag*/) noexcept;

  void create_instance(this Context& self) noexcept;
  void pick_physical_device(this Context& self) noexcept;
  void get_queue_family_index(this Context& self) noexcept;
  void create_device(this Context& self) noexcept;
};

} // namespace vulkan_renderer
