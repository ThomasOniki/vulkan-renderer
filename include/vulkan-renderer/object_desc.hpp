#pragma once

#include "vulkan-renderer/vertex_desc.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"

namespace vulkan_renderer {

// size = (usize)type
enum class IndexType : yacsl::u8 {
  U16 = 2,
  U32 = 4,
};

struct ObjectDesc {
  VertexDesc vertex_desc;
  IndexType  index_type = IndexType::U32;

  yacsl::ranges::Slice<yacsl::u8> vertices = {nullptr, 0ZU};
  yacsl::ranges::Slice<yacsl::u8> indices  = {nullptr, 0ZU};

  constexpr bool operator==(this ObjectDesc const&,
                            ObjectDesc const&) noexcept = default;
};

} // namespace vulkan_renderer
