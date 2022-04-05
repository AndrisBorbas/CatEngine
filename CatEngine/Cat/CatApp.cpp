#include "CatApp.hpp"

#include "Cat/Objects/CatInput.hpp"
#include "Cat/Rendering/CatBuffer.hpp"
#include "Cat/Objects/CatCamera.hpp"
#include "CatRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/constants.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "CatImgui.hpp"

#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>

namespace cat
{

CatApp::CatApp()
{
	m_pGlobalPool = CatDescriptorPool::Builder( m_device )
						.setMaxSets( CatSwapChain::MAX_FRAMES_IN_FLIGHT )
						.addPoolSize( vk::DescriptorType::eUniformBuffer, CatSwapChain::MAX_FRAMES_IN_FLIGHT )
						.build();
	loadGameObjects();
}

CatApp::~CatApp() = default;

void CatApp::run()
{
	std::vector< std::unique_ptr< CatBuffer > > uboBuffers( CatSwapChain::MAX_FRAMES_IN_FLIGHT );
	for ( auto& uboBuffer : uboBuffers )
	{
		uboBuffer = std::make_unique< CatBuffer >( m_device, sizeof( GlobalUbo ), 1, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible );
		uboBuffer->map();
	}

	auto globalSetLayout = CatDescriptorSetLayout::Builder( m_device )
							   .addBinding( 0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex )
							   .build();

	std::vector< vk::DescriptorSet > globalDescriptorSets( CatSwapChain::MAX_FRAMES_IN_FLIGHT );
	for ( size_t i = 0; i < globalDescriptorSets.size(); i++ )
	{
		auto bufferInfo = uboBuffers[i]->descriptorInfo();
		CatDescriptorWriter( *globalSetLayout, *m_pGlobalPool ).writeBuffer( 0, &bufferInfo ).build( globalDescriptorSets[i] );
	}

	CatImgui imgui{ *this, m_window, m_device, m_renderer.getSwapChainRenderPass(), m_renderer.getImageCount() };

	CatRenderSystem simpleRenderSystem{
		m_device, m_renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
	CatCamera camera{};

	auto viewerObject = CatObject::createObject( "Camera" );
	viewerObject.m_transform.translation = { 0.f, 1.5f, 2.5f };
	CatInput cameraController{};

	GlobalUbo ubo{};

	CatFrameInfo frameInfo{
		nullptr,
		camera,
		globalDescriptorSets[0],
		ubo,
		m_mObjects,
	};

	auto currentTime = std::chrono::high_resolution_clock::now();
	while ( !m_window.shouldClose() )
	{
		glfwPollEvents();

		auto newTime = std::chrono::high_resolution_clock::now();
		float frameTime = std::chrono::duration< float, std::chrono::seconds::period >( newTime - currentTime ).count();
		currentTime = newTime;

		cameraController.moveInPlaneXZ( m_window.getGLFWwindow(), frameTime, viewerObject );
		camera.setViewYXZ( viewerObject.m_transform.translation, viewerObject.m_transform.rotation );

		float aspect = m_renderer.getAspectRatio();
		camera.setPerspectiveProjection( glm::radians( 50.f ), aspect, 0.1f, 100.f );

		auto imguizmoCamera = camera;
		imguizmoCamera.setPerspectiveProjectionRH( glm::radians( 50.f ), aspect, 0.1f, 100.f );
		imguizmoCamera.setViewYXZRH( viewerObject.m_transform.translation, viewerObject.m_transform.rotation );

		if ( auto commandBuffer = m_renderer.beginFrame() )
		{
			uint64_t frameIndex = m_renderer.getFrameIndex();

			frameInfo.update( commandBuffer, globalDescriptorSets[frameIndex], frameTime, frameIndex );

			ubo.projectionView = camera.getProjection() * camera.getView();
			uboBuffers[frameInfo.m_nFrameIndex]->writeToBuffer( &ubo );
			uboBuffers[frameInfo.m_nFrameIndex]->flush();

			// tell imgui that we're starting a new frame
			CatImgui::newFrame();

			// start new frame
			m_renderer.beginSwapChainRenderPass( commandBuffer );


			ImGuizmo::DrawGrid(
				glm::value_ptr( imguizmoCamera.getView() ), glm::value_ptr( imguizmoCamera.getProjection() ), ID_MX, 16.f );

			// render game objects first, so they will be rendered in the background. This
			// is the best we can do for now.
			// Once we cover offscreen rendering, we can render the scene to a image/texture rather than
			// directly to the swap chain. This texture of the scene can then be rendered to an imgui
			// subwindow
			simpleRenderSystem.renderObjects( frameInfo );

			ImGuizmo::Enable( true );
			ImGuizmo::SetDrawlist( ImGui::GetBackgroundDrawList() );
			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(
				ImGui::GetMainViewport()->Pos.x, ImGui::GetMainViewport()->Pos.y, io.DisplaySize.x, io.DisplaySize.y );

			glm::vec3 flush{ 1.f, 1.f, 1.f };
			float asd[16];
			ImGuizmo::RecomposeMatrixFromComponents(
				glm::value_ptr( m_mObjects.at( frameInfo.m_selectedItemId ).m_transform.translation ),
				glm::value_ptr( m_mObjects.at( frameInfo.m_selectedItemId ).m_transform.rotation * glm::pi< float >() / 180.f ),
				glm::value_ptr( m_mObjects.at( frameInfo.m_selectedItemId ).m_transform.scale ), asd );
			/*ImGuizmo::RecomposeMatrixFromComponents( glm::value_ptr( ubo.lightPosition ),
				glm::value_ptr( glm::vec3{ 0.f, 0.f, 0.f } ), glm::value_ptr( glm::vec3{ 1.f, 1.f, 1.f } ), asd );*/
			ImGuizmo::Manipulate( glm::value_ptr( imguizmoCamera.getView() ), glm::value_ptr( imguizmoCamera.getProjection() ),
				ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, asd, nullptr, nullptr );
			ImGuizmo::DecomposeMatrixToComponents( asd,
				glm::value_ptr( m_mObjects.at( frameInfo.m_selectedItemId ).m_transform.translation ),
				glm::value_ptr( m_mObjects.at( frameInfo.m_selectedItemId ).m_transform.rotation ),
				glm::value_ptr( m_mObjects.at( frameInfo.m_selectedItemId ).m_transform.scale ) );
			m_mObjects.at( 1 ).m_transform.rotation / glm::pi< float >() * 180.f;

			/*ImGuizmo::DecomposeMatrixToComponents(
				asd, glm::value_ptr( ubo.lightPosition ), glm::value_ptr( flush ), glm::value_ptr( flush ) );*/

			// example code telling imgui what windows to render, and their contents
			// this can be replaced with whatever code/classes you set up configuring your
			// desired engine UI
			imgui.drawWindows( frameInfo, viewerObject.m_transform.translation, viewerObject.m_transform.rotation );

			imgui.drawDebug( camera.getView(), imguizmoCamera.getView() );


			// as last step in render pass, record the imgui draw commands
			imgui.render( commandBuffer );

			m_renderer.endSwapChainRenderPass( commandBuffer );
			m_renderer.endFrame();
		}

		// Update and Render additional Platform Windows
		CatImgui::renderPlatformWindows();
	}

	vkDeviceWaitIdle( m_device.getDevice() );
}

void CatApp::loadGameObjects()
{
	std::shared_ptr< CatModel > model = CatModel::createModelFromFile( m_device, "assets/models/quad.obj" );
	auto floor = CatObject::createObject( "Floor" );
	floor.m_pModel = model;
	floor.m_transform.translation = { 0.f, 0.f, 0.f };
	floor.m_transform.scale = { 5.f, 1.f, 5.f };
	m_mObjects.emplace( floor.getId(), std::move( floor ) );

	model = CatModel::createModelFromFile( m_device, "assets/models/flat_vase.obj" );
	auto flatVase = CatObject::createObject( "FlatVase" );
	flatVase.m_pModel = model;
	flatVase.m_transform.translation = { -.5f, .5f, 0.f };
	flatVase.m_transform.scale = { 3.f, 1.5f, 3.f };
	m_mObjects.emplace( flatVase.getId(), std::move( flatVase ) );

	model = CatModel::createModelFromFile( m_device, "assets/models/smooth_vase.obj" );
	auto smoothVase = CatObject::createObject( "SmoothVase" );
	smoothVase.m_pModel = model;
	smoothVase.m_transform.translation = { .5f, .5f, 0.f };
	smoothVase.m_transform.scale = { 3.f, 1.5f, 3.f };
	m_mObjects.emplace( smoothVase.getId(), std::move( smoothVase ) );

	model = CatModel::createModelFromFile( m_device, "assets/models/colored_cube.obj" );
	auto cube = CatObject::createObject( "Cube" );
	cube.m_pModel = model;
	cube.m_transform.translation = { 1.5f, .5f, 0.f };
	cube.m_transform.scale = { .25f, .25f, .25f };
	m_mObjects.emplace( cube.getId(), std::move( cube ) );
}

} // namespace cat
