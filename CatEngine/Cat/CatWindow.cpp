#include "CatWindow.hpp"

namespace cat
{
CatWindow::CatWindow( int iWidth, int iHeight, std::string sWindowName )
	: m_iWidth{ iWidth }, m_iHeight{ iHeight }, m_sWindowName{ sWindowName }
{
	initWindow();
}

CatWindow::~CatWindow()
{
	glfwDestroyWindow( m_pWindow );
	glfwTerminate();
}

void CatWindow::initWindow()
{
	glfwInit();
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );

	m_pWindow = glfwCreateWindow( m_iWidth, m_iHeight, m_sWindowName.c_str(), nullptr, nullptr );
	glfwSetWindowUserPointer( m_pWindow, this );
	glfwSetFramebufferSizeCallback( m_pWindow, frameBufferResizeCallback );
}

void CatWindow::createWindowSurface( vk::Instance& instance, vk::SurfaceKHR* pSurface )
{
	VkSurfaceKHR tempSurface;
	if ( glfwCreateWindowSurface( instance, m_pWindow, nullptr, &tempSurface ) != VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to create window surface!" );
	}
	pSurface = new vk::SurfaceKHR( tempSurface );
}

void CatWindow::frameBufferResizeCallback( GLFWwindow* pWindow, int iWidth, int iHeight )
{
	auto catWindow = reinterpret_cast< CatWindow* >( glfwGetWindowUserPointer( pWindow ) );
	catWindow->m_bFramebufferResized = true;
	catWindow->m_iWidth = iWidth;
	catWindow->m_iHeight = iHeight;
}

} // namespace cat
