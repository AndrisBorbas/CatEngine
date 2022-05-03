#ifndef CATENGINE_CATRENDERER_HPP
#define CATENGINE_CATRENDERER_HPP

#include "CatDevice.hpp"
#include "CatSwapChain.hpp"
#include "Cat/CatWindow.hpp"

#include <cassert>
#include <memory>
#include <vector>

#include "loguru.hpp"

namespace cat
{
class CatRenderer
{
public:
	CatRenderer( CatWindow& window, CatDevice& device );
	~CatRenderer();

	CatRenderer( const CatRenderer& ) = delete;
	CatRenderer& operator=( const CatRenderer& ) = delete;

	[[nodiscard]] vk::RenderPass getSwapChainRenderPass() const { return m_pSwapChain->getRenderPass(); }
	[[nodiscard]] float getAspectRatio() const { return m_pSwapChain->extentAspectRatio(); }
	[[nodiscard]] size_t getImageCount() const { return m_pSwapChain->getImageCount(); }
	[[nodiscard]] bool isFrameInProgress() const { return m_bIsFrameStarted; }

	[[nodiscard]] vk::CommandBuffer getCurrentCommandBuffer() const
	{
		CHECK_F( m_bIsFrameStarted, "Cannot get command buffer when frame not in progress" );
		return m_pCommandBuffers[m_nCurrentFrameIndex];
	}

	[[nodiscard]] short getFrameIndex() const
	{
		CHECK_F( m_bIsFrameStarted, "Cannot get frame index when frame not in progress" );
		return m_nCurrentFrameIndex;
	}

	[[nodiscard]] uint64_t getFrameNumber() const { return m_nFrameNumber; }

	[[nodiscard]] vk::CommandBuffer beginFrame();
	void endFrame();
	void beginSwapChainRenderPass( vk::CommandBuffer commandBuffer );
	void endSwapChainRenderPass( vk::CommandBuffer commandBuffer );

private:
	void createCommandBuffers();
	void freeCommandBuffers();
	void recreateSwapChain();

	CatWindow& m_rWindow;
	CatDevice& m_rDevice;
	std::unique_ptr< CatSwapChain > m_pSwapChain;
	std::vector< vk::CommandBuffer > m_pCommandBuffers;

	uint32_t m_nCurrentImageIndex;
	short m_nCurrentFrameIndex{ 0 };
	uint64_t m_nFrameNumber{ 0 };
	bool m_bIsFrameStarted{ false };
};
} // namespace cat


#endif // CATENGINE_CATRENDERER_HPP
