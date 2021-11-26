#include "CatApp.hpp"

#include "Cat/Objects/CatInput.hpp"
#include "Cat/Rendering/CatBuffer.hpp"
#include "Cat/Objects/CatCamera.hpp"
#include "CatRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "CatImGui.hpp"

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

CatApp::~CatApp()
{
}

void CatApp::run()
{
	std::vector< std::unique_ptr< CatBuffer > > uboBuffers( CatSwapChain::MAX_FRAMES_IN_FLIGHT );
	for ( int i = 0; i < uboBuffers.size(); i++ )
	{
		uboBuffers[i] = std::make_unique< CatBuffer >( m_device, sizeof( GlobalUbo ), 1,
			vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible );
		uboBuffers[i]->map();
	}

	auto globalSetLayout = CatDescriptorSetLayout::Builder( m_device )
							   .addBinding( 0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex )
							   .build();

	std::vector< vk::DescriptorSet > globalDescriptorSets( CatSwapChain::MAX_FRAMES_IN_FLIGHT );
	for ( int i = 0; i < globalDescriptorSets.size(); i++ )
	{
		auto bufferInfo = uboBuffers[i]->descriptorInfo();
		CatDescriptorWriter( *globalSetLayout, *m_pGlobalPool ).writeBuffer( 0, &bufferInfo ).build( globalDescriptorSets[i] );
	}

	CatImGui catImGui{ m_window, m_device, m_renderer.getSwapChainRenderPass(), m_renderer.getImageCount() };

	CatRenderSystem simpleRenderSystem{
		m_device, m_renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
	CatCamera camera{};

	auto viewerObject = CatObject::createObject();
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
		camera.setPerspectiveProjection( glm::radians( 50.f ), aspect, 0.1f, 100.f );

		if ( auto commandBuffer = m_renderer.beginFrame() )
		{
			uint64_t frameIndex = m_renderer.getFrameIndex();
			CatFrameInfo frameInfo{
				frameIndex,
				frameTime,
				commandBuffer,
				camera,
				globalDescriptorSets[frameIndex],
			};

			// update
			GlobalUbo ubo{};
			ubo.projectionView = camera.getProjection() * camera.getView();
			uboBuffers[frameIndex]->writeToBuffer( &ubo );
			uboBuffers[frameIndex]->flush();

			// tell imgui that we're starting a new frame
			catImGui.newFrame();

			// start new frame
			m_renderer.beginSwapChainRenderPass( commandBuffer );

			// render game objects first, so they will be rendered in the background. This
			// is the best we can do for now.
			// Once we cover offscreen rendering, we can render the scene to a image/texture rather than
			// directly to the swap chain. This texture of the scene can then be rendered to an imgui
			// subwindow
			simpleRenderSystem.renderObjects( frameInfo, m_aObjects );

			// example code telling imgui what windows to render, and their contents
			// this can be replaced with whatever code/classes you set up configuring your
			// desired engine UI
			catImGui.runExample();

			// as last step in render pass, record the imgui draw commands
			catImGui.render( commandBuffer );

			m_renderer.endSwapChainRenderPass( commandBuffer );
			m_renderer.endFrame();
		}
	}

	vkDeviceWaitIdle( m_device.getDevice() );
}

void CatApp::loadGameObjects()
{
	std::shared_ptr< CatModel > lveModel = CatModel::createModelFromFile( m_device, "assets/models/flat_vase.obj" );
	auto flatVase = CatObject::createObject();
	flatVase.m_pModel = lveModel;
	flatVase.m_transform.translation = { -.5f, .5f, 0.f };
	flatVase.m_transform.scale = { 3.f, 1.5f, 3.f };
	m_aObjects.push_back( std::move( flatVase ) );

	lveModel = CatModel::createModelFromFile( m_device, "assets/models/smooth_vase.obj" );
	auto smoothVase = CatObject::createObject();
	smoothVase.m_pModel = lveModel;
	smoothVase.m_transform.translation = { .5f, .5f, 0.f };
	smoothVase.m_transform.scale = { 3.f, 1.5f, 3.f };
	m_aObjects.push_back( std::move( smoothVase ) );

	lveModel = CatModel::createModelFromFile( m_device, "assets/models/quad.obj" );
	auto floor = CatObject::createObject();
	floor.m_pModel = lveModel;
	floor.m_transform.translation = { 0.f, .5f, 0.f };
	floor.m_transform.scale = { 3.f, 1.f, 3.f };
	m_aObjects.push_back( std::move( floor ) );
}

} // namespace cat
