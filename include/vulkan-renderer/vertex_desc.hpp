#pragma once

#include "yacsl/containers/vector.hpp"
#include "yacsl/type_aliases.hpp"

namespace vulkan_renderer {

enum class VertexAttributeType : yacsl::u8;

struct VertexAttribute {
  VertexAttributeType type;
  yacsl::u32          offset;

  constexpr bool operator==(this VertexAttribute,
                            VertexAttribute) noexcept = default;
};

struct VertexDesc {
  yacsl::containers::Vector<VertexAttribute> attributes;
  yacsl::u32                                 stride;

  constexpr bool operator==(this VertexDesc const&,
                            VertexDesc const&) noexcept = default;
};

enum class VertexAttributeType : yacsl::u8 {
  U8x1,
  U8x2,
  U8x3,
  U8x4,

  U16x1,
  U16x2,
  U16x3,
  U16x4,

  U32x1,
  U32x2,
  U32x3,
  U32x4,

  I8x1,
  I8x2,
  I8x3,
  I8x4,

  I16x1,
  I16x2,
  I16x3,
  I16x4,

  I32x1,
  I32x2,
  I32x3,
  I32x4,

  F16x1,
  F16x2,
  F16x3,
  F16x4,

  F32x1,
  F32x2,
  F32x3,
  F32x4,
};

} // namespace vulkan_renderer
