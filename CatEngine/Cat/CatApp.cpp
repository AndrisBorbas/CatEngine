#include "CatApp.hpp"

#include "Cat/Objects/CatInput.hpp"
#include "Cat/Rendering/CatBuffer.hpp"
#include "Cat/Objects/CatCamera.hpp"
#include "CatRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
struct GlobalUbo
{
	glm::mat4 projectionView{ 1.f };
	glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f }; // w is intensity
	glm::vec3 lightPosition{ -1.f };
	alignas( 16 ) glm::vec4 lightColor{ 1.f }; // w is light intensity
};
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

	auto viewerObject = CatObject::createObject();
	viewerObject.m_transform.translation.y = -1.5f;
	viewerObject.m_transform.translation.z = -2.5f;
	CatInput cameraController{};

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
		camera.setPerspectiveProjection( glm::radians( 70.f ), aspect, 0.1f, 100.f );

		if ( auto commandBuffer = m_renderer.beginFrame() )
		{
			uint64_t frameIndex = m_renderer.getFrameIndex();
			CatFrameInfo frameInfo{
				frameIndex,
				frameTime,
				commandBuffer,
				camera,
				globalDescriptorSets[frameIndex],
				m_mObjects,
			};

			// update
			GlobalUbo ubo{};
			ubo.projectionView = camera.getProjection() * camera.getView();
			uboBuffers[frameIndex]->writeToBuffer( &ubo );
			uboBuffers[frameIndex]->flush();

			// tell imgui that we're starting a new frame
			CatImgui::newFrame();

			// start new frame
			m_renderer.beginSwapChainRenderPass( commandBuffer );

			// render game objects first, so they will be rendered in the background. This
			// is the best we can do for now.
			// Once we cover offscreen rendering, we can render the scene to a image/texture rather than
			// directly to the swap chain. This texture of the scene can then be rendered to an imgui
			// subwindow
			simpleRenderSystem.renderObjects( frameInfo );

			static constexpr float identityMatrix[16] = {
				1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f };

			ImGuizmo::Enable( true );

			// ImGuizmo::SetDrawlist( ImGui::GetBackgroundDrawList() );

			ImGuiIO& io = ImGui::GetIO();

			ImGuizmo::SetRect( 0, 0, io.DisplaySize.x, io.DisplaySize.y );

			float asd[16];
			ImGuizmo::RecomposeMatrixFromComponents( glm::value_ptr( m_mObjects.at( 1 ).m_transform.translation ),
				glm::value_ptr( m_mObjects.at( 1 ).m_transform.rotation ),
				glm::value_ptr( m_mObjects.at( 1 ).m_transform.scale ), asd );

			ImGuizmo::Manipulate( glm::value_ptr( camera.getView() ), glm::value_ptr( camera.getProjection() ),
				ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, asd, nullptr, nullptr );

			ImGuizmo::DecomposeMatrixToComponents( asd, glm::value_ptr( m_mObjects.at( 1 ).m_transform.translation ),
				glm::value_ptr( m_mObjects.at( 1 ).m_transform.rotation ),
				glm::value_ptr( m_mObjects.at( 1 ).m_transform.scale ) );

			ImGuizmo::DrawGrid(
				glm::value_ptr( camera.getView() ), glm::value_ptr( camera.getProjection() ), identityMatrix, 16.f );

			// example code telling imgui what windows to render, and their contents
			// this can be replaced with whatever code/classes you set up configuring your
			// desired engine UI
			imgui.runExample( viewerObject.m_transform.translation, viewerObject.m_transform.rotation );

			imgui.drawDebug( camera.getView(), camera.getProjection() );

			// as last step in render pass, record the imgui draw commands
			imgui.render( commandBuffer );

			m_renderer.endSwapChainRenderPass( commandBuffer );
			m_renderer.endFrame();
		}

		// Update and Render additional Platform Windows
		CatImgui::renderPlatforWindows();
	}

	vkDeviceWaitIdle( m_device.getDevice() );
}

void CatApp::loadGameObjects()
{
	std::shared_ptr< CatModel > model = CatModel::createModelFromFile( m_device, "assets/models/quad.obj" );
	auto floor = CatObject::createObject();
	floor.m_pModel = model;
	floor.m_transform.translation = { 0.f, 0.f, 0.f };
	floor.m_transform.scale = { 5.f, 1.f, 5.f };
	m_mObjects.emplace( floor.getId(), std::move( floor ) );

	model = CatModel::createModelFromFile( m_device, "assets/models/flat_vase.obj" );
	auto flatVase = CatObject::createObject();
	flatVase.m_pModel = model;
	flatVase.m_transform.translation = { -.5f, -.5f, 0.f };
	flatVase.m_transform.scale = { 3.f, 1.5f, 3.f };
	m_mObjects.emplace( flatVase.getId(), std::move( flatVase ) );

	model = CatModel::createModelFromFile( m_device, "assets/models/smooth_vase.obj" );
	auto smoothVase = CatObject::createObject();
	smoothVase.m_pModel = model;
	smoothVase.m_transform.translation = { .5f, -.5f, 0.f };
	smoothVase.m_transform.scale = { 3.f, 1.5f, 3.f };
	m_mObjects.emplace( smoothVase.getId(), std::move( smoothVase ) );

	model = CatModel::createModelFromFile( m_device, "assets/models/colored_cube.obj" );
	auto cube = CatObject::createObject();
	cube.m_pModel = model;
	cube.m_transform.translation = { 1.5f, -.5f, 0.f };
	cube.m_transform.scale = { .25f, .25f, .25f };
	m_mObjects.emplace( cube.getId(), std::move( cube ) );
}

} // namespace cat
