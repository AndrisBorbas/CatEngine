#include "CatSwapChain.hpp"

#include <array>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace cat
{
CatSwapChain::CatSwapChain( CatDevice& rDevice, vk::Extent2D extent ) : m_rDevice{ rDevice }, windowExtent{ extent }
{
	init();
}

CatSwapChain::CatSwapChain( CatDevice& rDevice, vk::Extent2D extent, std::shared_ptr< CatSwapChain > previous )
	: m_rDevice{ rDevice }, windowExtent{ extent }, oldSwapChain{ previous }
{
	init();
	oldSwapChain = nullptr;
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
		vkDestroyImageView( m_rDevice.getDevice(), imageView, nullptr );
	}
	m_aSwapChainImageViews.clear();

	if ( !swapChain )
	{
		vkDestroySwapchainKHR( m_rDevice.getDevice(), swapChain, nullptr );
		swapChain = nullptr;
	}

	for ( int i = 0; i < m_aDepthImages.size(); i++ )
	{
		vkDestroyImageView( m_rDevice.getDevice(), m_aDepthImageViews[i], nullptr );
		vkDestroyImage( m_rDevice.getDevice(), m_aDepthImages[i], nullptr );
		vkFreeMemory( m_rDevice.getDevice(), m_aDepthImageMemorys[i], nullptr );
	}

	for ( auto framebuffer : m_aSwapChainFramebuffers )
	{
		vkDestroyFramebuffer( m_rDevice.getDevice(), framebuffer, nullptr );
	}

	vkDestroyRenderPass( m_rDevice.getDevice(), m_pRenderPass, nullptr );

	// cleanup synchronization objects
	for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
	{
		vkDestroySemaphore( m_rDevice.getDevice(), renderFinishedSemaphores[i], nullptr );
		vkDestroySemaphore( m_rDevice.getDevice(), imageAvailableSemaphores[i], nullptr );
		vkDestroyFence( m_rDevice.getDevice(), inFlightFences[i], nullptr );
	}
}

vk::Result CatSwapChain::acquireNextImage( uint32_t* imageIndex )
{
	m_rDevice.getDevice().waitForFences( 1, &inFlightFences[m_nCurrentFrame], true, std::numeric_limits< uint64_t >::max() );

	return m_rDevice.getDevice().acquireNextImageKHR(
		swapChain, std::numeric_limits< uint64_t >::max(), imageAvailableSemaphores[m_nCurrentFrame], nullptr, imageIndex );
}

