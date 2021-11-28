#include "CatImgui.hpp"

namespace cat
{

// ok this just initializes imgui using the provided integration files. So in our case we need to
// initialize the vulkan and glfw imgui implementations, since that's what our engine is built
// using.
CatImgui::CatImgui( CatApp& app, CatWindow& window, CatDevice& device, vk::RenderPass renderPass, uint32_t imageCount )
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
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

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
		.ImageCount = imageCount,
		// todo, I should probably get around to integrating a memory allocator library such as Vulkan
		// memory allocator (VMA) sooner than later. We don't want to have to update adding an allocator
		// in a ton of locations.
		.Allocator = nullptr,
		.CheckVkResultFn = check_vk_result,
	};


	ImGui_ImplVulkan_Init( &init_info, renderPass );

	// upload fonts, this is done by recording and submitting a one time use command buffer
	// which can be done easily bye using some existing helper functions on the lve device object
	auto commandBuffer = device.beginSingleTimeCommands();
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

void CatImgui::runExample( glm::vec3 vCameraPos, glm::vec3 vCameraRot )
{
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can
	// browse its code to learn more about Dear ImGui!).
	if ( m_bShowDemoWindow ) ImGui::ShowDemoWindow( &m_bShowDemoWindow );

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named
	// window.
	{
		static float f = 0.0f;
		static int counter = 0;

		char title[128];

		sprintf( title, "%.4f ms / %.1f FPS###Main", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );

		ImGui::Begin( title ); // Create a window and append into it.

		ImGui::Text( "This is some useful text." ); // Display some text (you can use a format strings too)
		ImGui::Checkbox( "Demo Window",
			&m_bShowDemoWindow ); // Edit bools storing our window open/close state
		ImGui::Checkbox( "Another Window", &m_bShowAnotherWindow );

		ImGui::SliderFloat( "float", &f, 0.0f, 1.0f ); // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3( "clear color",
			(float*)&m_vClearColor ); // Edit 3 floats representing a color

		if ( ImGui::Button( "Button" ) ) // Buttons return true when clicked (most widgets return true
										 // when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text( "counter = %d", counter );

		ImGui::DragFloat3( "camera position", (float*)&vCameraPos );
		ImGui::DragFloat3( "camera rotation", (float*)&vCameraRot );
		ImGui::DragFloat3( "pos", (float*)&m_rApp.getObjects()[0].m_transform.translation, 0.1f );

		ImGui::End();
	}

	// 3. Show another simple window.
	if ( m_bShowAnotherWindow )
	{
		ImGui::Begin( "Another Window",
			&m_bShowAnotherWindow ); // Pass a pointer to our bool variable (the window will have a
									 // closing button that will clear the bool when clicked)
		ImGui::Text( "Hello from another window!" );
		if ( ImGui::Button( "Close Me" ) ) m_bShowAnotherWindow = false;
		ImGui::End();
	}
}

} // namespace cat
