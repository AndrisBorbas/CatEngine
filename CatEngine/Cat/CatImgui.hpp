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

	std::list< double > m_qFrameTimes{ 0.0 };

	const size_t m_nQueueSize = 100;
};
} // namespace cat

#endif // CATENGINE_CATIMGUI_HPP
