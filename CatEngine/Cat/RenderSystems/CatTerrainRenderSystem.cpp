#include "CatTerrainRenderSystem.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>

namespace cat
{
struct CatPushConstantData
{
	glm::mat4 m_mxModel{ 1.f };
	glm::mat4 m_mxNormal{ 1.f };
	glm::vec3 m_vColor{ -2.0f, -2.0f, -2.0f };
};

CatTerrainRenderSystem::CatTerrainRenderSystem( CatDevice* pDevice,
	vk::RenderPass renderPass,
	vk::DescriptorSetLayout descriptorSetLayout )
	: m_pDevice{ pDevice }
{
	createPipelineLayout( descriptorSetLayout );
	createPipeline( renderPass );
}

CatTerrainRenderSystem::~CatTerrainRenderSystem()
{
	( **m_pDevice ).destroy( m_pPipelineLayout );
}

void CatTerrainRenderSystem::createPipelineLayout( vk::DescriptorSetLayout descriptorSetLayout )
{
	vk::PushConstantRange pushConstantRange{
		.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
					  | vk::ShaderStageFlagBits::eTessellationControl | vk::ShaderStageFlagBits::eTessellationEvaluation,
		.offset = 0,
		.size = sizeof( CatPushConstantData ),
	};

	std::vector< vk::DescriptorSetLayout > descriptorSetLayouts{ descriptorSetLayout };

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
		.setLayoutCount = static_cast< uint32_t >( descriptorSetLayouts.size() ),
		.pSetLayouts = descriptorSetLayouts.data(),
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstantRange,
	};

	if ( ( **m_pDevice ).createPipelineLayout( &pipelineLayoutInfo, nullptr, &m_pPipelineLayout ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create pipeline layout!" );
	}
}

void CatTerrainRenderSystem::createPipeline( vk::RenderPass renderPass )
{
	assert( !!m_pPipelineLayout && "Cannot create pipeline before pipeline layout" );

	PipelineConfigInfo pipelineConfig{};
	CatPipeline::defaultPipelineConfigInfo( pipelineConfig );
	// CatPipeline::enableAlphaBlending( pipelineConfig );
	// CatPipeline::enableWireframe( pipelineConfig );
	CatPipeline::enableTessellation( pipelineConfig, 4 );
	pipelineConfig.m_pRenderPass = renderPass;
	pipelineConfig.m_pPipelineLayout = m_pPipelineLayout;
	pipelineConfig.m_aShaderStages = {
		CatPipeline::loadShader( m_pDevice, "assets/shaders/terrain/terrain.vert.spv", vk::ShaderStageFlagBits::eVertex ),
		CatPipeline::loadShader(
			m_pDevice, "assets/shaders/terrain/terrain.tesc.spv", vk::ShaderStageFlagBits::eTessellationControl ),
		CatPipeline::loadShader(
			m_pDevice, "assets/shaders/terrain/terrain.tese.spv", vk::ShaderStageFlagBits::eTessellationEvaluation ),
		CatPipeline::loadShader( m_pDevice, "assets/shaders/terrain/terrain.frag.spv", vk::ShaderStageFlagBits::eFragment ),
	};
	m_pPipeline = std::make_unique< CatPipeline >( m_pDevice, pipelineConfig );
}

void CatTerrainRenderSystem::render( const CatFrameInfo& rFrameInfo )
{
	if ( !rFrameInfo.m_pLevel->m_PTerrain ) return;

	m_pPipeline->bind( rFrameInfo.m_pCommandBuffer );

	rFrameInfo.m_pCommandBuffer.bindDescriptorSets( vk::PipelineBindPoint::eGraphics, m_pPipelineLayout, 0, 1,
		&rFrameInfo.m_pLevel->m_PTerrain->m_ADescriptorSets[rFrameInfo.m_nFrameIndex], 0, nullptr );

	rFrameInfo.m_pLevel->m_PTerrain->bind( rFrameInfo.m_pCommandBuffer );
	rFrameInfo.m_pLevel->m_PTerrain->draw( rFrameInfo.m_pCommandBuffer );
}

} // namespace cat
