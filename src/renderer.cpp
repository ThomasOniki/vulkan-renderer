#include "vulkan-renderer/renderer.hpp"
#include "vulkan-renderer/gpu_scene.hpp"
#include "yacsl/type_aliases.hpp"
#include <vulkan/vulkan_core.h>

namespace vulkan_renderer {

using yacsl::u8, yacsl::u64, yacsl::usize;

GPUScene Renderer::load_scene(this Renderer const& self,
                              SceneData const&     data) noexcept {
  return {
      GPUScene::Buffers{
          .vertex = GPUBuffer::create_filled(*self.context_, data.vertices(),
                                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
          .index  = GPUBuffer::create_filled(*self.context_, data.indices(),
                                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
          .indirect = GPUBuffer::create_filled(
              *self.context_, data.indirect_commands().bytes(),
              VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT),
      },
  };
}

} // namespace vulkan_renderer
