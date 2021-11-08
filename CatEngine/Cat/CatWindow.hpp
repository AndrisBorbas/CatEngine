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
	int m_nWidth;
	int m_nHeight;
	bool m_bFramebufferResized;

	void initWindow();
	static void frameBufferResizeCallback( GLFWwindow* pWindow, int nWidth, int nHeight );

public:
	CatWindow( int nWidth, int nHeight, std::string sWindowName );
	~CatWindow();

	CatWindow( const CatWindow& ) = delete;
	CatWindow& operator=( const CatWindow& ) = delete;
	CatWindow( CatWindow&& ) = delete;
	CatWindow& operator=( CatWindow&& ) = delete;


	bool shouldClose() const { return glfwWindowShouldClose( m_pWindow ); }
	vk::Extent2D getExtent() const { return { static_cast< uint32_t >( m_nWidth ), static_cast< uint32_t >( m_nHeight ) }; }
	bool wasWindowResized() const { return m_bFramebufferResized; }
	void resetWindowResizedFlag() { m_bFramebufferResized = false; }
	GLFWwindow* getGLFWwindow() const { return m_pWindow; }

	vk::SurfaceKHR* createWindowSurface( const vk::Instance& rInstance, vk::SurfaceKHR* pSurface ) const;
};
} // namespace cat
