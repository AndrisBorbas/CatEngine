#include "CatWireframeRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>

#include "Cat/Objects/CatVolume.hpp"

namespace cat
{
struct CatPushConstantData
{
	glm::mat4 m_mxModel{ 1.f };
	glm::mat4 m_mxNormal{ 1.f };
};

CatWireframeRenderSystem::CatWireframeRenderSystem( CatDevice& device,
	vk::RenderPass renderPass,
	vk::DescriptorSetLayout globalSetLayout )
	: m_rDevice{ device }
{
	createPipelineLayout( globalSetLayout );
	createPipeline( renderPass );
}

CatWireframeRenderSystem::~CatWireframeRenderSystem()
{
	vkDestroyPipelineLayout( m_rDevice.getDevice(), m_pPipelineLayout, nullptr );
}

void CatWireframeRenderSystem::createPipelineLayout( vk::DescriptorSetLayout globalSetLayout )
{
	vk::PushConstantRange pushConstantRange{
		.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eFragment,
		.offset = 0,
		.size = sizeof( CatPushConstantData ),
	};

	std::vector< vk::DescriptorSetLayout > descriptorSetLayouts{ globalSetLayout };

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
		.setLayoutCount = static_cast< uint32_t >( descriptorSetLayouts.size() ),
		.pSetLayouts = descriptorSetLayouts.data(),
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstantRange,
	};

	if ( m_rDevice.getDevice().createPipelineLayout( &pipelineLayoutInfo, nullptr, &m_pPipelineLayout )
		 != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create pipeline layout!" );
	}
}

void CatWireframeRenderSystem::createPipeline( vk::RenderPass renderPass )
{
	assert( !!m_pPipelineLayout && "Cannot create pipeline before pipeline layout" );

	PipelineConfigInfo pipelineConfig{};
	CatPipeline::defaultPipelineConfigInfo( pipelineConfig );
	CatPipeline::enableAlphaBlending( pipelineConfig );
	CatPipeline::enableWireframe( pipelineConfig );
	pipelineConfig.m_pRenderPass = renderPass;
	pipelineConfig.m_pPipelineLayout = m_pPipelineLayout;
	m_pPipeline = std::make_unique< CatPipeline >(
		m_rDevice, "assets/shaders/simple_shader_2.vert.spv", "assets/shaders/simple_shader_2.frag.spv", pipelineConfig );
}

void CatWireframeRenderSystem::renderObjects( const CatFrameInfo& frameInfo )
{
	m_pPipeline->bind( frameInfo.m_pCommandBuffer );

	frameInfo.m_pCommandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, m_pPipelineLayout, 0, 1, &frameInfo.m_pGlobalDescriptorSet, 0, nullptr );

	for ( auto& [key, obj] : frameInfo.m_mObjects )
	{
		if ( obj->m_pModel == nullptr ) continue;
		if ( auto volume = dynamic_cast< CatVolume* >( obj.get() ) )
		{
			CatPushConstantData push{};
			push.m_mxModel = volume->m_transform.mat4();
			push.m_mxNormal = volume->m_transform.normalMatrix();

			frameInfo.m_pCommandBuffer.pushConstants( m_pPipelineLayout,
				vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof( CatPushConstantData ),
				&push );
			volume->m_pModel->bind( frameInfo.m_pCommandBuffer );
			volume->m_pModel->draw( frameInfo.m_pCommandBuffer );
		}
	}
}
} // namespace cat
