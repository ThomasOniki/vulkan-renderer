#pragma once

#include "vulkan-renderer/compiler/export.hpp"
#include "vulkan-renderer/context.hpp"
#include "yacsl/ranges/integer_range.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"
#include <vulkan/vulkan_core.h>

namespace vulkan_renderer {

class GPUBuffer {
public:
  enum class MemoryType : yacsl::u8 {
    Mappable,
    Optimal,
  };

  static GPUBuffer create(Context& context, yacsl::usize size,
                          VkBufferUsageFlags usage,
                          MemoryType memory_type) noexcept VULKAN_RENDERER_API;

  // Uses a staging buffer.
  static GPUBuffer
  create_filled(Context&                                            context,
                yacsl::ranges::Slice<yacsl::u8 const, yacsl::usize> data,
                VkBufferUsageFlags usage) noexcept VULKAN_RENDERER_API;

  ~GPUBuffer() noexcept VULKAN_RENDERER_API;

  GPUBuffer(GPUBuffer const&) = delete;
  GPUBuffer(GPUBuffer&& other) noexcept
      : context_(other.context_), buffer_(other.buffer_),
        memory_(other.memory_) {
    other.context_ = nullptr;
  }
  GPUBuffer& operator=(this GPUBuffer&, GPUBuffer const&) = delete;
  GPUBuffer& operator=(this GPUBuffer&, GPUBuffer&&)      = delete;

  void map(this GPUBuffer&                           self,
           yacsl::ranges::IntegerRange<yacsl::usize> range) noexcept
      VULKAN_RENDERER_API;
  void unmap(this GPUBuffer& self) noexcept VULKAN_RENDERER_API;

  void write_to_mapped(this GPUBuffer const&                               self,
                       yacsl::ranges::Slice<yacsl::u8 const, yacsl::usize> data,
                       yacsl::usize offset = 0) noexcept VULKAN_RENDERER_API;

  void flush_mapped(this GPUBuffer const&                     self,
                    yacsl::ranges::IntegerRange<yacsl::usize> range) noexcept
      VULKAN_RENDERER_API;

private:
  GPUBuffer(Context& context) noexcept : context_(&context) {}

private:
  Context*       context_ = nullptr;
  VkBuffer       buffer_  = VK_NULL_HANDLE;
  VkDeviceMemory memory_  = VK_NULL_HANDLE;
  void*          mapped_  = nullptr;
};

} // namespace vulkan_renderer
