#pragma once

#include "vulkan-renderer/compiler/export.hpp"
#include "yacsl/type_aliases.hpp"
#include <vulkan/vulkan_core.h>
#include <yacsl/containers/hash_map.hpp>
#include <yacsl/memory/arena.hpp>
#include <yacsl/memory/lifetimes.hpp>

namespace vulkan_renderer {

class Context {
public:
  Context() noexcept VULKAN_RENDERER_API;
  ~Context() noexcept VULKAN_RENDERER_API;

  Context(Context const&)            = delete;
  Context(Context&&)                 = delete;
  Context& operator=(Context const&) = delete;
  Context& operator=(Context&&)      = delete;

  [[nodiscard]] auto instance(this Context const& self) noexcept {
    return self.instance_;
  }
  [[nodiscard]] auto device(this Context const& self) noexcept {
    return self.device_;
  }
  [[nodiscard]] auto physical_device(this Context const& self) noexcept {
    return self.physical_device_;
  }
  [[nodiscard]] auto queue(this Context const& self) noexcept {
    return self.queue_;
  }

  [[nodiscard]] auto const& device_info(this Context const& self) noexcept {
    return self.device_info_;
  }
  [[nodiscard]] auto const&
  vk_cpu_alloc_callbacks(this Context const& self) noexcept {
    return self.vk_cpu_alloc_callbacks_;
  }

public:
  struct DeviceInfo {
    yacsl::u32 queue_family_index         = 0;
    yacsl::u32 mappable_memory_type_index = VK_MAX_MEMORY_TYPES;
    yacsl::u32 optimal_memory_type_index  = VK_MAX_MEMORY_TYPES;
  };

private:
  struct VulkanCPUAlloc {
    yacsl::memory::Arena arena;

    using Allocations = yacsl::containers::HashMap<void*, yacsl::usize, 16>;
    Allocations vulkan_loader_allocations = Allocations(16);

    struct Callbacks;
  };

  VulkanCPUAlloc        vk_cpu_alloc_;
  VkAllocationCallbacks vk_cpu_alloc_callbacks_ = {};
  VkInstance            instance_               = VK_NULL_HANDLE;
  VkPhysicalDevice      physical_device_        = VK_NULL_HANDLE;
  VkDevice              device_                 = VK_NULL_HANDLE;
  VkQueue               queue_                  = VK_NULL_HANDLE;
  DeviceInfo            device_info_;

private:
  void create_instance(this Context& self) noexcept;
  void pick_physical_device(this Context& self) noexcept;
  void find_queue_family_index(this Context& self) noexcept;
  void find_memory_type_indices(this Context& self) noexcept;
  void create_device(this Context& self) noexcept;
};

} // namespace vulkan_renderer
