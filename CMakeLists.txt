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

add_executable(CatEngine CatEngine/main.cpp CatEngine/Cat/CatWindow.hpp CatEngine/Cat/CatWindow.cpp CatEngine/Cat/Controller/CatCamera.cpp CatEngine/Cat/Controller/CatCamera.hpp CatEngine/Cat/Controller/CatInput.cpp CatEngine/Cat/Controller/CatInput.hpp CatEngine/Cat/Objects/CatObject.cpp CatEngine/Cat/Objects/CatObject.hpp CatEngine/Cat/Objects/CatModel.cpp CatEngine/Cat/Objects/CatModel.hpp CatEngine/Cat/Rendering/CatDevice.cpp CatEngine/Cat/Rendering/CatDevice.hpp CatEngine/Cat/Utils/CatUtils.hpp CatEngine/Cat/CatApp.cpp CatEngine/Cat/CatApp.hpp CatEngine/Cat/Rendering/CatBuffer.cpp CatEngine/Cat/Rendering/CatBuffer.hpp CatEngine/Cat/Rendering/CatDescriptors.cpp CatEngine/Cat/Rendering/CatDescriptors.hpp CatEngine/Cat/CatFrameInfo.hpp CatEngine/Cat/Rendering/CatPipeline.cpp CatEngine/Cat/Rendering/CatPipeline.hpp CatEngine/Cat/Rendering/CatRenderer.cpp CatEngine/Cat/Rendering/CatRenderer.hpp CatEngine/Cat/Rendering/CatSwapChain.cpp CatEngine/Cat/Rendering/CatSwapChain.hpp CatEngine/Cat/RenderSystems/CatSimpleRenderSystem.cpp CatEngine/Cat/RenderSystems/CatSimpleRenderSystem.hpp CatEngine/Globals.hpp CatEngine/Cat/CatImGui.cpp CatEngine/Cat/CatImGui.hpp CatEngine/Cat/RenderSystems/CatPointLightRenderSystem.hpp CatEngine/Cat/RenderSystems/CatPointLightRenderSystem.cpp CatEngine/Cat/RenderSystems/CatWireframeRenderSystem.cpp CatEngine/Cat/RenderSystems/CatWireframeRenderSystem.hpp CatEngine/Cat/Objects/CatVolume.cpp CatEngine/Cat/Objects/CatVolume.hpp CatEngine/Cat/Objects/CatLight.cpp CatEngine/Cat/Objects/CatLight.hpp CatEngine/Cat/RenderSystems/CatGridRenderSystem.cpp CatEngine/Cat/RenderSystems/CatGridRenderSystem.hpp)

# ###Vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} glm tinyobjloader stb_image loguru imgui ImGuizmo json)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw)

if (MSVC)
	# target_compile_options(${PROJECT_NAME} PUBLIC "/ZI")
	target_link_options(${PROJECT_NAME} PUBLIC "/INCREMENTAL")
endif ()

include(Shaders.cmake)

# Find all shaders.
file(GLOB vertex-shaders ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/*.vert)
file(GLOB fragment-shaders ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/*.frag)
file(GLOB compute-shaders ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/*.comp)

# Add them to the build.
foreach (vertex-shader ${vertex-shaders})
	get_filename_component(shader ${vertex-shader} NAME)
	add_shader(${PROJECT_NAME} ${shader})
endforeach (vertex-shader)

foreach (fragment-shader ${fragment-shaders})
	get_filename_component(shader ${fragment-shader} NAME)
	add_shader(${PROJECT_NAME} ${shader})
endforeach (fragment-shader)

foreach (compute-shader ${compute-shaders})
	get_filename_component(shader ${compute-shader} NAME)
	add_shader(${PROJECT_NAME} ${shader})
endforeach (compute-shader)

set(output-assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/${TARGET}/assets)

add_custom_target(shader-target ALL)
message("Shaders: ${shaders}")
add_dependencies(shader-target ${shaders})
add_dependencies(${PROJECT_NAME} shader-target)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

add_custom_command(TARGET shader-target POST_BUILD
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
