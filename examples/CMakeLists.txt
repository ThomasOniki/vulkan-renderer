file(GLOB_RECURSE VULKAN_RENDERER_EXAMPLES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
foreach(path IN LISTS VULKAN_RENDERER_EXAMPLES)
  get_filename_component(name "${path}" NAME_WE)

  add_executable("${name}" "${path}")
  add_warnings("${name}")

  target_compile_definitions(
    "${name}" PRIVATE "VULKAN_RENDERER_EXAMPLES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

  target_link_libraries("${name}" PRIVATE vulkan-renderer)
endforeach()
