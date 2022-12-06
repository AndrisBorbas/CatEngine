#include "CatApp.hpp"

#include "Cat/Controller/CatInput.hpp"
#include "Cat/VulkanRHI/CatBuffer.hpp"
#include "Cat/Controller/CatCamera.hpp"
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

#include "ImGuizmo.h"
#include "loguru.hpp"
#include "Objects/CatLight.hpp"
#include "Objects/CatVolume.hpp"
#include "RenderSystems/CatWireframeRenderSystem.hpp"
#include "Cat/RenderSystems/CatGridRenderSystem.hpp"
#include "Cat/Texture/CatTexture.hpp"
#include "Cat/RenderSystems/CatTerrainRenderSystem.hpp"
#include "Cat/Rendering/CatFrustum.hpp"

namespace cat
{
CatApp* GlobalEditorInstance = nullptr;

CatApp* GetEditorInstance()
{
	return GlobalEditorInstance;
}

CatApp* GEI()
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
	m_pWindow = new CatWindow( WIDTH, HEIGHT, "Cat Engine", false );
	m_pDevice = new CatDevice( m_PWindow );
	m_pRenderer = new CatRenderer( m_PWindow, m_PDevice );

	m_pGlobalDescriptorPool = CatDescriptorPool::Builder( *m_PDevice )
								  .setMaxSets( CatSwapChain::MAX_FRAMES_IN_FLIGHT )
								  .addPoolSize( vk::DescriptorType::eUniformBuffer, CatSwapChain::MAX_FRAMES_IN_FLIGHT )
								  .build();

	m_aUboBuffers = std::vector< std::unique_ptr< CatBuffer > >( CatSwapChain::MAX_FRAMES_IN_FLIGHT );
	for ( auto& uboBuffer : m_aUboBuffers )
	{
		uboBuffer = std::make_unique< CatBuffer >( m_PDevice, sizeof( GlobalUbo ), 1, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible );
		uboBuffer->map();
	}

	m_pGlobalDescriptorSetLayout =
		CatDescriptorSetLayout::Builder( *m_PDevice )
			.addBinding( 0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics )
			.build();

	m_aGlobalDescriptorSets = std::vector< vk::DescriptorSet >( CatSwapChain::MAX_FRAMES_IN_FLIGHT );
	for ( size_t i = 0; i < m_aGlobalDescriptorSets.size(); i++ )
	{
		auto bufferInfo = m_aUboBuffers[i]->descriptorInfo();
		CatDescriptorWriter( *m_pGlobalDescriptorSetLayout, *m_pGlobalDescriptorPool )
			.writeBuffer( 0, &bufferInfo )
			.build( m_aGlobalDescriptorSets[i] );
	}
}

CatApp::~CatApp() = default;

void CatApp::init()
{
	m_pCurrentLevel = CatLevel::create( "Base" );

	m_pImgui = new CatImgui( m_PWindow, m_PDevice, m_pRenderer->getSwapChainRenderPass(), m_pRenderer->getImageCount() );

	CatInput::registerInputHandlers();

	m_pCameraObject = CatObject::create( "Camera", "", ObjectType::eCamera );
	m_pCameraObject->m_transform.translation = { 0.f, 1.5f, 2.5f };

	m_pFrameInfo = std::make_unique< CatFrameInfo >(
		nullptr, m_camera, *m_pCameraObject, m_aGlobalDescriptorSets[0], m_ubo, m_pCurrentLevel );
}

