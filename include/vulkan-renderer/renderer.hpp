#pragma once

#include "vulkan-renderer/compiler/export.hpp"
#include "vulkan-renderer/context.hpp"
#include "vulkan-renderer/gpu_scene.hpp"
#include "vulkan-renderer/scene_data.hpp"

namespace vulkan_renderer {

class Renderer {
public:
  static Renderer create(Context& context) noexcept { return {context}; }

  ~Renderer() = default;

  Renderer(Renderer const&)                                 = delete;
  Renderer(Renderer&&)                                      = delete;
  Renderer& operator=(this Renderer& self, Renderer const&) = delete;
  Renderer& operator=(this Renderer& self, Renderer&&)      = delete;

  GPUScene load_scene(this Renderer const& self,
                      SceneData const&     data) noexcept VULKAN_RENDERER_API;

private:
  Renderer(Context& context) noexcept : context_(&context) {}

  Context* context_;
};

} // namespace vulkan_renderer
