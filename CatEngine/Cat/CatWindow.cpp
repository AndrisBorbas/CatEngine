#include "CatWindow.hpp"

#include <utility>
#include <iostream>

namespace cat
{
CatWindow::CatWindow( int nWidth, int nHeight, std::string sWindowName )
	: m_sWindowName{ std::move( sWindowName ) }, m_nWidth{ nWidth }, m_nHeight{ nHeight }
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

	m_pWindow = glfwCreateWindow( m_nWidth, m_nHeight, m_sWindowName.c_str(), nullptr, nullptr );
	glfwSetWindowUserPointer( m_pWindow, this );
	glfwSetFramebufferSizeCallback( m_pWindow, frameBufferResizeCallback );
}

vk::SurfaceKHR* CatWindow::createWindowSurface( const vk::Instance& rInstance, vk::SurfaceKHR* pSurface ) const
{
	VkSurfaceKHR tempSurface;
	if ( glfwCreateWindowSurface( rInstance, m_pWindow, nullptr, &tempSurface ) != VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to create window surface!" );
	}
	pSurface = new vk::SurfaceKHR( tempSurface );
}

void CatWindow::frameBufferResizeCallback( GLFWwindow* pWindow, int nWidth, int nHeight )
{
	const auto catWindow = static_cast< CatWindow* >( glfwGetWindowUserPointer( pWindow ) );
	catWindow->m_bFramebufferResized = true;
	catWindow->m_nWidth = nWidth;
	catWindow->m_nHeight = nHeight;
}

} // namespace cat
