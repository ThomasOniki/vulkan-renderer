#pragma once

#include "vulkan-renderer/object_desc.hpp"
#include "vulkan-renderer/scene_desc.hpp"
#include "vulkan-renderer/vertex_desc.hpp"
#include "yacsl/containers/hash.hpp"
#include "yacsl/containers/hash_map.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/type_aliases.hpp"
#include <vulkan/vulkan_core.h>

namespace vulkan_renderer::detail {
struct PipelineDesc {
  VertexDesc vertex_desc;
  IndexType  index_type;

  constexpr bool operator==(this PipelineDesc const&,
                            PipelineDesc const&) noexcept = default;
};
} // namespace vulkan_renderer::detail

template <>
struct yacsl::containers::Hash<vulkan_renderer::detail::PipelineDesc,
                               yacsl::usize> {
  constexpr yacsl::usize
  operator()(this Hash const& /*self*/,
             vulkan_renderer::detail::PipelineDesc const& pipeline) noexcept {
    return static_cast<yacsl::usize>(pipeline.index_type) * 1234 ^
           pipeline.vertex_desc.stride;
  }
};

namespace vulkan_renderer {

class SceneData {
public:
  SceneData(SceneDesc const& desc) {
    using yacsl::usize;
    using yacsl::containers::HashMap, yacsl::containers::Vector;
    auto index_map = HashMap<detail::PipelineDesc, Vector<usize>, 8>(8);

    for (usize const obj_index : desc.objects.indices()) {
      detail::PipelineDesc pipeline_desc = {
          .vertex_desc = desc.objects[obj_index].vertex_desc,
          .index_type  = desc.objects[obj_index].index_type,
      };
      auto inserted = index_map.try_insert(pipeline_desc, 1ZU, obj_index);
      if (inserted.is_error()) {
        inserted.error_unchecked().push(obj_index);
      }
    }

    for (auto const& [pipeline_desc, indices] : index_map) {
      draw_calls_.push(DrawCall{
          .desc = pipeline_desc,
          .indirect_buffer_offset =
              indirect_commands_.len() * sizeof(VkDrawIndexedIndirectCommand),
          .indirect_draw_count = static_cast<yacsl::u32>(indices.len()),
      });
      for (auto const index : indices) {
        yacsl::u32 stride = desc.objects[index].vertex_desc.stride;
        vertices_.resize((vertices_.len() + stride - 1) -
                         (vertices_.len() + stride - 1) % stride);

        auto index_size =
            static_cast<yacsl::u32>(desc.objects[index].index_type);
        indirect_commands_.push(VkDrawIndexedIndirectCommand{
            .indexCount = static_cast<yacsl::u32>(
                desc.objects[index].indices.len() / index_size),
            .instanceCount = 1,
            .firstIndex = static_cast<yacsl::u32>(indices_.len() / index_size),
            .vertexOffset  = static_cast<yacsl::i32>(vertices_.len() / stride),
            .firstInstance = 0,
        });

        vertices_.push_range(desc.objects[index].vertices);
        indices_.push_range(desc.objects[index].indices);
      }
    }
  }

  [[nodiscard]] auto draw_calls(this SceneData const& self) noexcept {
    return self.draw_calls_.slice();
  }
  [[nodiscard]] auto vertices(this SceneData const& self) noexcept {
    return self.vertices_.slice();
  }
  [[nodiscard]] auto indices(this SceneData const& self) noexcept {
    return self.indices_.slice();
  }
  [[nodiscard]] auto indirect_commands(this SceneData const& self) noexcept {
    return self.indirect_commands_.slice();
  }

private:
  struct DrawCall {
    detail::PipelineDesc desc;
    yacsl::u64           indirect_buffer_offset = 0;
    yacsl::u32           indirect_draw_count    = 0;
  };

private:
  yacsl::containers::Vector<DrawCall>                     draw_calls_;
  yacsl::containers::Vector<yacsl::u8>                    vertices_;
  yacsl::containers::Vector<yacsl::u8>                    indices_;
  yacsl::containers::Vector<VkDrawIndexedIndirectCommand> indirect_commands_;
};

} // namespace vulkan_renderer
