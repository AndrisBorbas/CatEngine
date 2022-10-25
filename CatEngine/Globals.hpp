#pragma once

#ifndef CATENGINE_GLOBALS_HPP
#define CATENGINE_GLOBALS_HPP

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#ifndef VULKAN_HPP_NO_NODISCARD_WARNINGS
#define VULKAN_HPP_NO_NODISCARD_WARNINGS
#endif // VULKAN_HPP_NO_NODISCARD_WARNINGS

#ifndef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#endif // VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif // GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#ifndef GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif // GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.hpp>

#include <json.hpp>

#include "Cat/Utils/CatUtils.hpp"

// for convenience
using json = nlohmann::json;

using id_t = unsigned long long;

static constexpr vk::PresentModeKHR SELECTED_PRESENTMODE = vk::PresentModeKHR::eImmediate;
static constexpr char SELECTED_PRESENTMODE_TEXT[] = "Immediate";
static constexpr float ID_MX[16] = { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f };
static constexpr char LEVELS_BASE_PATH[] = "assets/levels/";


#endif // CATENGINE_GLOBALS_HPP
