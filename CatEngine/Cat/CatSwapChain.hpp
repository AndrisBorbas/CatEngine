#pragma once

#include "Defines.h"

#include "CatDevice.hpp"

#include <memory>
#include <string>
#include <vector>

namespace cat
{

class CatSwapChain
{
public:
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	CatSwapChain( CatDevice* pDevice, vk::Extent2D windowExtent );
	CatSwapChain( CatDevice* pDevice, vk::Extent2D windowExtent, std::shared_ptr< CatSwapChain > pPrevious );

	~CatSwapChain();

	CatSwapChain( const CatSwapChain& ) = delete;
	CatSwapChain& operator=( const CatSwapChain& ) = delete;

	vk::Framebuffer getFrameBuffer( const int index ) { return m_aSwapChainFramebuffers[index]; }
	vk::RenderPass getRenderPass() { return m_renderPass; }
	vk::ImageView getImageView( const int index ) { return m_aSwapChainImageViews[index]; }
	size_t imageCount() { return m_aSwapChainImages.size(); }
	vk::Format getSwapChainImageFormat() { return m_swapChainImageFormat; }
	vk::Extent2D getSwapChainExtent() { return m_swapChainExtent; }
	uint32_t width() { return m_swapChainExtent.width; }
	uint32_t height() { return m_swapChainExtent.height; }

	float extentAspectRatio()
	{
		return static_cast< float >( m_swapChainExtent.width ) / static_cast< float >( m_swapChainExtent.height );
	}
	vk::Format findDepthFormat();

	vk::Result acquireNextImage( uint32_t* imageIndex );
	vk::Result submitCommandBuffers( const vk::CommandBuffer* buffers, uint32_t* imageIndex );

	bool compareSwapFormats( const CatSwapChain& swapChain ) const
	{
		return swapChain.m_swapChainDepthFormat == m_swapChainDepthFormat
			   && swapChain.m_swapChainImageFormat == m_swapChainImageFormat;
	}

private:
	void init();
	void createSwapChain();
	void createImageViews();
	void createDepthResources();
	void createRenderPass();
	void createFramebuffers();
	void createSyncObjects();

	// Helper functions
	vk::SurfaceFormatKHR chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >& availableFormats );
	vk::PresentModeKHR chooseSwapPresentMode( const std::vector< vk::PresentModeKHR >& availablePresentModes );
	vk::Extent2D chooseSwapExtent( const vk::SurfaceCapabilitiesKHR& capabilities );

	vk::Format m_swapChainImageFormat;
	vk::Format m_swapChainDepthFormat;
	vk::Extent2D m_swapChainExtent;

	std::vector< vk::Framebuffer > m_aSwapChainFramebuffers;
	vk::RenderPass m_renderPass;

	std::vector< vk::Image > m_aDepthImages;
	std::vector< vk::DeviceMemory > m_aDepthImageMemorys;
	std::vector< vk::ImageView > m_aDepthImageViews;
	std::vector< vk::Image > m_aSwapChainImages;
	std::vector< vk::ImageView > m_aSwapChainImageViews;

	CatDevice* m_pDevice;
	vk::Extent2D m_windowExtent;

	vk::SwapchainKHR m_swapChain;
	std::shared_ptr< CatSwapChain > m_pOldSwapChain;

	std::vector< vk::Semaphore > m_aImageAvailableSemaphores;
	std::vector< vk::Semaphore > m_aRenderFinishedSemaphores;
	std::vector< vk::Fence > m_aInFlightFences;
	std::vector< vk::Fence > m_aImagesInFlight;
	size_t m_nCurrentFrame = 0;
};

} // namespace cat
