#include "CatPointLightRenderSystem.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <map>
#include <stdexcept>

#include "loguru.hpp"

namespace cat
{
struct PointLightPushConstants
{
	glm::vec4 position{};
	glm::vec4 color{};
	float radius;
};

CatPointLightRenderSystem::CatPointLightRenderSystem( CatDevice& rDevice,
	vk::RenderPass pRenderPass,
	vk::DescriptorSetLayout pGlobalSetLayout )
	: m_rDevice{ rDevice }
{
	createPipelineLayout( pGlobalSetLayout );
	createPipeline( pRenderPass );
}

CatPointLightRenderSystem::~CatPointLightRenderSystem()
{
	m_rDevice.getDevice().destroyPipelineLayout( m_pPipelineLayout, nullptr );
}

void CatPointLightRenderSystem::createPipelineLayout( vk::DescriptorSetLayout pGlobalSetLayout )
{
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof( PointLightPushConstants );

	std::vector< vk::DescriptorSetLayout > descriptorSetLayouts{ pGlobalSetLayout };

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setLayoutCount = static_cast< uint32_t >( descriptorSetLayouts.size() );
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if ( m_rDevice.getDevice().createPipelineLayout( &pipelineLayoutInfo, nullptr, &m_pPipelineLayout )
		 != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create pipeline layout!" );
	}
}

void CatPointLightRenderSystem::createPipeline( vk::RenderPass pRenderPass )
{
	assert( !!m_pPipelineLayout && "Cannot create pipeline before pipeline layout" );

	PipelineConfigInfo pipelineConfig{};
	CatPipeline::defaultPipelineConfigInfo( pipelineConfig );
	CatPipeline::enableAlphaBlending( pipelineConfig );
	pipelineConfig.m_aAttributeDescriptions.clear();
	pipelineConfig.m_aBindingDescriptions.clear();
	pipelineConfig.m_pRenderPass = pRenderPass;
	pipelineConfig.m_pPipelineLayout = m_pPipelineLayout;
	m_pPipeline = std::make_unique< CatPipeline >(
		m_rDevice, "assets/shaders/point_light.vert.spv", "assets/shaders/point_light.frag.spv", pipelineConfig );
}

void CatPointLightRenderSystem::update( const CatFrameInfo& rFrameInfo, GlobalUbo& ubo, const bool bIsRotating ) const
{
	const auto rotateLight = glm::rotate( glm::mat4( 1.f ), 0.5f * rFrameInfo.m_fFrameTime, { 0.f, -1.f, 0.f } );
	int lightIndex = 0;
	for ( auto& [key, obj] : rFrameInfo.m_mObjects )
	{
		if ( obj.m_pPointLight == nullptr ) continue;

		CHECK_F( lightIndex < MAX_LIGHTS, "Point lights exceed maximum specified" );

		// update light position
		if ( bIsRotating )
		{
			obj.m_transform.translation = glm::vec3( rotateLight * glm::vec4( obj.m_transform.translation, 1.f ) );
		}

		// copy light to ubo
		ubo.pointLights[lightIndex].position = glm::vec4( obj.m_transform.translation, 1.f );
		ubo.pointLights[lightIndex].color = glm::vec4( obj.m_vColor, obj.m_pPointLight->lightIntensity );

		lightIndex += 1;
	}
	ubo.numLights = lightIndex;
}

void CatPointLightRenderSystem::render( const CatFrameInfo& rFrameInfo ) const
{
	std::map< float, CatObject::id_t > sorted;
	for ( auto& [key, obj] : rFrameInfo.m_mObjects )
	{
		if ( obj.m_pPointLight == nullptr ) continue;

		// calculate distance
		auto offset = rFrameInfo.m_rCamera.getPosition() - obj.m_transform.translation;
		float disSquared = glm::dot( offset, offset );
		sorted[disSquared] = obj.getId();
	}

	m_pPipeline->bind( rFrameInfo.m_pCommandBuffer );

	rFrameInfo.m_pCommandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, m_pPipelineLayout, 0, 1, &rFrameInfo.m_pGlobalDescriptorSet, 0, nullptr );

	// iterate through sorted lights in reverse order
	for ( auto it = sorted.rbegin(); it != sorted.rend(); ++it )
	{
		// use game obj id to find light object
		auto& obj = rFrameInfo.m_mObjects.at( it->second );

		PointLightPushConstants push{};
		push.position = glm::vec4( obj.m_transform.translation, 1.f );
		push.color = glm::vec4( obj.m_vColor, obj.m_pPointLight->lightIntensity );
		push.radius = obj.m_transform.scale.x;

		rFrameInfo.m_pCommandBuffer.pushConstants( m_pPipelineLayout,
			vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof( PointLightPushConstants ),
			&push );
		rFrameInfo.m_pCommandBuffer.draw( 6, 1, 0, 0 );
	}
}
} // namespace cat
