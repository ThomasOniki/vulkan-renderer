#include "vulkan-renderer/gpu_buffer.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"
#include <vulkan/vulkan_core.h>

namespace vulkan_renderer {

using yacsl::u8, yacsl::u32, yacsl::usize, yacsl::ranges::Slice;
using yacsl::debug::assert_, yacsl::debug::assert_eq;

GPUBuffer GPUBuffer::create(Context& context, usize size,
                            VkBufferUsageFlags usage,
                            MemoryType         memory_type) noexcept {
  auto buffer = GPUBuffer(context);

  yacsl::u32 const queue_family_index =
      context.device_info().queue_family_index;
  VkBufferCreateInfo info = {
      .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size                  = size,
      .usage                 = usage,
      .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 1,
      .pQueueFamilyIndices   = &queue_family_index,
  };
  assert_eq(vkCreateBuffer(context.device(), &info,
                           &context.vk_loader_allocation_callbacks(),
                           &buffer.buffer_),
            VK_SUCCESS);

  VkMemoryAllocateInfo alloc_info = {
      .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize  = size,
      .memoryTypeIndex = memory_type == MemoryType::Mappable
                             ? context.device_info().mappable_memory_type_index
                             : context.device_info().optimal_memory_type_index,
  };
  assert_eq(vkAllocateMemory(context.device(), &alloc_info,
                             &context.vk_loader_allocation_callbacks(),
                             &buffer.memory_),
            VK_SUCCESS);

  return buffer;
}

GPUBuffer::~GPUBuffer() noexcept {
  if (context_ != nullptr) {
    vkFreeMemory(context_->device(), memory_,
                 &context_->vk_loader_allocation_callbacks());
    vkDestroyBuffer(context_->device(), buffer_,
                    &context_->vk_loader_allocation_callbacks());
  }
}

void GPUBuffer::map(this GPUBuffer&                    self,
                    yacsl::ranges::IntegerRange<usize> range) noexcept {
  vkMapMemory(self.context_->device(), self.memory_, range.begin(),
              range.end() - range.begin(), 0, &self.mapped_);
}
void GPUBuffer::unmap(this GPUBuffer& self) noexcept {
  vkUnmapMemory(self.context_->device(), self.memory_);
  self.mapped_ = nullptr;
}

void GPUBuffer::write_to_mapped(this GPUBuffer const&                 self,
                                yacsl::ranges::Slice<yacsl::u8 const> data,
                                yacsl::usize offset) noexcept {
  assert_(self.mapped_ != nullptr, "Tried to write to unmapped GPUBuffer!\n");

  yacsl::memory::copy(data, static_cast<u8*>(self.mapped_) + offset);
}

void GPUBuffer::flush_mapped(
    this GPUBuffer const&                     self,
    yacsl::ranges::IntegerRange<yacsl::usize> range) noexcept {
  VkMappedMemoryRange memory_range = {
      .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
      .memory = self.memory_,
      .offset = range.begin(),
      .size   = range.end() - range.begin(),
  };
  assert_eq(
      vkFlushMappedMemoryRanges(self.context_->device(), 1, &memory_range),
      VK_SUCCESS);
}

GPUBuffer GPUBuffer::create_filled(Context&                      context,
                                   Slice<u8 const, yacsl::usize> data,
                                   VkBufferUsageFlags usage) noexcept {
  GPUBuffer staging_buffer =
      create(context, static_cast<usize>(data.len()),
             VK_BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryType::Mappable);
  staging_buffer.map({0, data.len()});
  staging_buffer.write_to_mapped(data);
  staging_buffer.flush_mapped({0, data.len()});
  staging_buffer.unmap();

  GPUBuffer buffer =
      create(context, static_cast<usize>(data.len()),
             usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryType::Optimal);

  return buffer;
}

} // namespace vulkan_renderer