void CatApp::run()
{
	auto yes = CatTexture2D( m_PDevice, "assets/textures/yes.png" );
	auto tex = CatTexture2D( m_PDevice, "assets/textures/terrain.tga", vk::Format::eR8Srgb, STBI_grey );

	CatSimpleRenderSystem simpleRenderSystem{
		m_PDevice, m_pRenderer->getSwapChainRenderPass(), m_pGlobalDescriptorSetLayout->getDescriptorSetLayout() };
	CatPointLightRenderSystem pointLightRenderSystem{
		m_PDevice, m_pRenderer->getSwapChainRenderPass(), m_pGlobalDescriptorSetLayout->getDescriptorSetLayout() };
	CatWireframeRenderSystem wireframeRenderSystem{
		m_PDevice, m_pRenderer->getSwapChainRenderPass(), m_pGlobalDescriptorSetLayout->getDescriptorSetLayout() };
	CatGridRenderSystem gridRenderSystem{
		m_PDevice, m_pRenderer->getSwapChainRenderPass(), m_pGlobalDescriptorSetLayout->getDescriptorSetLayout() };
	CatTerrainRenderSystem terrainRenderSystem{ m_PDevice, m_pRenderer->getSwapChainRenderPass(),
		m_pCurrentLevel->m_PTerrain->m_PDescriptorSetLayout->getDescriptorSetLayout() };
	CatFrustum frustum;


	uint64_t nFrames = 0;

	auto currentTime = std::chrono::high_resolution_clock::now();

	std::shared_ptr< cat::CatObject > pSelectedItem;

	// Main loop
	while ( !m_PWindow->shouldClose() )
	{
		glfwPollEvents();

		auto newTime = std::chrono::high_resolution_clock::now();
		m_dFrameTime = std::chrono::duration< double, std::chrono::seconds::period >( newTime - currentTime ).count();
		currentTime = newTime;
		m_dDeltaTime += m_dFrameTime;
		nFrames++;

		// Update delta time every frame
		if ( m_dDeltaTime >= 0.1 )
		{
			m_dFrameRate = ( static_cast< double >( nFrames ) * 0.5 / 0.1 + m_dFrameRate * 0.5 );
			nFrames = 0;
			m_dDeltaTime -= 0.1;
			// m_dFrameRate = 1000.0 / ( m_dFrameRate == 0.0 ? 0.001 : m_dFrameRate );
		}

		m_pCurrentLevel->loadChunk( m_pCameraObject->m_transform.translation, m_bRenderEverything ? 1000 : 1 );

		if ( m_pCurrentLevel->isLoadingFinished() )
		{
			m_bTerrain = true;
			DLOG_F( INFO, "Frame: %llu, level fully loaded", GetEditorInstance()->getFrameInfo().m_nFrameNumber );
			// Level fully loaded
		}

		auto bWasCameraMatrixUpdated = m_cameraController.moveInPlaneXZ(
			m_PWindow->getGLFWwindow(), static_cast< float >( m_dFrameTime ), getFrameInfo().m_rCameraObject );
		m_camera.setViewYXZ(
			getFrameInfo().m_rCameraObject.m_transform.translation, getFrameInfo().m_rCameraObject.m_transform.rotation );

		float aspect = m_pRenderer->getAspectRatio();
		m_camera.setPerspectiveProjection( glm::radians( 50.f ), aspect, 0.10f, 1000.f );

		auto imguizmoCamera = m_camera;
		imguizmoCamera.setPerspectiveProjectionImGuizmo( glm::radians( 50.f ), aspect, 0.10f, 100.f );
		imguizmoCamera.setViewYXZ(
			getFrameInfo().m_rCameraObject.m_transform.translation, getFrameInfo().m_rCameraObject.m_transform.rotation );


		if ( auto commandBuffer = m_pRenderer->beginFrame() )
		{
			short frameIndex = m_pRenderer->getFrameIndex();

			m_pFrameInfo->update(
				commandBuffer, m_aGlobalDescriptorSets[frameIndex], m_dFrameTime, frameIndex, m_pRenderer->getFrameNumber() );

			m_ubo.projection = m_camera.getProjection();
			m_ubo.view = m_camera.getView();
			m_ubo.inverseView = m_camera.getInverseView();
			m_aUboBuffers[getFrameInfo().m_nFrameIndex]->writeToBuffer( &m_ubo );
			m_aUboBuffers[getFrameInfo().m_nFrameIndex]->flush();

			m_pCurrentLevel->m_PTerrain->m_Ubo.projection = m_camera.getProjection();
			m_pCurrentLevel->m_PTerrain->m_Ubo.view = m_camera.getView();
			m_pCurrentLevel->m_PTerrain->m_Ubo.viewportDimensions = {
				m_pWindow->getExtent().width, m_pWindow->getExtent().height };

			if ( m_bUpdateFrustum )
			{
				frustum.update( m_camera.getProjection() * m_camera.getView() );
			}
			memcpy( m_pCurrentLevel->m_PTerrain->m_Ubo.frustumPlanes, frustum.m_APlanes.data(), sizeof( glm::vec4 ) * 6 );

			m_pCurrentLevel->m_PTerrain->m_AUboBuffers[getFrameInfo().m_nFrameIndex]->writeToBuffer(
				&m_pCurrentLevel->m_PTerrain->m_Ubo );
			m_pCurrentLevel->m_PTerrain->m_AUboBuffers[getFrameInfo().m_nFrameIndex]->flush();


			// tell imgui that we're starting a new frame
			CatImgui::newFrame();

			// start new frame
			m_pRenderer->beginSwapChainRenderPass( commandBuffer );

			pointLightRenderSystem.update( getFrameInfo(), m_ubo, true );

			cat::CatImgui::createDockSpace();

			ImGuizmo::SetDrawlist( ImGui::GetBackgroundDrawList() );

			// ImGuizmo::DrawGrid(
			// glm::value_ptr( imguizmoCamera.getView() ), glm::value_ptr( imguizmoCamera.getProjection() ), ID_MX, 16.f );

			// render game objects first, so they will be rendered in the background. This
			// is the best we can do for now.
			// Once we cover offscreen rendering, we can render the scene to a image/texture rather than
			// directly to the swap chain. This texture of the scene can then be rendered to an imgui
			// subwindow
			if ( m_bTerrain )
			{
				terrainRenderSystem.render( getFrameInfo() );
			}
			simpleRenderSystem.renderObjects( getFrameInfo() );
			wireframeRenderSystem.renderObjects( getFrameInfo() );
			gridRenderSystem.renderObjects( getFrameInfo() );
			pointLightRenderSystem.render( getFrameInfo() );

			ImGuizmo::Enable( true );
			ImGuizmo::SetDrawlist( ImGui::GetBackgroundDrawList() );
			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(
				ImGui::GetMainViewport()->Pos.x, ImGui::GetMainViewport()->Pos.y, io.DisplaySize.x, io.DisplaySize.y );

			if ( ImGui::IsKeyPressed( ImGuiKey_1 ) ) m_eGizmoOperation = ImGuizmo::TRANSLATE;
			if ( ImGui::IsKeyPressed( ImGuiKey_2 ) ) m_eGizmoOperation = ImGuizmo::ROTATE;
			if ( ImGui::IsKeyPressed( ImGuiKey_3 ) ) m_eGizmoOperation = ImGuizmo::SCALE;
			if ( ImGui::IsKeyPressed( ImGuiKey_4 ) ) m_eGizmoOperation = ImGuizmo::UNIVERSAL;

			if ( ImGui::IsKeyPressed( ImGuiKey_8 ) ) m_eGizmoMode = ImGuizmo::LOCAL;
			if ( ImGui::IsKeyPressed( ImGuiKey_9 ) ) m_eGizmoMode = ImGuizmo::WORLD;

			// glm::vec3 flush{ 1.f, 1.f, 1.f };
			if ( getFrameInfo().m_selectedItemId != 0 )
			{
				pSelectedItem = m_pCurrentLevel->getAllObjects().at( getFrameInfo().m_selectedItemId );

				float mxManipulate[16];
				ImGuizmo::RecomposeMatrixFromComponents( glm::value_ptr( pSelectedItem->m_transform.translation ),
					glm::value_ptr( pSelectedItem->m_transform.rotation ), glm::value_ptr( pSelectedItem->m_transform.scale ),
					mxManipulate );
				auto isManipulated = ImGuizmo::Manipulate( glm::value_ptr( imguizmoCamera.getView() ),
					glm::value_ptr( imguizmoCamera.getProjection() ), m_eGizmoOperation, m_eGizmoMode, mxManipulate, nullptr,
					nullptr );
				ImGuizmo::DecomposeMatrixToComponents( mxManipulate, glm::value_ptr( pSelectedItem->m_transform.translation ),
					glm::value_ptr( pSelectedItem->m_transform.rotation ), glm::value_ptr( pSelectedItem->m_transform.scale ) );

				if ( isManipulated )
				{
					// m_mObjects.at( frameInfo.m_selectedItemId ).m_transform.rotation *= ( glm::pi< float >() * 180.f );
					m_pCurrentLevel->updateObjectLocation( getFrameInfo().m_selectedItemId );
				}
			}

			// example code telling imgui what windows to render, and their contents
			// this can be replaced with whatever code/classes you set up configuring your
			// desired engine UI
			m_pImgui->drawWindows();

			m_pImgui->drawDebug( m_camera.getProjection(), imguizmoCamera.getProjection() );


			// as last step in render pass, record the imgui draw commands
			cat::CatImgui::render( commandBuffer );

			// Update and Render additional Platform Windows
			CatImgui::renderPlatformWindows();


			m_pRenderer->endSwapChainRenderPass( commandBuffer );
			m_pRenderer->endFrame();
		}
	}

	( **m_PDevice ).waitIdle();
}

void CatApp::saveLevel( const std::string& sFileName ) const
{
	m_pCurrentLevel->save( sFileName );
}

void CatApp::loadLevel( const std::string& sFileName, const bool bClearPrevious /* = true */ )
{
	m_RFrameInfo.m_selectedItemId = 0;
	m_bTerrain = false;
	// std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	m_pCurrentLevel = CatLevel::load( sFileName );
}


} // namespace cat
