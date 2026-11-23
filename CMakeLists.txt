cmake_minimum_required(VERSION 3.28)
project(vulkan-renderer LANGUAGES CXX C)

include(
  "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cmake-helpers/cmake-helpers.cmake")

add_library(vulkan-renderer)
add_library(vulkan-renderer::vulkan-renderer ALIAS vulkan-renderer)

source_directory(vulkan-renderer "${CMAKE_CURRENT_SOURCE_DIR}/src")
public_include_directory(vulkan-renderer "${CMAKE_CURRENT_SOURCE_DIR}/include")
gen_export_header(vulkan-renderer "vulkan-renderer/compiler/export.hpp"
                  "VULKAN_RENDERER_API")

add_warnings(vulkan-renderer)
set_utf8(vulkan-renderer)
set_hidden_visibility(vulkan-renderer)
link_threads(vulkan-renderer)
target_compile_features(vulkan-renderer PUBLIC cxx_std_26)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third_party/yacsl")
target_link_libraries(vulkan-renderer PUBLIC yacsl::yacsl)

find_package(Vulkan REQUIRED)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third_party/glfw")
target_link_libraries(vulkan-renderer PRIVATE glfw)
target_link_libraries(vulkan-renderer PUBLIC Vulkan::Vulkan)

option(VULKAN_RENDERER_WERROR "Treat warnings as errors." ON)
option(VULKAN_RENDERER_NO_LIBCPP "Don't link with the c++ standard library."
       OFF)
option(VULKAN_RENDERER_VALIDATION_LAYERS "Enable vulkan validation layers." OFF)

set_target_properties(vulkan-renderer PROPERTIES COMPILE_WARNING_AS_ERROR
                                                 ${VULKAN_RENDERER_WERROR})

target_compile_options(vulkan-renderer
                       PRIVATE "-Wno-missing-field-initializers")

if(VULKAN_RENDERER_NO_LIBCPP)
  target_compile_options(vulkan-renderer PUBLIC "-nostdinc++" "-fno-exceptions")
  target_link_options(vulkan-renderer PUBLIC "-nostdlib++")
  target_compile_definitions(vulkan-renderer PUBLIC VULKAN_RENDERER_NO_LIBCPP)
endif()

if(VULKAN_RENDERER_VALIDATION_LAYERS)
  target_compile_definitions(vulkan-renderer
                             PRIVATE VULKAN_RENDERER_VALIDATION_LAYERS)
endif()

target_precompile_headers(vulkan-renderer PUBLIC
                          "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp")

if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/examples")
endif()

install_project_targets(vulkan-renderer TARGETS vulkan-renderer)
