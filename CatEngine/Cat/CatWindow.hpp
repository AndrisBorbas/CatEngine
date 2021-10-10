#pragma once

#include "Defines.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <string>

namespace cat
{
class CatWindow
{
private:
	GLFWwindow* m_pWindow;
	std::string m_sWindowName;
	int m_iWidth;
	int m_iHeight;
	bool m_bFramebufferResized;
	void initWindow();
	static void frameBufferResizeCallback( GLFWwindow* pWindow, int iWidth, int iHeight );

public:
	CatWindow( int iWidth, int iHeight, std::string sWindowName );
	~CatWindow();

	CatWindow( const CatWindow& ) = delete;
	CatWindow& operator=( const CatWindow& ) = delete;

	bool shouldClose() { return glfwWindowShouldClose( m_pWindow ); }
	vk::Extent2D getExtent() { return { static_cast< uint32_t >( m_iWidth ), static_cast< uint32_t >( m_iHeight ) }; }
	bool wasWindowResized() { return m_bFramebufferResized; }
	void resetWindowResizedFlag() { m_bFramebufferResized = false; }
	GLFWwindow* getGLFWwindow() const { return m_pWindow; }

	void createWindowSurface( vk::Instance& instance, vk::SurfaceKHR* surface );
};
} // namespace cat