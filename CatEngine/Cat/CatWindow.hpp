#ifndef CATENGINE_CATWINDOW_HPP
#define CATENGINE_CATWINDOW_HPP

#include "Globals.hpp"

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

#endif // CATENGINE_CATWINDOW_HPP
