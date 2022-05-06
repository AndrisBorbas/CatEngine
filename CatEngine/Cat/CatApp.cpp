#include "CatApp.hpp"

#include "Cat/Objects/CatInput.hpp"
#include "Cat/Rendering/CatBuffer.hpp"
#include "Cat/Objects/CatCamera.hpp"
#include "Cat/RenderSystems/CatSimpleRenderSystem.hpp"
#include "Cat/RenderSystems/CatPointLightRenderSystem.hpp"

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
#include <fstream>
#include <stdexcept>

#include "loguru.hpp"

namespace cat
{
CatApp* GlobalEditorInstance = nullptr;

CatApp* GetEditorInstance()
{
	return GlobalEditorInstance;
}

void CreateEditorInstance()
{
	GlobalEditorInstance = new CatApp();
}

void DestroyGameInstance()
{
	if ( GlobalEditorInstance )
	{
		delete GlobalEditorInstance;
		GlobalEditorInstance = nullptr;
	}
}

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
							   .addBinding( 0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics )
							   .build();

	std::vector< vk::DescriptorSet > globalDescriptorSets( CatSwapChain::MAX_FRAMES_IN_FLIGHT );
	for ( size_t i = 0; i < globalDescriptorSets.size(); i++ )
	{
		auto bufferInfo = uboBuffers[i]->descriptorInfo();
		CatDescriptorWriter( *globalSetLayout, *m_pGlobalPool ).writeBuffer( 0, &bufferInfo ).build( globalDescriptorSets[i] );
	}

	CatImgui imgui{ *this, m_window, m_device, m_renderer.getSwapChainRenderPass(), m_renderer.getImageCount() };

