#pragma once

#include "vulkan-renderer/gpu_buffer.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/type_aliases.hpp"
#include <vulkan/vulkan_core.h>

namespace vulkan_renderer {

namespace detail {} // namespace detail

class GPUScene {
public:
  struct Buffers {
    GPUBuffer vertex;
    GPUBuffer index;
    GPUBuffer indirect;
  };
  GPUScene(Buffers&& buffers) noexcept
      : buffers_(yacsl::memory::move(buffers)) {}

  ~GPUScene() noexcept                 = default;
  GPUScene(GPUScene const&)            = delete;
  GPUScene(GPUScene&&)                 = delete;
  GPUScene& operator=(GPUScene const&) = delete;
  GPUScene& operator=(GPUScene&&)      = delete;

  [[nodiscard]] auto const& buffers(this GPUScene const& self) noexcept {
    return self.buffers_;
  }
  [[nodiscard]] auto draw_calls(this GPUScene const& self) noexcept {
    return self.draw_calls_.slice();
  }

  struct DrawCall {
    VkPipeline pipeline;
    yacsl::u64 offset;
    yacsl::u32 indirect_draw_count;
  };

private:
  yacsl::containers::Vector<DrawCall, 16> draw_calls_;

  Buffers buffers_;
};

} // namespace vulkan_renderer
