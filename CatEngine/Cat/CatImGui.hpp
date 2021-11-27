#ifndef CATENGINE_CATIMGUI_HPP
#define CATENGINE_CATIMGUI_HPP

#include "Cat/Rendering/CatDevice.hpp"
#include "Cat/CatWindow.hpp"
#include "Cat/Rendering/CatDescriptors.hpp"


#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

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

class CatImGui
{
public:
	CatImGui( CatWindow& window, CatDevice& device, vk::RenderPass renderPass, uint32_t imageCount );
	~CatImGui();

	void newFrame();

	void render( vk::CommandBuffer commandBuffer );

	// Example state
	bool m_bShowDemoWindow = false;
	bool m_bShowAnotherWindow = false;
	ImVec4 m_vClearColor = ImVec4( 0.45f, 0.55f, 0.60f, 1.00f );
	void runExample( glm::vec3 vCameraPos, glm::vec3 vCameraRot );

private:
	CatDevice& m_rDevice;
	CatWindow& m_rWindow;

	std::unique_ptr< CatDescriptorPool > m_pDescriptorPool;
};

} // namespace cat

#endif // CATENGINE_CATIMGUI_HPP
