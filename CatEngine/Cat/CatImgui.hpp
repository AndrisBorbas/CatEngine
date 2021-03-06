#ifndef CATENGINE_CATIMGUI_HPP
#define CATENGINE_CATIMGUI_HPP

#include "Cat/Rendering/CatDevice.hpp"
#include "Cat/CatWindow.hpp"
#include "Cat/Rendering/CatDescriptors.hpp"
#include "Cat/CatApp.hpp"
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
	CatImgui( CatApp& app, CatWindow& window, CatDevice& device, vk::RenderPass renderPass, size_t imageCount );
	~CatImgui();

	static void newFrame();

	void render( vk::CommandBuffer commandBuffer );
	static void renderPlatformWindows();

	// Example state
	bool m_bShowDemoWindow = false;
	bool m_bShowDebugWindow = false;
	ImVec4 m_vClearColor = ImVec4( 0.45f, 0.55f, 0.60f, 1.00f );
	void drawWindows();

	void drawDebug( const glm::mat4 mView, const glm::mat4 mProj );

private:
	CatDevice& m_rDevice;
	CatWindow& m_rWindow;
	CatApp& m_rApp;

	std::unique_ptr< CatDescriptorPool > m_pDescriptorPool;

	std::list< double > m_qFrameTimes{ 0.0 };

	const size_t m_nQueueSize = 100;
};
} // namespace cat

#endif // CATENGINE_CATIMGUI_HPP
