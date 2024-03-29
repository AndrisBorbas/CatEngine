cmake_minimum_required(VERSION 3.20)
project(CatEngine)

# Get OS information
if (CMAKE_HOST_WIN32)
	set(WINDOWS 1)
elseif (CMAKE_HOST_APPLE)
	set(MACOS 1)
elseif (CMAKE_HOST_UNIX)
	set(LINUX 1)
endif ()

if (CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
	message("Compiler is GNU compatible.")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-function -Wno-unused-variable -DVK_PROTOTYPES=1 -pthread")
elseif (MSVC)
	message("Compiler is MSVC.")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CRT_SECURE_NO_WARNINGS /wd4267 /wd4244 /wd4146")
endif ()

if ((${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdeclspec")
endif ()

set(CMAKE_CXX_STANDARD 20)

message("CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")

add_subdirectory(Libraries)

include_directories(CatEngine)

add_executable(CatEngine CatEngine/main.cpp CatEngine/Cat/CatWindow.hpp CatEngine/Cat/CatWindow.cpp CatEngine/Cat/Controller/CatCamera.cpp CatEngine/Cat/Controller/CatCamera.hpp CatEngine/Cat/Controller/CatInput.cpp CatEngine/Cat/Controller/CatInput.hpp CatEngine/Cat/Objects/CatObject.cpp CatEngine/Cat/Objects/CatObject.hpp CatEngine/Cat/Objects/CatModel.cpp CatEngine/Cat/Objects/CatModel.hpp CatEngine/Cat/VulkanRHI/CatDevice.cpp CatEngine/Cat/VulkanRHI/CatDevice.hpp CatEngine/Cat/Utils/CatUtils.hpp CatEngine/Cat/CatApp.cpp CatEngine/Cat/CatApp.hpp CatEngine/Cat/VulkanRHI/CatBuffer.cpp CatEngine/Cat/VulkanRHI/CatBuffer.hpp CatEngine/Cat/VulkanRHI/CatDescriptors.cpp CatEngine/Cat/VulkanRHI/CatDescriptors.hpp CatEngine/Cat/CatFrameInfo.hpp CatEngine/Cat/VulkanRHI/CatPipeline.cpp CatEngine/Cat/VulkanRHI/CatPipeline.hpp CatEngine/Cat/VulkanRHI/CatRenderer.cpp CatEngine/Cat/VulkanRHI/CatRenderer.hpp CatEngine/Cat/VulkanRHI/CatSwapChain.cpp CatEngine/Cat/VulkanRHI/CatSwapChain.hpp CatEngine/Cat/RenderSystems/CatSimpleRenderSystem.cpp CatEngine/Cat/RenderSystems/CatSimpleRenderSystem.hpp CatEngine/Globals.hpp CatEngine/Cat/CatImgui.cpp CatEngine/Cat/CatImgui.hpp CatEngine/Cat/RenderSystems/CatPointLightRenderSystem.hpp CatEngine/Cat/RenderSystems/CatPointLightRenderSystem.cpp CatEngine/Cat/RenderSystems/CatWireframeRenderSystem.cpp CatEngine/Cat/RenderSystems/CatWireframeRenderSystem.hpp CatEngine/Cat/Objects/CatVolume.cpp CatEngine/Cat/Objects/CatVolume.hpp CatEngine/Cat/Objects/CatLight.cpp CatEngine/Cat/Objects/CatLight.hpp CatEngine/Cat/RenderSystems/CatGridRenderSystem.cpp CatEngine/Cat/RenderSystems/CatGridRenderSystem.hpp CatEngine/Cat/Level/CatLevel.cpp CatEngine/Cat/Level/CatLevel.hpp CatEngine/Cat/Objects/CatObjectType.hpp CatEngine/Cat/Objects/CatAssetLoader.cpp CatEngine/Cat/Objects/CatAssetLoader.hpp CatEngine/Cat/Level/CatChunk.cpp CatEngine/Cat/Level/CatChunk.hpp CatEngine/Cat/Terrain/CatTerrain.cpp CatEngine/Cat/Terrain/CatTerrain.hpp CatEngine/Cat/Texture/CatTexture.cpp CatEngine/Cat/Texture/CatTexture.hpp CatEngine/Cat/RenderSystems/CatTerrainRenderSystem.cpp CatEngine/Cat/RenderSystems/CatTerrainRenderSystem.hpp CatEngine/Cat/Rendering/CatFrustum.hpp)

# ###Vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} glm tinyobjloader stb_image dds_image loguru imgui ImGuizmo json concurrentqueue threadpool implot)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw)

if (MSVC)
	# target_compile_options(${PROJECT_NAME} PUBLIC "/ZI")
	target_link_options(${PROJECT_NAME} PUBLIC "/INCREMENTAL /ZI")
endif ()

include(Shaders.cmake)

# Find all shaders.
file(GLOB_RECURSE vertex-shaders ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/*.vert)
file(GLOB_RECURSE fragment-shaders ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/*.frag)
file(GLOB_RECURSE compute-shaders ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/*.comp)
file(GLOB_RECURSE geometry-shaders ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/*.geom)
file(GLOB_RECURSE tessellation-control-shaders ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/*.tesc)
file(GLOB_RECURSE tessellation-evaluation-shaders ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/*.tese)

# Add them to the build.
foreach (vertex-shader ${vertex-shaders})

	string(REPLACE "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/" "" shader ${vertex-shader})
	add_shader(${PROJECT_NAME} ${shader})
endforeach (vertex-shader)
foreach (fragment-shader ${fragment-shaders})
	string(REPLACE "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/" "" shader ${fragment-shader})
	add_shader(${PROJECT_NAME} ${shader})
endforeach (fragment-shader)
foreach (compute-shader ${compute-shaders})
	string(REPLACE "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/" "" shader ${compute-shader})
	add_shader(${PROJECT_NAME} ${shader})
endforeach (compute-shader)
foreach (geometry-shader ${geometry-shaders})
	string(REPLACE "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/" "" shader ${geometry-shader})
	add_shader(${PROJECT_NAME} ${shader})
endforeach (geometry-shader)
foreach (tessellation-control-shader ${tessellation-control-shaders})
	string(REPLACE "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/" "" shader ${tessellation-control-shader})
	add_shader(${PROJECT_NAME} ${shader})
endforeach (tessellation-control-shader)
foreach (tessellation-evaluation-shader ${tessellation-evaluation-shaders})
	string(REPLACE "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/" "" shader ${tessellation-evaluation-shader})
	add_shader(${PROJECT_NAME} ${shader})
endforeach (tessellation-evaluation-shader)

message("Shaders: ${shaders}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)


if (CMAKE_CONFIGURATION_TYPES)
	string(TOLOWER "${CMAKE_CONFIGURATION_TYPES}" CMAKE_CONFIGURATION_TYPES_LOWER)
else ()
	string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_CONFIGURATION_TYPES_LOWER)
endif ()

foreach (config ${CMAKE_CONFIGURATION_TYPES_LOWER})
	if (${config} MATCHES "debug")
		# add preprocessor definition something like this bellow
		add_definitions(-DENABLE_VALIDATION_LAYERS)
	elseif (${config} MATCHES "release")
		# and so on...
	endif ()
endforeach ()
