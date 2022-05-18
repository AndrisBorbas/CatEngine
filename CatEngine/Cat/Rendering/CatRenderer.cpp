#include "CatRenderer.hpp"

#include <array>
#include <cassert>
#include <stdexcept>


namespace cat
{
CatRenderer::CatRenderer( CatWindow& window, CatDevice& device ) : m_rWindow{ window }, m_rDevice{ device }
{
	recreateSwapChain();
	createCommandBuffers();
}

CatRenderer::~CatRenderer()
{
	freeCommandBuffers();
}

void CatRenderer::recreateSwapChain()
{
	auto extent = m_rWindow.getExtent();
	while ( extent.width == 0 || extent.height == 0 )
	{
		extent = m_rWindow.getExtent();
		glfwWaitEvents();
	}
	m_rDevice.getDevice().waitIdle();

	if ( m_pSwapChain == nullptr )
	{
		m_pSwapChain = std::make_unique< CatSwapChain >( m_rDevice, extent );
	}
	else
	{
		std::shared_ptr< CatSwapChain > oldSwapChain = std::move( m_pSwapChain );
		m_pSwapChain = std::make_unique< CatSwapChain >( m_rDevice, extent, oldSwapChain );

		if ( !oldSwapChain->compareSwapFormats( *m_pSwapChain.get() ) )
		{
			throw std::runtime_error( "Swap chain image(or depth) format has changed!" );
		}
	}
}

void CatRenderer::createCommandBuffers()
{
	m_pCommandBuffers.resize( CatSwapChain::MAX_FRAMES_IN_FLIGHT );

	vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = m_rDevice.getCommandPool(),
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = static_cast< uint32_t >( m_pCommandBuffers.size() ),
	};

	if ( m_rDevice.getDevice().allocateCommandBuffers( &allocInfo, m_pCommandBuffers.data() ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "Failed to allocate command buffers!" );
	}
}

void CatRenderer::freeCommandBuffers()
{
	m_rDevice.getDevice().free(
		m_rDevice.getCommandPool(), static_cast< uint32_t >( m_pCommandBuffers.size() ), m_pCommandBuffers.data() );
	m_pCommandBuffers.clear();
}

vk::CommandBuffer CatRenderer::beginFrame()
{
	assert( !m_bIsFrameStarted && "Can't call beginFrame while already in progress" );

	auto result = m_pSwapChain->acquireNextImage( &m_nCurrentImageIndex );
	if ( result == vk::Result::eErrorOutOfDateKHR )
	{
		recreateSwapChain();
		return nullptr;
	}

	if ( result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR )
	{
		throw std::runtime_error( "failed to acquire swap chain image!" );
	}

	m_bIsFrameStarted = true;

	auto commandBuffer = getCurrentCommandBuffer();
	vk::CommandBufferBeginInfo beginInfo{};

	if ( commandBuffer.begin( &beginInfo ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to begin recording command buffer!" );
	}
	return commandBuffer;
}

void CatRenderer::endFrame()
{
	CHECK_F( m_bIsFrameStarted && "Can't call endFrame while frame is not in progress" );
	auto commandBuffer = getCurrentCommandBuffer();
	commandBuffer.end();

	auto result = m_pSwapChain->submitCommandBuffers( &commandBuffer, &m_nCurrentImageIndex );
	if ( result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_rWindow.wasWindowResized() )
	{
		m_rWindow.resetWindowResizedFlag();
		recreateSwapChain();
	}
	else if ( result != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to present swap chain image!" );
	}

	m_bIsFrameStarted = false;
	m_nFrameNumber++;
	m_nCurrentFrameIndex = m_nFrameNumber % CatSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void CatRenderer::beginSwapChainRenderPass( vk::CommandBuffer commandBuffer )
{
	CHECK_F( m_bIsFrameStarted, "Can't call beginSwapChainRenderPass if frame is not in progress" );
	CHECK_F( commandBuffer == getCurrentCommandBuffer(), "Can't begin render pass on command buffer from a different frame" );

	std::array< vk::ClearValue, 3 > clearValues{};
	clearValues[0].setColor( std::array< float, 4 >{ 0.01f, 0.01f, 0.0125f, 1.0f } );
	clearValues[1].setDepthStencil( { 1.0f, 0 } );
	clearValues[2].setColor( std::array< float, 4 >{ 0.01f, 0.01f, 0.0125f, 1.0f } );

	vk::RenderPassBeginInfo renderPassInfo{
		.renderPass = m_pSwapChain->getRenderPass(),
		.framebuffer = m_pSwapChain->getFrameBuffer( m_nCurrentImageIndex ),
		.renderArea =
			vk::Rect2D{
				.offset = vk::Offset2D{ 0, 0 },
				.extent = m_pSwapChain->getSwapChainExtent(),
			},
		.clearValueCount = static_cast< uint32_t >( clearValues.size() ),
		.pClearValues = clearValues.data(),
	};

	commandBuffer.beginRenderPass( &renderPassInfo, vk::SubpassContents::eInline );

	vk::Viewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast< float >( m_pSwapChain->getSwapChainExtent().width ),
		.height = static_cast< float >( m_pSwapChain->getSwapChainExtent().height ),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	vk::Rect2D scissor{
		.offset = { 0, 0 },
		.extent = m_pSwapChain->getSwapChainExtent(),
	};
	commandBuffer.setViewport( 0, 1, &viewport );
	commandBuffer.setScissor( 0, 1, &scissor );
}

void CatRenderer::endSwapChainRenderPass( vk::CommandBuffer commandBuffer )
{
	CHECK_F( m_bIsFrameStarted, "Can't call endSwapChainRenderPass if frame is not in progress" );
	CHECK_F( commandBuffer == getCurrentCommandBuffer(), "Can't end render pass on command buffer from a different frame" );
	commandBuffer.endRenderPass();
}
} // namespace cat
