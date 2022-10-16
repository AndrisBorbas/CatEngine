#include "CatWindow.hpp"

#include <utility>
#include "loguru.hpp"

namespace cat
{
CatWindow::CatWindow( const int iWidth, const int iHeight, std::string sWindowName, bool bIsFullscreen )
	: m_sWindowName{ std::move( sWindowName ) },
	  m_iWidth{ iWidth },
	  m_iHeight{ iHeight },
	  m_bIsFullscreen{ bIsFullscreen },
	  m_iLastWindowWidth{ iWidth },
	  m_iLastWindowHeight{ iHeight }
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

	m_pMonitor = glfwGetPrimaryMonitor();


	if ( m_bIsFullscreen )
	{
		const GLFWvidmode* mode = glfwGetVideoMode( m_pMonitor );
		glfwWindowHint( GLFW_RED_BITS, mode->redBits );
		glfwWindowHint( GLFW_GREEN_BITS, mode->greenBits );
		glfwWindowHint( GLFW_BLUE_BITS, mode->blueBits );
		glfwWindowHint( GLFW_REFRESH_RATE, mode->refreshRate );
		m_pWindow = glfwCreateWindow( m_iWidth, m_iHeight, m_sWindowName.c_str(), m_pMonitor, nullptr );
	}
	else
	{
		m_pWindow = glfwCreateWindow( m_iWidth, m_iHeight, m_sWindowName.c_str(), nullptr, nullptr );
	}

	glfwSetWindowUserPointer( m_pWindow, this );
	// glfwSetCursorPosCallback( m_pWindow, mouseCallback );
	// glfwSetInputMode( m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
	glfwSetFramebufferSizeCallback( m_pWindow, frameBufferResizeCallback );
}

void CatWindow::toggleFullscreen()
{
	if ( m_bIsFullscreen )
	{
		glfwWindowHint( GLFW_RED_BITS, GLFW_DONT_CARE );
		glfwWindowHint( GLFW_GREEN_BITS, GLFW_DONT_CARE );
		glfwWindowHint( GLFW_BLUE_BITS, GLFW_DONT_CARE );
		glfwWindowHint( GLFW_REFRESH_RATE, GLFW_DONT_CARE );
		glfwSetWindowMonitor(
			m_pWindow, nullptr, m_iLastWindowX, m_iLastWindowY, m_iLastWindowWidth, m_iLastWindowHeight, GLFW_DONT_CARE );
		m_bIsFullscreen = false;
	}
	else
	{
		m_iLastWindowWidth = m_iWidth;
		m_iLastWindowHeight = m_iHeight;
		glfwGetWindowPos( m_pWindow, &m_iLastWindowX, &m_iLastWindowY );
		const GLFWvidmode* mode = glfwGetVideoMode( m_pMonitor );
		glfwWindowHint( GLFW_RED_BITS, mode->redBits );
		glfwWindowHint( GLFW_GREEN_BITS, mode->greenBits );
		glfwWindowHint( GLFW_BLUE_BITS, mode->blueBits );
		glfwWindowHint( GLFW_REFRESH_RATE, mode->refreshRate );
		glfwSetWindowMonitor( m_pWindow, m_pMonitor, 0, 0, mode->width, mode->height, mode->refreshRate );
		m_bIsFullscreen = true;
	}
}

void CatWindow::createWindowSurface( vk::Instance instance, VkSurfaceKHR* pSurface )
{
	if ( glfwCreateWindowSurface( instance, m_pWindow, nullptr, pSurface ) != VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to create window surface!" );
	}
}

void CatWindow::frameBufferResizeCallback( GLFWwindow* pWindow, int iWidth, int iHeight )
{
	auto catWindow = reinterpret_cast< CatWindow* >( glfwGetWindowUserPointer( pWindow ) );
	catWindow->m_bFramebufferResized = true;
	catWindow->m_iWidth = iWidth;
	catWindow->m_iHeight = iHeight;
}

} // namespace cat
