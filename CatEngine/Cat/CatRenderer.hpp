#ifndef CATENGINE_CATRENDERER_HPP
#define CATENGINE_CATRENDERER_HPP

#include "CatDevice.hpp"
#include "CatSwapChain.hpp"
#include "CatWindow.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace cat
{
class CatRenderer
{
public:
	CatRenderer( CatWindow& window, CatDevice& device );
	~CatRenderer();

	CatRenderer( const CatRenderer& ) = delete;
	CatRenderer& operator=( const CatRenderer& ) = delete;

	vk::RenderPass getSwapChainRenderPass() const { return m_pSwapChain->getRenderPass(); }
	float getAspectRatio() const { return m_pSwapChain->extentAspectRatio(); }
	bool isFrameInProgress() const { return m_bIsFrameStarted; }

	vk::CommandBuffer getCurrentCommandBuffer() const
	{
		assert( m_bIsFrameStarted && "Cannot get command buffer when frame not in progress" );
		return m_pCommandBuffers[m_nCurrentFrameIndex];
	}

	uint64_t getFrameIndex() const
	{
		assert( m_bIsFrameStarted && "Cannot get frame index when frame not in progress" );
		return m_nCurrentFrameIndex;
	}

	vk::CommandBuffer beginFrame();
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
	uint64_t m_nCurrentFrameIndex{ 0 };
	bool m_bIsFrameStarted{ false };
};
} // namespace cat


#endif // CATENGINE_CATRENDERER_HPP
