#include "CatSimpleRenderSystem.hpp"

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

CatSimpleRenderSystem::CatSimpleRenderSystem( CatDevice& device,
	vk::RenderPass renderPass,
	vk::DescriptorSetLayout globalSetLayout )
	: m_rDevice{ device }
{
	createPipelineLayout( globalSetLayout );
	createPipeline( renderPass );
}

CatSimpleRenderSystem::~CatSimpleRenderSystem()
{
	vkDestroyPipelineLayout( m_rDevice.getDevice(), m_pPipelineLayout, nullptr );
}

void CatSimpleRenderSystem::createPipelineLayout( vk::DescriptorSetLayout globalSetLayout )
{
	vk::PushConstantRange pushConstantRange{
		.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
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

void CatSimpleRenderSystem::createPipeline( vk::RenderPass renderPass )
{
	assert( !!m_pPipelineLayout && "Cannot create pipeline before pipeline layout" );

	PipelineConfigInfo pipelineConfig{};
	CatPipeline::defaultPipelineConfigInfo( pipelineConfig );
	pipelineConfig.m_pRenderPass = renderPass;
	pipelineConfig.m_pPipelineLayout = m_pPipelineLayout;
	m_pPipeline = std::make_unique< CatPipeline >(
		m_rDevice, "assets/shaders/simple_shader_2.vert.spv", "assets/shaders/simple_shader_2.frag.spv", pipelineConfig );
}

void CatSimpleRenderSystem::renderObjects( const CatFrameInfo& frameInfo )
{
	m_pPipeline->bind( frameInfo.m_pCommandBuffer );

	frameInfo.m_pCommandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, m_pPipelineLayout, 0, 1, &frameInfo.m_pGlobalDescriptorSet, 0, nullptr );

	for ( auto& [key, obj] : frameInfo.m_mObjects )
	{
		if ( obj->m_pModel == nullptr ) continue;
		if ( dynamic_cast< CatVolume* >( obj.get() ) != nullptr ) continue;
		CatPushConstantData push{};
		push.m_mxModel = obj->m_transform.mat4();
		push.m_mxNormal = obj->m_transform.normalMatrix();

		frameInfo.m_pCommandBuffer.pushConstants( m_pPipelineLayout,
			vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof( CatPushConstantData ), &push );
		obj->m_pModel->bind( frameInfo.m_pCommandBuffer );
		obj->m_pModel->draw( frameInfo.m_pCommandBuffer );
	}
}
} // namespace cat
