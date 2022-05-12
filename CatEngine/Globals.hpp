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

// for convenience
using json = nlohmann::json;


static constexpr vk::PresentModeKHR SELECTED_PRESENTMODE = vk::PresentModeKHR::eImmediate;
static constexpr char SELECTED_PRESENTMODE_TEXT[] = "Immediate";
static constexpr float ID_MX[16] = { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f };

struct Lambda
{
	template < typename Tret, typename T >
	static Tret lambda_ptr_exec( void* data, int i )
	{
		return (Tret)( *(T*)fn< T >() )( data, i );
	}

	template < typename Tret = void, typename Tfp = Tret ( * )( void* ), typename T >
	static Tfp ptr( T& t )
	{
		fn< T >( &t );
		return (Tfp)lambda_ptr_exec< Tret, T >;
	}

	template < typename T >
	static void* fn( void* new_fn = nullptr )
	{
		static void* fn;
		if ( new_fn != nullptr ) fn = new_fn;
		return fn;
	}
};

#endif // CATENGINE_GLOBALS_HPP
