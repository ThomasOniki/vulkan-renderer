#pragma once

#include "vulkan-renderer/object_desc.hpp"
#include "yacsl/ranges/slice.hpp"

namespace vulkan_renderer {

struct SceneDesc {
  yacsl::ranges::Slice<ObjectDesc> objects;
};

} // namespace vulkan_renderer
