find_package(Vulkan REQUIRED)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE stb-master)

add_library(dds_image INTERFACE)
target_include_directories(dds_image INTERFACE dds_image-main/include)

add_library(glfw INTERFACE)
target_include_directories(glfw INTERFACE glfw-3.3.8.bin.WIN64/include)
target_link_directories(glfw INTERFACE glfw-3.3.8.bin.WIN64/lib-vc2022)
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

target_sources(imgui PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"

		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp"

		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_vulkan.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_glfw.cpp"

		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp"
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

add_library(concurrentqueue INTERFACE)
target_include_directories(concurrentqueue INTERFACE concurrentqueue-1.0.3)

add_library(threadpool INTERFACE)
target_include_directories(threadpool INTERFACE thread-pool-3.3.0)

add_library(implot STATIC)
target_include_directories(implot PUBLIC implot-master)
target_sources(implot PRIVATE
		"implot-master/implot_internal.h"
		"implot-master/implot.h"
		"implot-master/implot.cpp"
		"implot-master/implot_demo.cpp"
		"implot-master/implot_items.cpp"
		)
target_link_libraries(implot imgui)
