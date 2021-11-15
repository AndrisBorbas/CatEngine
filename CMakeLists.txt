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

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(Libraries)

include_directories(CatEngine)

add_executable(CatEngine CatEngine/main.cpp CatEngine/Shaders.hpp CatEngine/Shaders.cpp CatEngine/utils.hpp CatEngine/Vulkan/VulkanDevices.hpp CatEngine/Vulkan/VulkanInit.hpp CatEngine/Vulkan/VulkanRendering.hpp CatEngine/Vulkan/VulkanRendering.cpp CatEngine/Vulkan/VulkanInit.cpp CatEngine/Defines.h CatEngine/Vulkan/VulkanDevices.cpp "CatEngine/Cat/CatWindow.hpp" "CatEngine/Cat/CatWindow.cpp" CatEngine/Cat/CatCamera.cpp CatEngine/Cat/CatCamera.hpp CatEngine/Cat/CatInput.cpp CatEngine/Cat/CatInput.hpp CatEngine/Cat/CatObject.cpp CatEngine/Cat/CatObject.hpp CatEngine/Cat/CatModel.cpp CatEngine/Cat/CatModel.hpp CatEngine/Cat/CatDevice.cpp CatEngine/Cat/CatDevice.hpp CatEngine/Cat/CatUtils.hpp CatEngine/Cat/CatApp.cpp CatEngine/Cat/CatApp.hpp CatEngine/Cat/CatBuffer.cpp CatEngine/Cat/CatBuffer.hpp CatEngine/Cat/CatDescriptors.cpp CatEngine/Cat/CatDescriptors.hpp CatEngine/Cat/CatFrameInfo.hpp CatEngine/Cat/CatPipeline.cpp CatEngine/Cat/CatPipeline.hpp CatEngine/Cat/CatRenderer.cpp CatEngine/Cat/CatRenderer.hpp CatEngine/Cat/CatSwapChain.cpp CatEngine/Cat/CatSwapChain.hpp CatEngine/Cat/CatRenderSystem.cpp CatEngine/Cat/CatRenderSystem.hpp CatEngine/Globals.hpp)

####Vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} glm tinyobjloader stb_image loguru imgui)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw)


add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

include(Shaders.cmake)

# Find all shaders.
file(GLOB vertex-shaders ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/*.vert)
file(GLOB fragment-shaders ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/*.frag)
file(GLOB compute-shaders ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/*.comp)

# Add them to the build.
foreach (vertex-shader ${vertex-shaders})
    get_filename_component(p ${vertex-shader} NAME)
    add_shader(${PROJECT_NAME} ${p})
endforeach (vertex-shader)

foreach (fragment-shader ${fragment-shaders})
    get_filename_component(p ${fragment-shader} NAME)
    add_shader(${PROJECT_NAME} ${p})
endforeach (fragment-shader)

foreach (compute-shader ${compute-shaders})
    get_filename_component(p ${compute-shader} NAME)
    add_shader(${PROJECT_NAME} ${p})
endforeach (compute-shader)

set(output-assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/${TARGET}/assets)

if (CMAKE_CONFIGURATION_TYPES)
    string(TOLOWER "${CMAKE_CONFIGURATION_TYPES}" CMAKE_CONFIGURATION_TYPES_LOWER)
else ()
    string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_CONFIGURATION_TYPES_LOWER)
endif ()

foreach (config ${CMAKE_CONFIGURATION_TYPES_LOWER})
    if (${config} MATCHES "debug")
        #add preprocessor definition something like this bellow
        add_definitions(-DENABLE_VALIDATION_LAYERS)
    elseif (${config} MATCHES "release")
        #and so on...
    endif ()
endforeach ()
