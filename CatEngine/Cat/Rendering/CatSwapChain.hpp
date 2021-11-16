#ifndef CATENGINE_CATSWAPCHAIN_HPP
#define CATENGINE_CATSWAPCHAIN_HPP

#include "Cat/Rendering/CatDevice.hpp"

namespace cat
{
class CatSwapChain
{
public:
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	CatSwapChain( CatDevice& rDevice, vk::Extent2D windowExtent );
	CatSwapChain( CatDevice& rDevice, vk::Extent2D windowExtent, std::shared_ptr< CatSwapChain > previous );

	~CatSwapChain();

	CatSwapChain( const CatSwapChain& ) = delete;
	CatSwapChain& operator=( const CatSwapChain& ) = delete;

	vk::Framebuffer getFrameBuffer( int index ) { return m_aSwapChainFramebuffers[index]; }
	vk::RenderPass getRenderPass() { return m_pRenderPass; }
	vk::ImageView getImageView( int index ) { return m_aSwapChainImageViews[index]; }
	size_t imageCount() { return m_aSwapChainImages.size(); }
	vk::Format getSwapChainImageFormat() { return m_pSwapChainImageFormat; }
	vk::Extent2D getSwapChainExtent() { return m_pSwapChainExtent; }
	uint32_t width() { return m_pSwapChainExtent.width; }
	uint32_t height() { return m_pSwapChainExtent.height; }

	float extentAspectRatio()
	{
		return static_cast< float >( m_pSwapChainExtent.width ) / static_cast< float >( m_pSwapChainExtent.height );
	}
	vk::Format findDepthFormat();

	vk::Result acquireNextImage( uint32_t* imageIndex );
	vk::Result submitCommandBuffers( const vk::CommandBuffer* buffers, uint32_t* imageIndex );

	bool compareSwapFormats( const CatSwapChain& swapChain ) const
	{
		return swapChain.m_pSwapChainDepthFormat == m_pSwapChainDepthFormat
			   && swapChain.m_pSwapChainImageFormat == m_pSwapChainImageFormat;
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

	vk::Format m_pSwapChainImageFormat;
	vk::Format m_pSwapChainDepthFormat;
	vk::Extent2D m_pSwapChainExtent;

	std::vector< vk::Framebuffer > m_aSwapChainFramebuffers;
	vk::RenderPass m_pRenderPass;

	std::vector< vk::Image > m_aDepthImages;
	std::vector< vk::DeviceMemory > m_aDepthImageMemorys;
	std::vector< vk::ImageView > m_aDepthImageViews;
	std::vector< vk::Image > m_aSwapChainImages;
	std::vector< vk::ImageView > m_aSwapChainImageViews;

	CatDevice& m_rDevice;
	vk::Extent2D windowExtent;

	vk::SwapchainKHR swapChain;
	std::shared_ptr< CatSwapChain > oldSwapChain;

	std::vector< vk::Semaphore > imageAvailableSemaphores;
	std::vector< vk::Semaphore > renderFinishedSemaphores;
	std::vector< vk::Fence > inFlightFences;
	std::vector< vk::Fence > imagesInFlight;
	uint64_t m_nCurrentFrame = 0;
};
} // namespace cat

#endif // CATENGINE_CATSWAPCHAIN_HPP
