#ifndef CATENGINE_CATIMGUI_HPP
#define CATENGINE_CATIMGUI_HPP

#include "Cat/VulkanRHI/CatDevice.hpp"
#include "Cat/CatWindow.hpp"
#include "Cat/VulkanRHI/CatDescriptors.hpp"
#include "CatFrameInfo.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <queue>

#include <glm/vec3.hpp>

#include <stdexcept>

namespace cat
{
static void check_vk_result( VkResult err )
{
	if ( err == 0 ) return;
	fprintf( stderr, "[vulkan] Error: VkResult = %d\n", err );
	if ( err < 0 ) abort();
}

struct CatScrollingBuffer
{
	int m_nMaxSize;
	int m_nOffset;
	ImVector< ImVec2 > m_vData;
	CatScrollingBuffer( int nMaxSize = 1000 )
	{
		m_nMaxSize = nMaxSize;
		m_nOffset = 0;
		m_vData.reserve( m_nMaxSize );
	}
	void add( float x, float y )
	{
		if ( m_vData.size() < m_nMaxSize )
			m_vData.push_back( ImVec2( x, y ) );
		else
		{
			m_vData[m_nOffset] = ImVec2( x, y );
			m_nOffset = ( m_nOffset + 1 ) % m_nMaxSize;
		}
	}
	void erase()
	{
		if ( m_vData.size() > 0 )
		{
			m_vData.shrink( 0 );
			m_nOffset = 0;
		}
	}
};

class CatImgui
{
public:
	CatImgui( CatWindow* pWindow, CatDevice* pDevice, vk::RenderPass renderPass, size_t imageCount );
	~CatImgui();

	static void newFrame();

	static void render( vk::CommandBuffer commandBuffer );
	static void renderPlatformWindows();

	// Example state
	bool m_bShowDemoWindow = false;
	bool m_bShowDebugWindow = false;
	ImVec4 m_vClearColor = ImVec4( 0.45f, 0.55f, 0.60f, 1.00f );
	static void createDockSpace();
	void drawWindows();

	void drawDebug( glm::mat4 mx1, glm::mat4 mx2 );

private:
	CatWindow* m_pWindow;
	CatDevice* m_pDevice;

	std::unique_ptr< CatDescriptorPool > m_pDescriptorPool;

	CatScrollingBuffer m_vFrameTimes{ 1 << 12 };
	CatScrollingBuffer m_vFrameRates{ 1 << 12 };

	const size_t m_nQueueSize = 3000;
};
} // namespace cat

#endif // CATENGINE_CATIMGUI_HPP