	CatSimpleRenderSystem simpleRenderSystem{
		m_device, m_renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
	CatPointLightRenderSystem pointLightRenderSystem{
		m_device, m_renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };

	CatCamera camera{};
	auto viewerObject = CatObject::createObject( "Camera", "Camera" );
	viewerObject.m_transform.translation = { 0.f, 1.5f, 2.5f };
	CatInput cameraController{};

	GlobalUbo ubo{};

	m_pFrameInfo = std::make_unique< CatFrameInfo >( nullptr, camera, viewerObject, globalDescriptorSets[0], ubo, m_mObjects );

	auto currentTime = std::chrono::high_resolution_clock::now();
	while ( !m_window.shouldClose() )
	{
		glfwPollEvents();

		auto newTime = std::chrono::high_resolution_clock::now();
		float frameTime = std::chrono::duration< float, std::chrono::seconds::period >( newTime - currentTime ).count();
		currentTime = newTime;

		cameraController.moveInPlaneXZ( m_window.getGLFWwindow(), frameTime, getFrameInfo().m_rCameraObject );
		camera.setViewYXZ(
			getFrameInfo().m_rCameraObject.m_transform.translation, getFrameInfo().m_rCameraObject.m_transform.rotation );

		float aspect = m_renderer.getAspectRatio();
		camera.setPerspectiveProjection( glm::radians( 50.f ), aspect, 0.1f, 100.f );

		auto imguizmoCamera = camera;
		imguizmoCamera.setPerspectiveProjectionRH( glm::radians( 50.f ), aspect, 0.1f, 100.f );
		imguizmoCamera.setViewYXZRH(
			getFrameInfo().m_rCameraObject.m_transform.translation, getFrameInfo().m_rCameraObject.m_transform.rotation );

		if ( auto commandBuffer = m_renderer.beginFrame() )
		{
			short frameIndex = m_renderer.getFrameIndex();

			if ( m_jLevelLoad.valid() )
			{
				using namespace std::chrono_literals;
				auto status = m_jLevelLoad.wait_for( 1ns );
				if ( status == std::future_status::ready )
				{
					LOG_F( INFO, "Frame: %llu, level loaded", GetEditorInstance()->getFrameInfo().m_nFrameNumber );
					m_jLevelLoad = {};
				}
			}

			m_pFrameInfo->update(
				commandBuffer, globalDescriptorSets[frameIndex], frameTime, frameIndex, m_renderer.getFrameNumber() );

			ubo.projection = camera.getProjection();
			ubo.view = camera.getView();
			ubo.inverseView = camera.getInverseView();
			uboBuffers[getFrameInfo().m_nFrameIndex]->writeToBuffer( &ubo );
			uboBuffers[getFrameInfo().m_nFrameIndex]->flush();

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
			simpleRenderSystem.renderObjects( getFrameInfo() );
			pointLightRenderSystem.update( getFrameInfo(), ubo, false );
			pointLightRenderSystem.render( getFrameInfo() );

			ImGuizmo::Enable( true );
			ImGuizmo::SetDrawlist( ImGui::GetBackgroundDrawList() );
			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(
				ImGui::GetMainViewport()->Pos.x, ImGui::GetMainViewport()->Pos.y, io.DisplaySize.x, io.DisplaySize.y );

			// glm::vec3 flush{ 1.f, 1.f, 1.f };
			float mxManipulate[16];
			ImGuizmo::RecomposeMatrixFromComponents(
				glm::value_ptr( m_mObjects.at( getFrameInfo().m_selectedItemId ).m_transform.translation ),
				glm::value_ptr( m_mObjects.at( getFrameInfo().m_selectedItemId ).m_transform.rotation ),
				glm::value_ptr( m_mObjects.at( getFrameInfo().m_selectedItemId ).m_transform.scale ), mxManipulate );
			auto isManipulated = ImGuizmo::Manipulate( glm::value_ptr( imguizmoCamera.getView() ),
				glm::value_ptr( imguizmoCamera.getProjection() ), ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, mxManipulate, nullptr,
				nullptr );
			ImGuizmo::DecomposeMatrixToComponents( mxManipulate,
				glm::value_ptr( m_mObjects.at( getFrameInfo().m_selectedItemId ).m_transform.translation ),
				glm::value_ptr( m_mObjects.at( getFrameInfo().m_selectedItemId ).m_transform.rotation ),
				glm::value_ptr( m_mObjects.at( getFrameInfo().m_selectedItemId ).m_transform.scale ) );
			if ( isManipulated )
			{
				// m_mObjects.at( frameInfo.m_selectedItemId ).m_transform.rotation *= ( glm::pi< float >() * 180.f );
			}

			// example code telling imgui what windows to render, and their contents
			// this can be replaced with whatever code/classes you set up configuring your
			// desired engine UI
			imgui.drawWindows();

			imgui.drawDebug( camera.getView(), imguizmoCamera.getView() );


			// as last step in render pass, record the imgui draw commands
			imgui.render( commandBuffer );

			m_renderer.endSwapChainRenderPass( commandBuffer );
			m_renderer.endFrame();
		}

		// Update and Render additional Platform Windows
		CatImgui::renderPlatformWindows();
	}

	m_device.getDevice().waitIdle();
}

void CatApp::saveLevel( const std::string& sFileName ) const
{
	json file;
	json objects;
	int i = 0;
	for ( auto& [key, obj] : getFrameInfo().m_mObjects )
	{
		auto curr = std::to_string( obj.getId() ) /* + " : " + obj.getName() */;

		objects[i]["name"] = obj.getName();
		objects[i]["file"] = obj.getFileName();

		objects[i]["transform"]["t"] = obj.m_transform.translation;
		objects[i]["transform"]["r"] = obj.m_transform.rotation;
		objects[i]["transform"]["s"] = obj.m_transform.scale;

		objects[i]["color"] = obj.m_vColor;
		if ( obj.m_pPointLight != nullptr )
		{
			objects[i]["lightIntensity"] = obj.m_pPointLight->lightIntensity;
		}

		++i;
	}
	file["objects"] = objects;

	// LOG_F( INFO, file.dump( 2 ).c_str() );

	std::ofstream ofs( sFileName );
	ofs << std::setw( 2 ) << file << std::endl;
	ofs.close();
}

void CatApp::loadLevel( const std::string& sFileName, const bool bClearPrevious /* = true */ )
{
	LOG_F( INFO, "Frame: %llu", GetEditorInstance()->getFrameInfo().m_nFrameNumber );

	if ( bClearPrevious )
	{
		m_mObjects.clear();
	}

	json file;
	std::ifstream ifs( sFileName );
	ifs >> file;
	ifs.close();

	json objects;
	objects = file["objects"];
	for ( auto& object : objects )
	{
		if ( object["file"] == "Camera" )
		{
			continue;
		}
		else if ( object["file"].get< std::string >().starts_with( "Light" ) )
		{
			auto pointLight = CatObject::makePointLight( object["name"], object["lightIntensity"] );
			pointLight.m_transform.translation = glm::make_vec3( &object["transform"]["t"].get< std::vector< float > >()[0] );
			pointLight.m_transform.rotation = glm::make_vec3( &object["transform"]["r"].get< std::vector< float > >()[0] );
			pointLight.m_transform.scale = glm::make_vec3( &object["transform"]["s"].get< std::vector< float > >()[0] );
			pointLight.m_vColor = glm::make_vec3( &object["color"].get< std::vector< float > >()[0] );
			m_mObjects.emplace( pointLight.getId(), std::move( pointLight ) );
		}
		else
		{
			std::shared_ptr< CatModel > model = CatModel::createModelFromFile( m_device, object["file"] );
			auto obj = CatObject::createObject( object["name"], object["file"] );
			obj.m_pModel = model;
			obj.m_transform.translation = glm::make_vec3( &object["transform"]["t"].get< std::vector< float > >()[0] );
			obj.m_transform.rotation = glm::make_vec3( &object["transform"]["r"].get< std::vector< float > >()[0] );
			obj.m_transform.scale = glm::make_vec3( &object["transform"]["s"].get< std::vector< float > >()[0] );
			obj.m_vColor = glm::make_vec3( &object["color"].get< std::vector< float > >()[0] );
			m_mObjects.emplace( obj.getId(), std::move( obj ) );
		}
	}
}

void CatApp::loadGameObjects()
{
	std::shared_ptr< CatModel > model = CatModel::createModelFromFile( m_device, "assets/models/quad.obj" );
	auto floor = CatObject::createObject( "Floor", "assets/models/quad.obj" );
	floor.m_pModel = model;
	floor.m_transform.translation = { 0.f, 0.f, 0.f };
	floor.m_transform.scale = { 5.f, 1.f, 5.f };
	floor.m_transform.rotation = { glm::radians( 180.0f ), 0.0f, 0.0f };
	m_mObjects.emplace( floor.getId(), std::move( floor ) );

	model = CatModel::createModelFromFile( m_device, "assets/models/flat_vase.obj" );
	auto flatVase = CatObject::createObject( "FlatVase", "assets/models/flat_vase.obj" );
	flatVase.m_pModel = model;
	flatVase.m_transform.translation = { -.5f, .5f, 0.f };
	flatVase.m_transform.scale = { 3.f, 1.5f, 3.f };
	m_mObjects.emplace( flatVase.getId(), std::move( flatVase ) );

	model = CatModel::createModelFromFile( m_device, "assets/models/smooth_vase.obj" );
	auto smoothVase = CatObject::createObject( "SmoothVase", "assets/models/smooth_vase.obj" );
	smoothVase.m_pModel = model;
	smoothVase.m_transform.translation = { .5f, .5f, 0.f };
	smoothVase.m_transform.scale = { 3.f, 1.5f, 3.f };
	m_mObjects.emplace( smoothVase.getId(), std::move( smoothVase ) );

	model = CatModel::createModelFromFile( m_device, "assets/models/colored_cube.obj" );
	auto cube = CatObject::createObject( "Cube", "assets/models/colored_cube.obj" );
	cube.m_pModel = model;
	cube.m_transform.translation = { 1.5f, .5f, 0.f };
	cube.m_transform.scale = { .25f, .25f, .25f };
	m_mObjects.emplace( cube.getId(), std::move( cube ) );

	std::vector< glm::vec3 > lightColors{
		{ 1.f, .1f, .1f },
		{ .1f, .1f, 1.f },
		{ .1f, 1.f, .1f },
		{ 1.f, 1.f, .1f },
		{ .1f, 1.f, 1.f },
		{ 1.f, 1.f, 1.f },
	};

	for ( int i = 0; i < lightColors.size(); i++ )
	{
		auto pointLight = CatObject::makePointLight( std::string( "Light" ), 0.2f );
		pointLight.m_vColor = lightColors[i];
		auto rotateLight =
			glm::rotate( glm::mat4( 1.f ), ( i * glm::two_pi< float >() ) / lightColors.size(), { 0.f, -1.f, 0.f } );
		pointLight.m_transform.translation = glm::vec3( rotateLight * glm::vec4( -1.f, 1.f, -1.f, 1.f ) );
		m_mObjects.emplace( pointLight.getId(), std::move( pointLight ) );
	}
}
} // namespace cat
