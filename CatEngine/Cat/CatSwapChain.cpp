#include "CatSwapChain.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace cat
{

CatSwapChain::CatSwapChain( CatDevice* pDevice, vk::Extent2D extent ) : m_pDevice{ pDevice }, m_windowExtent{ extent }
{
	init();
}

CatSwapChain::CatSwapChain( CatDevice* pDevice, vk::Extent2D extent, std::shared_ptr< CatSwapChain > pPrevious )
	: m_pDevice{ pDevice }, m_windowExtent{ extent }, m_pOldSwapChain{ pPrevious }
{
	init();
	m_pOldSwapChain = nullptr;
}

void CatSwapChain::init()
{
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDepthResources();
	createFramebuffers();
	createSyncObjects();
}

CatSwapChain::~CatSwapChain()
{
	for ( auto imageView : m_aSwapChainImageViews )
	{
		m_pDevice->getDevice().destroyImageView( imageView, nullptr );
	}
	m_aSwapChainImageViews.clear();

	if ( !m_swapChain )
	{
		m_pDevice->getDevice().destroySwapchainKHR( m_swapChain, nullptr );
		m_swapChain = nullptr;
	}

	for ( int i = 0; i < m_aDepthImages.size(); i++ )
	{
		m_pDevice->getDevice().destroyImageView( m_aDepthImageViews[i], nullptr );
		m_pDevice->getDevice().destroyImage( m_aDepthImages[i], nullptr );
		m_pDevice->getDevice().freeMemory( m_aDepthImageMemorys[i], nullptr );
	}

	for ( auto framebuffer : m_aSwapChainFramebuffers )
	{
		m_pDevice->getDevice().destroyFramebuffer( framebuffer, nullptr );
	}

	m_pDevice->getDevice().destroyRenderPass( m_renderPass, nullptr );

	// cleanup synchronization objects
	for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
	{
		m_pDevice->getDevice().destroySemaphore( m_aRenderFinishedSemaphores[i], nullptr );
		m_pDevice->getDevice().destroySemaphore( m_aImageAvailableSemaphores[i], nullptr );
		m_pDevice->getDevice().destroyFence( m_aInFlightFences[i], nullptr );
	}
}

vk::Result CatSwapChain::acquireNextImage( uint32_t* imageIndex )
{
	m_pDevice->getDevice().waitForFences(
		1, &m_aInFlightFences[m_nCurrentFrame], VK_TRUE, std::numeric_limits< uint64_t >::max() );

	return m_pDevice->getDevice().acquireNextImageKHR( m_swapChain, std::numeric_limits< uint64_t >::max(),
		m_aImageAvailableSemaphores[m_nCurrentFrame], // must be a not signaled semaphore
		VK_NULL_HANDLE, imageIndex );
}

vk::Result CatSwapChain::submitCommandBuffers( const vk::CommandBuffer* buffers, uint32_t* imageIndex )
{
	if ( m_aImagesInFlight[*imageIndex] )
	{
		m_pDevice->getDevice().waitForFences( 1, &m_aImagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX );
	}
	m_aImagesInFlight[*imageIndex] = m_aInFlightFences[m_nCurrentFrame];

	vk::Semaphore waitSemaphores[] = { m_aImageAvailableSemaphores[m_nCurrentFrame] };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	vk::Semaphore signalSemaphores[] = { m_aRenderFinishedSemaphores[m_nCurrentFrame] };

	vk::SubmitInfo submitInfo = {
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores,
		.pWaitDstStageMask = waitStages,

		.commandBufferCount = 1,
		.pCommandBuffers = buffers,

		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores,
	};

	vkResetFences( m_pDevice.getDevice(), 1, &m_aInFlightFences[m_nCurrentFrame] );
	if ( vkQueueSubmit( m_pDevice.getGraphicsQueue(), 1, &submitInfo, m_aInFlightFences[m_nCurrentFrame] ) != VK_SUCCESS )
	{
		throw std::runtime_error( "failed to submit draw command buffer!" );
	}

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	vk::SwapchainKHR swapChains[] = { m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = imageIndex;

	auto result = vkQueuePresentKHR( m_pDevice.getPresentQueue(), &presentInfo );

	m_nCurrentFrame = ( m_nCurrentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;

	return result;
}

void CatSwapChain::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = m_pDevice.getSwapChainSupport();

	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat( swapChainSupport.formats );
	vk::PresentModeKHR presentMode = chooseSwapPresentMode( swapChainSupport.presentModes );
	vk::Extent2D extent = chooseSwapExtent( swapChainSupport.capabilities );

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if ( swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount )
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_pDevice.getSurface();

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = m_pDevice.findPhysicalQueueFamilies();
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

	if ( indices.graphicsFamily != indices.presentFamily )
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;	  // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = m_pOldSwapChain == nullptr ? VK_NULL_HANDLE : m_pOldSwapChain->m_swapChain;

	if ( vkCreateSwapchainKHR( m_pDevice.getDevice(), &createInfo, nullptr, &m_swapChain ) != VK_SUCCESS )
	{
		throw std::runtime_error( "failed to create swap chain!" );
	}

	// we only specified a minimum number of images in the swap chain, so the implementation is
	// allowed to create a swap chain with more. That's why we'll first query the final number of
	// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
	// retrieve the handles.
	vkGetSwapchainImagesKHR( m_pDevice.getDevice(), m_swapChain, &imageCount, nullptr );
	m_aSwapChainImages.resize( imageCount );
	vkGetSwapchainImagesKHR( m_pDevice.getDevice(), m_swapChain, &imageCount, m_aSwapChainImages.data() );

	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;
}

void CatSwapChain::createImageViews()
{
	m_aSwapChainImageViews.resize( m_aSwapChainImages.size() );
	for ( size_t i = 0; i < m_aSwapChainImages.size(); i++ )
	{
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_aSwapChainImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_swapChainImageFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if ( vkCreateImageView( m_pDevice.getDevice(), &viewInfo, nullptr, &m_aSwapChainImageViews[i] ) != VK_SUCCESS )
		{
			throw std::runtime_error( "failed to create texture image view!" );
		}
	}
}

void CatSwapChain::createRenderPass()
{
	vk::AttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	vk::AttachmentDescription colorAttachment = {};
	colorAttachment.format = getSwapChainImageFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	vk::AttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	vk::SubpassDependency dependency = {};
	dependency.dstSubpass = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.srcAccessMask = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

	std::array< vk::AttachmentDescription, 2 > attachments = { colorAttachment, depthAttachment };
	vk::RenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if ( vkCreateRenderPass( m_pDevice.getDevice(), &renderPassInfo, nullptr, &m_renderPass ) != VK_SUCCESS )
	{
		throw std::runtime_error( "failed to create render pass!" );
	}
}

void CatSwapChain::createFramebuffers()
{
	m_aSwapChainFramebuffers.resize( imageCount() );
	for ( size_t i = 0; i < imageCount(); i++ )
	{
		std::array< vk::ImageView, 2 > attachments = { m_aSwapChainImageViews[i], m_aDepthImageViews[i] };

		vk::Extent2D swapChainExtent = getSwapChainExtent();
		vk::FramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if ( vkCreateFramebuffer( m_pDevice.getDevice(), &framebufferInfo, nullptr, &m_aSwapChainFramebuffers[i] )
			 != VK_SUCCESS )
		{
			throw std::runtime_error( "failed to create framebuffer!" );
		}
	}
}

void CatSwapChain::createDepthResources()
{
	vk::Format depthFormat = findDepthFormat();
	m_swapChainDepthFormat = depthFormat;
	vk::Extent2D swapChainExtent = getSwapChainExtent();

	m_aDepthImages.resize( imageCount() );
	m_aDepthImageMemorys.resize( imageCount() );
	m_aDepthImageViews.resize( imageCount() );

	for ( int i = 0; i < m_aDepthImages.size(); i++ )
	{
		vk::ImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = swapChainExtent.width;
		imageInfo.extent.height = swapChainExtent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = depthFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = 0;

		m_pDevice.createImageWithInfo(
			imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_aDepthImages[i], m_aDepthImageMemorys[i] );

		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_aDepthImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = depthFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if ( vkCreateImageView( m_pDevice.getDevice(), &viewInfo, nullptr, &m_aDepthImageViews[i] ) != VK_SUCCESS )
		{
			throw std::runtime_error( "failed to create texture image view!" );
		}
	}
}

void CatSwapChain::createSyncObjects()
{
	m_aImageAvailableSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
	m_aRenderFinishedSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
	m_aInFlightFences.resize( MAX_FRAMES_IN_FLIGHT );
	m_aImagesInFlight.resize( imageCount(), VK_NULL_HANDLE );

	vk::SemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vk::FenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
	{
		if ( vkCreateSemaphore( m_pDevice.getDevice(), &semaphoreInfo, nullptr, &m_aImageAvailableSemaphores[i] ) != VK_SUCCESS
			 || vkCreateSemaphore( m_pDevice.getDevice(), &semaphoreInfo, nullptr, &m_aRenderFinishedSemaphores[i] )
					!= VK_SUCCESS
			 || vkCreateFence( m_pDevice.getDevice(), &fenceInfo, nullptr, &m_aInFlightFences[i] ) != VK_SUCCESS )
		{
			throw std::runtime_error( "failed to create synchronization objects for a frame!" );
		}
	}
}

vk::SurfaceFormatKHR CatSwapChain::chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >& availableFormats )
{
	for ( const auto& availableFormat : availableFormats )
	{
		if ( availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			 && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

vk::PresentModeKHR CatSwapChain::chooseSwapPresentMode( const std::vector< vk::PresentModeKHR >& availablePresentModes )
{
	for ( const auto& availablePresentMode : availablePresentModes )
	{
		if ( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
		{
			std::cout << "Present mode: Mailbox" << std::endl;
			return availablePresentMode;
		}
	}

	// for (const auto &availablePresentMode : availablePresentModes) {
	//   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
	//     std::cout << "Present mode: Immediate" << std::endl;
	//     return availablePresentMode;
	//   }
	// }

	std::cout << "Present mode: V-Sync" << std::endl;
	return VK_PRESENT_MODE_FIFO_KHR;
}

vk::Extent2D CatSwapChain::chooseSwapExtent( const vk::SurfaceCapabilitiesKHR& capabilities )
{
	if ( capabilities.currentExtent.width != std::numeric_limits< uint32_t >::max() )
	{
		return capabilities.currentExtent;
	}
	else
	{
		vk::Extent2D actualExtent = m_windowExtent;
		actualExtent.width =
			std::max( capabilities.minImageExtent.width, std::min( capabilities.maxImageExtent.width, actualExtent.width ) );
		actualExtent.height =
			std::max( capabilities.minImageExtent.height, std::min( capabilities.maxImageExtent.height, actualExtent.height ) );

		return actualExtent;
	}
}

vk::Format CatSwapChain::findDepthFormat()
{
	return m_pDevice.findSupportedFormat( { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );
}

} // namespace cat
