find_package(Vulkan REQUIRED)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE stb-master)

add_library(glfw INTERFACE)
target_include_directories(glfw INTERFACE glfw-3.3.4.bin.WIN64/include)
target_link_directories(glfw INTERFACE glfw-3.3.4.bin.WIN64/lib-vc2019)
target_link_libraries(glfw INTERFACE GLFW3)

add_library(loguru STATIC)
target_sources(loguru PRIVATE
    "loguru-2.1.0/loguru.hpp"
    "loguru-2.1.0/loguru.cpp"
    )
target_include_directories(loguru PUBLIC loguru-2.1.0)

add_library(tinyobjloader STATIC)
target_sources(tinyobjloader PRIVATE
    "tinyobjloader-master/tiny_obj_loader.h"
    "tinyobjloader-master/tiny_obj_loader.cc"
    )
target_include_directories(tinyobjloader PUBLIC tinyobjloader-master)

add_library(imgui STATIC)
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

file(GLOB sources imgui/*.cpp)

message("${sources}")

target_sources(imgui PRIVATE
    "${sources}"

    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_vulkan.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_glfw.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp"
    )
target_link_libraries(imgui PUBLIC "glfw;Vulkan::Vulkan")

add_library(ImGuizmo STATIC)
target_sources(ImGuizmo PRIVATE
    "ImGuizmo-1.83/ImGuizmo.h"
    "ImGuizmo-1.83/ImGuizmo.cpp"
    )
target_include_directories(ImGuizmo PUBLIC "ImGuizmo-1.83")
target_link_libraries(ImGuizmo imgui)

add_library(json INTERFACE)
target_include_directories(json INTERFACE json)