vk::Result CatSwapChain::submitCommandBuffers( const vk::CommandBuffer* buffers, uint32_t* imageIndex )
{
	if ( !imagesInFlight[*imageIndex] )
	{
		m_rDevice.getDevice().waitForFences( 1, &imagesInFlight[*imageIndex], true, UINT64_MAX );
	}
	imagesInFlight[*imageIndex] = inFlightFences[m_nCurrentFrame];

	vk::SubmitInfo submitInfo = {
		.commandBufferCount = 1,
		.pCommandBuffers = buffers,
	};

	vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores[m_nCurrentFrame] };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[m_nCurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	m_rDevice.getDevice().resetFences( 1, &inFlightFences[m_nCurrentFrame] );
	if ( m_rDevice.getGraphicsQueue().submit( 1, &submitInfo, inFlightFences[m_nCurrentFrame] ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to submit draw command buffer!" );
	}

	vk::PresentInfoKHR presentInfo = {
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores,
		.pImageIndices = imageIndex,
	};
	vk::SwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	auto result = m_rDevice.getPresentQueue().presentKHR( &presentInfo );

	m_nCurrentFrame = ( m_nCurrentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;

	return result;
}

void CatSwapChain::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = m_rDevice.getSwapChainSupport();

	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat( swapChainSupport.m_aFormats );
	vk::PresentModeKHR presentMode = chooseSwapPresentMode( swapChainSupport.m_aPresentModes );
	vk::Extent2D extent = chooseSwapExtent( swapChainSupport.m_capabilities );

	uint32_t imageCount = swapChainSupport.m_capabilities.minImageCount + 1;
	if ( swapChainSupport.m_capabilities.maxImageCount > 0 && imageCount > swapChainSupport.m_capabilities.maxImageCount )
	{
		imageCount = swapChainSupport.m_capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo = {
		.surface = m_rDevice.getSurface(),
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.preTransform = swapChainSupport.m_capabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = presentMode,
		.clipped = true,
	};

	QueueFamilyIndices indices = m_rDevice.findPhysicalQueueFamilies();
	uint32_t queueFamilyIndices[] = { indices.m_nGraphicsFamily.value(), indices.m_nPresentFamily.value() };

	if ( indices.m_nGraphicsFamily != indices.m_nPresentFamily )
	{
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0;	  // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

	if ( m_rDevice.getDevice().createSwapchainKHR( &createInfo, nullptr, &swapChain ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create swap chain!" );
	}

	// we only specified a minimum number of images in the swap chain, so the implementation is
	// allowed to create a swap chain with more. That's why we'll first query the final number of
	// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
	// retrieve the handles.
	m_rDevice.getDevice().getSwapchainImagesKHR( swapChain, &imageCount, nullptr );
	m_aSwapChainImages.resize( imageCount );
	m_rDevice.getDevice().getSwapchainImagesKHR( swapChain, &imageCount, m_aSwapChainImages.data() );

	m_pSwapChainImageFormat = surfaceFormat.format;
	m_pSwapChainExtent = extent;
}

void CatSwapChain::createImageViews()
{
	m_aSwapChainImageViews.resize( m_aSwapChainImages.size() );
	for ( size_t i = 0; i < m_aSwapChainImages.size(); i++ )
	{
		vk::ImageViewCreateInfo viewInfo{
			.image = m_aSwapChainImages[i],
			.viewType = vk::ImageViewType::e2D,
			.format = m_pSwapChainImageFormat,
			.subresourceRange =
				{
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
		};

		if ( m_rDevice.getDevice().createImageView( &viewInfo, nullptr, &m_aSwapChainImageViews[i] ) != vk::Result::eSuccess )
		{
			throw std::runtime_error( "failed to create texture image view!" );
		}
	}
}

void CatSwapChain::createRenderPass()
{
	vk::AttachmentDescription depthAttachment{
		.format = findDepthFormat(),
		.samples = vk::SampleCountFlagBits::e1,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eDontCare,
		.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
		.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
		.initialLayout = vk::ImageLayout::eUndefined,
		.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
	};

	vk::AttachmentReference depthAttachmentRef{
		.attachment = 1,
		.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
	};

	vk::AttachmentDescription colorAttachment = {
		.format = getSwapChainImageFormat(),
		.samples = vk::SampleCountFlagBits::e1,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
		.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
		.initialLayout = vk::ImageLayout::eUndefined,
		.finalLayout = vk::ImageLayout::ePresentSrcKHR,
	};

	vk::AttachmentReference colorAttachmentRef = {
		.attachment = 0,
		.layout = vk::ImageLayout::eColorAttachmentOptimal,
	};

	vk::SubpassDescription subpass = {
		.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
		.pDepthStencilAttachment = &depthAttachmentRef,
	};

	vk::SubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
		.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
		.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eColorAttachmentWrite,
	};

	std::array< vk::AttachmentDescription, 2 > attachments = { colorAttachment, depthAttachment };
	vk::RenderPassCreateInfo renderPassInfo = {
		.attachmentCount = static_cast< uint32_t >( attachments.size() ),
		.pAttachments = attachments.data(),
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency,
	};

	if ( m_rDevice.getDevice().createRenderPass( &renderPassInfo, nullptr, &m_pRenderPass ) != vk::Result::eSuccess )
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
		vk::FramebufferCreateInfo framebufferInfo = {
			.renderPass = m_pRenderPass,
			.attachmentCount = static_cast< uint32_t >( attachments.size() ),
			.pAttachments = attachments.data(),
			.width = swapChainExtent.width,
			.height = swapChainExtent.height,
			.layers = 1,
		};

		if ( m_rDevice.getDevice().createFramebuffer( &framebufferInfo, nullptr, &m_aSwapChainFramebuffers[i] )
			 != vk::Result::eSuccess )
		{
			throw std::runtime_error( "failed to create framebuffer!" );
		}
	}
}

void CatSwapChain::createDepthResources()
{
	vk::Format depthFormat = findDepthFormat();
	m_pSwapChainDepthFormat = depthFormat;
	vk::Extent2D swapChainExtent = getSwapChainExtent();

	m_aDepthImages.resize( imageCount() );
	m_aDepthImageMemorys.resize( imageCount() );
	m_aDepthImageViews.resize( imageCount() );

	for ( int i = 0; i < m_aDepthImages.size(); i++ )
	{
		vk::ImageCreateInfo imageInfo{
			.imageType = vk::ImageType::e2D,
			.format = depthFormat,
			.extent =
				{
					swapChainExtent.width,
					swapChainExtent.height,
					1,
				},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = vk::SampleCountFlagBits::e1,
			.tiling = vk::ImageTiling::eOptimal,
			.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
			.sharingMode = vk::SharingMode::eExclusive,
			.initialLayout = vk::ImageLayout::eUndefined,
		};

		m_rDevice.createImageWithInfo(
			imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, m_aDepthImages[i], m_aDepthImageMemorys[i] );

		vk::ImageViewCreateInfo viewInfo{
			.image = m_aDepthImages[i],
			.viewType = vk::ImageViewType::e2D,
			.format = depthFormat,
			.subresourceRange =
				{
					vk::ImageAspectFlagBits::eDepth,
					0,
					1,
					0,
					1,
				},
		};

		if ( m_rDevice.getDevice().createImageView( &viewInfo, nullptr, &m_aDepthImageViews[i] ) != vk::Result::eSuccess )
		{
			throw std::runtime_error( "failed to create texture image view!" );
		}
	}
}

void CatSwapChain::createSyncObjects()
{
	imageAvailableSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
	renderFinishedSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
	inFlightFences.resize( MAX_FRAMES_IN_FLIGHT );
	imagesInFlight.resize( imageCount(), VK_NULL_HANDLE );

	vk::SemaphoreCreateInfo semaphoreInfo = {};

	vk::FenceCreateInfo fenceInfo = {
		.flags = vk::FenceCreateFlagBits::eSignaled,
	};

	for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
	{
		if ( m_rDevice.getDevice().createSemaphore( &semaphoreInfo, nullptr, &imageAvailableSemaphores[i] )
				 != vk::Result::eSuccess
			 || m_rDevice.getDevice().createSemaphore( &semaphoreInfo, nullptr, &renderFinishedSemaphores[i] )
					!= vk::Result::eSuccess
			 || m_rDevice.getDevice().createFence( &fenceInfo, nullptr, &inFlightFences[i] ) != vk::Result::eSuccess )
		{
			throw std::runtime_error( "failed to create synchronization objects for a frame!" );
		}
	}
}

vk::SurfaceFormatKHR CatSwapChain::chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >& availableFormats )
{
	for ( const auto& availableFormat : availableFormats )
	{
		if ( availableFormat.format == vk::Format::eB8G8R8A8Srgb
			 && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
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
		if ( availablePresentMode == SELECTED_PRESENTMODE )
		{
			DLOG_F( INFO, "Using selected present mode: ", SELECTED_PRESENTMODE_TEXT );
			return availablePresentMode;
		}
	}

	DLOG_F( INFO, "Using default present mode: V-Sync" );
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D CatSwapChain::chooseSwapExtent( const vk::SurfaceCapabilitiesKHR& capabilities )
{
	if ( capabilities.currentExtent.width != std::numeric_limits< uint32_t >::max() )
	{
		return capabilities.currentExtent;
	}
	else
	{
		vk::Extent2D actualExtent = windowExtent;
		actualExtent.width =
			std::max( capabilities.minImageExtent.width, std::min( capabilities.maxImageExtent.width, actualExtent.width ) );
		actualExtent.height =
			std::max( capabilities.minImageExtent.height, std::min( capabilities.maxImageExtent.height, actualExtent.height ) );

		return actualExtent;
	}
}

vk::Format CatSwapChain::findDepthFormat()
{
	return m_rDevice.findSupportedFormat( { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment );
}
} // namespace cat
