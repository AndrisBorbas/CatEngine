#include "CatImgui.hpp"

#include "CatApp.hpp"
#include "CatApp.hpp"
#include "CatApp.hpp"
#include "CatApp.hpp"
#include "CatFrameInfo.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <thread>
#include <future>
#include <functional>

#include "ImGuizmo.h"
#include "implot.h"

namespace cat
{
CatImgui::CatImgui( CatWindow* pWindow, CatDevice* pDevice, vk::RenderPass renderPass, size_t imageCount )
	: m_pWindow{ pWindow }, m_pDevice{ pDevice }
{
	m_pDescriptorPool = CatDescriptorPool::Builder( *m_pDevice )
							.addPoolSize( vk::DescriptorType::eSampler, 1000 )
							.addPoolSize( vk::DescriptorType::eCombinedImageSampler, 1000 )
							.addPoolSize( vk::DescriptorType::eSampledImage, 1000 )
							.addPoolSize( vk::DescriptorType::eStorageImage, 1000 )
							.addPoolSize( vk::DescriptorType::eUniformTexelBuffer, 1000 )
							.addPoolSize( vk::DescriptorType::eStorageTexelBuffer, 1000 )
							.addPoolSize( vk::DescriptorType::eUniformBuffer, 1000 )
							.addPoolSize( vk::DescriptorType::eStorageBuffer, 1000 )
							.addPoolSize( vk::DescriptorType::eUniformBufferDynamic, 1000 )
							.addPoolSize( vk::DescriptorType::eStorageBufferDynamic, 1000 )
							.addPoolSize( vk::DescriptorType::eInputAttachment, 1000 )
							.setMaxSets( 1000 * 11 )
							.setPoolFlags( vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet )
							.build();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	// Initialize imgui for vulkan
	ImGui_ImplGlfw_InitForVulkan( **m_pWindow, true );
	ImGui_ImplVulkan_InitInfo initInfo = {
		.Instance = m_pDevice->getInstance(),
		.PhysicalDevice = m_pDevice->getPhysicalDevice(),
		.Device = m_pDevice->getDevice(),
		.QueueFamily = m_pDevice->getGraphicsQueueFamily(),
		.Queue = m_pDevice->getGraphicsQueue(),
		// pipeline cache is a potential future optimization, ignoring for now
		.PipelineCache = nullptr,
		.DescriptorPool = m_pDescriptorPool->getDescriptorPool(),
		.Subpass = 0,
		.MinImageCount = 2,
		.ImageCount = static_cast< uint32_t >( imageCount ),
		.MSAASamples = static_cast< VkSampleCountFlagBits >( m_pDevice->getMSAA() ),
		// .MSAASamples = VK_SAMPLE_COUNT_8_BIT,
		// todo, I should probably get around to integrating a memory allocator library such as Vulkan
		// memory allocator (VMA) sooner than later. We don't want to have to update adding an allocator
		// in a ton of locations.
		.Allocator = nullptr,
		.CheckVkResultFn = check_vk_result,
	};
	ImGui_ImplVulkan_Init( &initInfo, renderPass );

	const auto commandBuffer = m_pDevice->beginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture( commandBuffer );
	m_pDevice->endSingleTimeCommands( commandBuffer );
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

CatImgui::~CatImgui()
{
	// m_rDevice.getDevice().destroyDescriptorPool( m_pDescriptorPool->getDescriptorPool() );
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}

void CatImgui::newFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

// this tells imgui that we're done setting up the current frame,
// then gets the draw data from imgui and uses it to record to the provided
// command buffer the necessary draw commands
void CatImgui::render( vk::CommandBuffer commandBuffer )
{
	ImGui::Render();
	ImDrawData* drawdata = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData( drawdata, commandBuffer );
}

void CatImgui::renderPlatformWindows()
{
	if ( ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void CatImgui::createDockSpace()
{
	static ImGuiDockNodeFlags dockspaceFlags =
		ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingInCentralNode;
	ImGui::DockSpaceOverViewport( ImGui::GetMainViewport(), dockspaceFlags );
}


void CatImgui::drawWindows()
{
	{
		// const ImGuiViewport* viewport = ImGui::GetMainViewport();
		//		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		//			window_flags |= ImGuiWindowFlags_NoBackground;
		// ImGui::DockSpaceOverViewport( ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode );
	}
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can
	// browse its code to learn more about Dear ImGui!).
	if ( m_bShowDemoWindow ) ImGui::ShowDemoWindow( &m_bShowDemoWindow );

	// ImPlot::ShowDemoWindow();

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named
	// window.
	{
		char title[128];
		sprintf( title, "%.4f ms / %.1f FPS | %llu# ###Main", GetEditorInstance()->m_DFrameTime * 1000.0,
			GetEditorInstance()->m_DFrameRate, GetEditorInstance()->m_RFrameInfo.m_nFrameNumber );
		ImGui::Begin( title ); // Create a window and append into it.

		// ImGui::Text( "This is some useful text." ); // Display some text (you can use a format strings too)
		ImGui::Checkbox( "Demo Window",
			&m_bShowDemoWindow ); // Edit bools storing our window open/close state
		ImGui::Checkbox( "Debug Window", &m_bShowDebugWindow );

		// ImGui::SliderFloat( "float", &f, 0.0f, 1.0f ); // Edit 1 float using a slider from 0.0f to 1.0f
		// ImGui::ColorEdit3( "clear color",
		//	reinterpret_cast< float* >( &m_vClearColor ) ); // Edit 3 floats representing a color

		// if ( ImGui::Button( "Button" ) ) // Buttons return true when clicked (most widgets return true
		//	// when edited/activated)
		//	counter++;
		// ImGui::SameLine();
		// ImGui::Text( "counter = %d", counter );

		ImGui::DragFloat3( "cam pos",
			reinterpret_cast< float* >( &GetEditorInstance()->m_RFrameInfo.m_rCameraObject.m_transform.translation ), 0.1f );
		ImGui::DragFloat3( "cam rot",
			reinterpret_cast< float* >( &GetEditorInstance()->m_RFrameInfo.m_rCameraObject.m_transform.rotation ), 0.1f );
		// ImGui::DragFloat3( "pos", (float*)&pFrameInfo.m_rUBO.lightPosition, 0.1f );

		static char buf[128] = "wasd";
		ImGui::DragFloat( "cam spd mult", &GetEditorInstance()->m_FCameraSpeed, 0.1f );
		ImGui::InputTextWithHint( "##Lavel Name", "Filename", buf, 128, ImGuiInputTextFlags_CharsNoBlank );
		if ( ImGui::Button( "Load Level" ) )
		{
			std::string name( buf );
			if ( !name.ends_with( ".json" ) )
			{
				name += ".json";
			}
			LOG_F( INFO, "Frame: %llu", GetEditorInstance()->m_RFrameInfo.m_nFrameNumber );

			GetEditorInstance()->loadLevel( name );
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Save Level" ) )
		{
			std::string name( buf );
			GetEditorInstance()->saveLevel( name );
		}

		
		if ( ImPlot::BeginPlot( "##FramePacing", ImVec2( -1, 150 ) ) )
		{
			static double history = 1.0;
			static double t = 0.0;
			t += GetEditorInstance()->m_DFrameTime;
			m_vFrameTimes.add( float( t ), float( GetEditorInstance()->m_DFrameTime * 1000.0 ) );
			m_vFrameRates.add( float( t ), float( GetEditorInstance()->m_DFrameRate ) );

			static ImPlotAxisFlags flags;
			ImPlot::SetupAxes( nullptr, nullptr, flags, flags );
			ImPlot::SetupAxis( ImAxis_Y2, "", ImPlotAxisFlags_AuxDefault );
			ImPlot::SetupAxisLimits( ImAxis_X1, t - history, t, ImGuiCond_Always );
			ImPlot::SetupAxisLimits( ImAxis_Y1, 0, 10.0f );
			ImPlot::SetupAxisLimits( ImAxis_Y2, 0, 1000.0f );

			ImPlot::TagY( GetEditorInstance()->m_DFrameTime * 1000.0f, ImVec4( 0, 0, 0.475f, 0.725f ), "%.4f",
				GetEditorInstance()->m_DFrameTime * 1000.0f );
			ImPlot::PlotLine( "FrameTime", &m_vFrameTimes.m_vData[0].x, &m_vFrameTimes.m_vData[0].y,
				m_vFrameTimes.m_vData.size(), 0, m_vFrameTimes.m_nOffset, 2 * sizeof( float ) );

			ImPlot::SetAxes( ImAxis_X1, ImAxis_Y2 );
			ImPlot::TagY( GetEditorInstance()->m_DFrameRate, ImVec4( 0.863f, 0.256f, 0, 0.625f ), "%.1f",
				GetEditorInstance()->m_DFrameRate );
			ImPlot::PlotLine( "FrameRate", &m_vFrameRates.m_vData[0].x, &m_vFrameRates.m_vData[0].y,
				m_vFrameRates.m_vData.size(), 0, m_vFrameRates.m_nOffset, 2 * sizeof( float ) );
			ImPlot::EndPlot();
		}

		ImGui::End();
	}
	{
		ImGui::Begin( "Objects" );

		static bool bShowHidden = false;

		ImGui::Checkbox( "Show Hidden", &bShowHidden );

		if ( ImGui::BeginListBox( "##ObjectsLB", ImVec2( -FLT_MIN, -FLT_MIN ) ) )
		{
			// static CatObject::id_t currentItemIdx = 0;
			int i = 0;
			for ( auto& [key, object] : GetEditorInstance()->m_RFrameInfo.m_rLevel->getAllObjects() )
			{
				if ( !bShowHidden && object->getName().find( "Chunk" ) != std::string::npos )
				{
					continue;
				}
				++i;
				ImGui::PushID( i );
				const bool isSelected = ( GetEditorInstance()->m_RFrameInfo.m_selectedItemId == key );
				if ( ImGui::Selectable( ( object->getName() ).c_str(), isSelected ) )
				{
					// currentItemIdx = key;
					GetEditorInstance()->m_RFrameInfo.updateSelectedItemId( key );
				}
				ImGui::PopID();

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if ( isSelected ) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}

		ImGui::End();
	}
	{
		if ( GetEditorInstance()->m_RFrameInfo.m_selectedItemId != 0 )
		{
			ImGui::Begin( "SelectedObject" );

			auto pObject = GetEditorInstance()->m_RFrameInfo.m_rLevel->getAllObjects().at(
				GetEditorInstance()->m_RFrameInfo.m_selectedItemId );

			auto bIsGlobal = false;

			ImGui::DragFloat3( "Position", reinterpret_cast< float* >( &pObject->m_transform.translation ), 0.1f );
			ImGui::DragFloat3( "Rotation", reinterpret_cast< float* >( &pObject->m_transform.rotation ), 0.1f );
			ImGui::DragFloat3( "Scale", reinterpret_cast< float* >( &pObject->m_transform.scale ), 0.1f );

			ImGui::Checkbox( "Is Global", &bIsGlobal );

			ImGui::End();
		}
	}
}

void CatImgui::drawDebug( const glm::mat4 mx1, const glm::mat4 mx2 )
{
	if ( m_bShowDebugWindow )
	{
		ImGui::Begin( "MX1", &m_bShowDebugWindow );
		ImGui::DragFloat3( "", (float*)&mx1[0] );
		ImGui::DragFloat3( "", (float*)&mx1[1] );
		ImGui::DragFloat3( "", (float*)&mx1[2] );
		ImGui::DragFloat3( "", (float*)&mx1[3] );
		ImGui::End();

		ImGui::Begin( "MX2", &m_bShowDebugWindow );
		ImGui::DragFloat3( "", (float*)&mx2[0] );
		ImGui::DragFloat3( "", (float*)&mx2[1] );
		ImGui::DragFloat3( "", (float*)&mx2[2] );
		ImGui::DragFloat3( "", (float*)&mx2[3] );
		ImGui::End();
	}
}
} // namespace cat
