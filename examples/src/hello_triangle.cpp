#include "vulkan-renderer/context.hpp"
#include "vulkan-renderer/gpu_scene.hpp"
#include "vulkan-renderer/object_desc.hpp"
#include "vulkan-renderer/renderer.hpp"
#include "vulkan-renderer/scene_data.hpp"
#include "vulkan-renderer/scene_desc.hpp"
#include "vulkan-renderer/vertex_desc.hpp"
#include "vulkan-renderer/window.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/math/matrix.hpp"
#include "yacsl/type_aliases.hpp"

using vulkan_renderer::Context, vulkan_renderer::Renderer,
    vulkan_renderer::Window;
using vulkan_renderer::SceneDesc, vulkan_renderer::ObjectDesc,
    vulkan_renderer::VertexDesc, vulkan_renderer::VertexAttributeType,
    vulkan_renderer::IndexType, vulkan_renderer::SceneData,
    vulkan_renderer::GPUScene;
using yacsl::containers::StackArray, yacsl::usize, yacsl::u16;
using yacsl::math::Vec2, yacsl::f32;

static SceneData create_scene_data() {
  struct Vertex {
    Vec2<f32> position;
  };
  StackArray<Vertex, 3, usize> vertices = {{
      {.position = {-0.5, 0.5}},
      {.position = {0.5, 0.5}},
      {.position = {0.5, 0.5}},
  }};
  StackArray<u16, 3, usize>    indices  = {0, 1, 2};

  VertexDesc vertex_desc = {
      .attributes = {{
          {
              .type   = VertexAttributeType::F32x2,
              .offset = offsetof(Vertex, position),
          },
      }},
      .stride     = sizeof(Vertex),
  };
  StackArray<ObjectDesc, 1, usize> objects    = {{
      {
          .vertex_desc = vertex_desc,
          .index_type  = IndexType::U16,
          .vertices    = vertices.bytes(),
          .indices     = indices.bytes(),
      },
  }};
  SceneDesc                        scene_desc = {
      .objects = objects.slice(),
  };
  return {scene_desc};
}

int main() {
  auto context = Context::create();

  auto renderer = Renderer::create(context);

  SceneData scene_data = create_scene_data();
  GPUScene  gpu_scene  = renderer.load_scene(scene_data);

  auto window    = Window::open({800, 400}, "hello triangle");
  auto swapchain = window.create_swapchain(context);
  while (!window.should_close()) {
    window.update();
  }
  yacsl::debug::flush_logger();
}
