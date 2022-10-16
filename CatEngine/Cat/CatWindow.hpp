#ifndef CATENGINE_CATWINDOW_HPP
#define CATENGINE_CATWINDOW_HPP

#include "Globals.hpp"

#include <string>

namespace cat
{
class CatWindow
{
public:
	CatWindow( int iWidth, int iHeight, std::string sWindowName, bool bIsFullscreen = false );
	~CatWindow();

	CatWindow( const CatWindow& ) = delete;
	CatWindow& operator=( const CatWindow& ) = delete;

	[[nodiscard]] bool shouldClose() { return glfwWindowShouldClose( m_pWindow ); }

	[[nodiscard]] vk::Extent2D getExtent() const
	{
		return { static_cast< uint32_t >( m_iWidth ), static_cast< uint32_t >( m_iHeight ) };
	}

	[[nodiscard]] bool wasWindowResized() const { return m_bFramebufferResized; }
	void resetWindowResizedFlag() { m_bFramebufferResized = false; }
	[[nodiscard]] GLFWwindow* getGLFWwindow() const { return m_pWindow; }

	void toggleFullscreen();
	void createWindowSurface( vk::Instance instance, VkSurfaceKHR* pSurface );

private:
	GLFWwindow* m_pWindow = nullptr;
	GLFWmonitor* m_pMonitor = nullptr;
	std::string m_sWindowName;
	int m_iWidth;
	int m_iHeight;
	int m_iLastWindowWidth;
	int m_iLastWindowHeight;
	int m_iLastWindowX = 1;
	int m_iLastWindowY = 31;
	bool m_bFramebufferResized = false;
	bool m_bIsFullscreen = false;

	void initWindow();
	static void frameBufferResizeCallback( GLFWwindow* pWindow, int iWidth, int iHeight );
};
} // namespace cat

#endif // CATENGINE_CATWINDOW_HPP
