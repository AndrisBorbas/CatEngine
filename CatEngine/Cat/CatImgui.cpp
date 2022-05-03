#include "CatImgui.hpp"

#include "CatApp.hpp"
#include "CatApp.hpp"
#include "CatApp.hpp"
#include "CatApp.hpp"
#include "CatFrameInfo.hpp"

namespace cat
{
// ok this just initializes imgui using the provided integration files. So in our case we need to
// initialize the vulkan and glfw imgui implementations, since that's what our engine is built
// using.
CatImgui::CatImgui( CatApp& app, CatWindow& window, CatDevice& device, vk::RenderPass renderPass, size_t imageCount )
	: m_rDevice{ device }, m_rWindow{ window }, m_rApp{ app }
{
	// set up a descriptor pool stored on this instance, see header for more comments on this.
	//	vk::DescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
	//		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 }, { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
	//		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 }, { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
	//		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 }, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
	//		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 }, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
	//		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 }, { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };
	//	vk::DescriptorPoolCreateInfo pool_info = {};
	//	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	//	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	//	pool_info.maxSets = 1000 * IM_ARRAYSIZE( pool_sizes );
	//	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE( pool_sizes );
	//	pool_info.pPoolSizes = pool_sizes;
	//	if ( vkCreateDescriptorPool( device.device(), &pool_info, nullptr, &descriptorPool ) != VK_SUCCESS )
	//	{
	//		throw std::runtime_error( "failed to set up descriptor pool for imgui" );
	//	}

	m_pDescriptorPool = CatDescriptorPool::Builder( m_rDevice )
							.setMaxSets( 1000 )
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
							.build();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	// Initialize imgui for vulkan
	ImGui_ImplGlfw_InitForVulkan( window.getGLFWwindow(), true );
	ImGui_ImplVulkan_InitInfo init_info = {
		.Instance = device.getInstance(),
		.PhysicalDevice = device.getPhysicalDevice(),
		.Device = device.getDevice(),
		.QueueFamily = device.getGraphicsQueueFamily(),
		.Queue = device.getGraphicsQueue(),
		// pipeline cache is a potential future optimization, ignoring for now
		.PipelineCache = nullptr,
		.DescriptorPool = m_pDescriptorPool->getDescriptorPool(),
		.MinImageCount = 2,
		.ImageCount = static_cast< uint32_t >( imageCount ),
		// todo, I should probably get around to integrating a memory allocator library such as Vulkan
		// memory allocator (VMA) sooner than later. We don't want to have to update adding an allocator
		// in a ton of locations.
		.Allocator = nullptr,
		.CheckVkResultFn = check_vk_result,
	};


	ImGui_ImplVulkan_Init( &init_info, renderPass );

	// upload fonts, this is done by recording and submitting a one time use command buffer
	// which can be done easily bye using some existing helper functions on the lve device object
	const auto commandBuffer = device.beginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture( commandBuffer );
	device.endSingleTimeCommands( commandBuffer );
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

CatImgui::~CatImgui()
{
	//	vkDestroyDescriptorPool( lveDevice.device(), descriptorPool, nullptr );
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
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


void CatImgui::drawWindows( CatFrameInfo& pFrameInfo, glm::vec3 vCameraPos, glm::vec3 vCameraRot )
{
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can
	// browse its code to learn more about Dear ImGui!).
	if ( m_bShowDemoWindow ) ImGui::ShowDemoWindow( &m_bShowDemoWindow );

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named
	// window.
	{
		static float f = 0.0f;

		char title[128];
		sprintf( title, "%.4f ms / %.1f FPS | %llu# ###Main", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate,
			pFrameInfo.m_nFrameNumber );
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

		ImGui::DragFloat3( "camera position", reinterpret_cast< float* >( &vCameraPos ) );
		ImGui::DragFloat3( "camera rotation", reinterpret_cast< float* >( &vCameraRot ) );
		// ImGui::DragFloat3( "pos", (float*)&pFrameInfo.m_rUBO.lightPosition, 0.1f );

		static char buf[32] = "asd";
		ImGui::InputTextWithHint( "##Lavel Name", "Filename", buf, 32, ImGuiInputTextFlags_CharsNoBlank );
		if ( ImGui::Button( "Load Level" ) )
		{
			std::string name( buf );
			if ( !name.ends_with( ".json" ) )
			{
				name += ".json";
			}
			m_rApp.loadLevel( name );
			pFrameInfo.updateSelectedItemId( pFrameInfo.m_mObjects.begin()->first );
			ImGui::End();
			return;
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Save Level" ) )
		{
			std::string name( buf );
			if ( !name.ends_with( ".json" ) )
			{
				name += ".json";
			}
			m_rApp.saveLevel( name );
		}

		ImGui::End();
	}
	{
		ImGui::Begin( "Objects" );

		if ( ImGui::BeginListBox( "##ObjectsLB", ImVec2( -FLT_MIN, -FLT_MIN ) ) )
		{
			// static CatObject::id_t currentItemIdx = 0;
			for ( auto& [key, object] : pFrameInfo.m_mObjects )
			{
				const bool isSelected = ( pFrameInfo.m_selectedItemId == key );
				if ( ImGui::Selectable( object.getName().c_str(), isSelected ) )
				{
					// currentItemIdx = key;
					pFrameInfo.updateSelectedItemId( key );
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if ( isSelected ) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}

		ImGui::End();
	}
	{
		ImGui::Begin( "SelectedObject" );

		ImGui::DragFloat3( "Position",
			reinterpret_cast< float* >( &pFrameInfo.m_mObjects.at( pFrameInfo.m_selectedItemId ).m_transform.translation ),
			0.1f );
		ImGui::DragFloat3( "Rotation",
			reinterpret_cast< float* >( &pFrameInfo.m_mObjects.at( pFrameInfo.m_selectedItemId ).m_transform.rotation ), 0.1f );
		ImGui::DragFloat3( "Scale",
			reinterpret_cast< float* >( &pFrameInfo.m_mObjects.at( pFrameInfo.m_selectedItemId ).m_transform.scale ), 0.1f );

		ImGui::End();
	}
}

void CatImgui::drawDebug( const glm::mat4 mView, const glm::mat4 mProj )
{
	if ( m_bShowDebugWindow )
	{
		ImGui::Begin( "View Matrix", &m_bShowDebugWindow );
		ImGui::DragFloat3( "A", (float*)&mView[0] );
		ImGui::DragFloat3( "B", (float*)&mView[1] );
		ImGui::DragFloat3( "C", (float*)&mView[2] );
		ImGui::DragFloat3( "D", (float*)&mView[3] );
		ImGui::End();

		ImGui::Begin( "Proj Matrix", &m_bShowDebugWindow );
		ImGui::DragFloat3( "A", (float*)&mProj[0] );
		ImGui::DragFloat3( "B", (float*)&mProj[1] );
		ImGui::DragFloat3( "C", (float*)&mProj[2] );
		ImGui::DragFloat3( "D", (float*)&mProj[3] );
		ImGui::End();
	}
}
} // namespace cat
