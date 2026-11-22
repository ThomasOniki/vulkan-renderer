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
    auto index_map =
        yacsl::containers::HashMap<detail::PipelineDesc, yacsl::usize, 8>(8);

    for (const auto& obj : desc.objects) {
      detail::PipelineDesc pipeline_desc = {
          .vertex_desc = obj.vertex_desc,
          .index_type  = obj.index_type,
      };
      auto inserted = index_map.try_insert(pipeline_desc, pipelines_.len());
      yacsl::usize pipeline_index = pipelines_.len();
      if (inserted.is_value()) {
        pipelines_.push(pipeline_desc);
      } else {
        pipeline_index = inserted.error_unchecked();
      }

      yacsl::u32 stride = obj.vertex_desc.stride;
      vertices_.resize((vertices_.len() + stride - 1) -
                       (vertices_.len() + stride - 1) % stride);

      auto index_size = static_cast<yacsl::u32>(obj.index_type);

      pipelines_[pipeline_index].cmds.push(VkDrawIndexedIndirectCommand{
          .indexCount = static_cast<yacsl::u32>(obj.indices.len() / index_size),
          .instanceCount = 1,
          .firstIndex    = static_cast<yacsl::u32>(indices_.len() / index_size),
          .vertexOffset  = static_cast<yacsl::i32>(vertices_.len() / stride),
          .firstInstance = 0,
      });

      vertices_.push_range(obj.vertices);
      indices_.push_range(obj.indices);
    }
  }

private:
  struct Pipeline {
    detail::PipelineDesc                                    desc;
    yacsl::containers::Vector<VkDrawIndexedIndirectCommand> cmds;
  };

private:
  yacsl::containers::Vector<Pipeline>  pipelines_;
  yacsl::containers::Vector<yacsl::u8> vertices_;
  yacsl::containers::Vector<yacsl::u8> indices_;
};

} // namespace vulkan_renderer
