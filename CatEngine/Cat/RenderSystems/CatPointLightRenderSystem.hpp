#ifndef CATENGINE_CATPOINTLIGHTRENDERSYSTEM_HPP
#define CATENGINE_CATPOINTLIGHTRENDERSYSTEM_HPP

#pragma once

#include "Cat/Objects/CatCamera.hpp"
#include "Cat/Rendering/CatDevice.hpp"
#include "Cat/CatFrameInfo.hpp"
#include "Cat/Objects/CatObject.hpp"
#include "Cat/Rendering/CatPipeline.hpp"

// std
#include <memory>
#include <vector>

namespace cat
{
class CatPointLightRenderSystem
{
public:
	CatPointLightRenderSystem( CatDevice& rDevice, vk::RenderPass pRenderPass, vk::DescriptorSetLayout pGlobalSetLayout );
	~CatPointLightRenderSystem();

	CatPointLightRenderSystem( const CatPointLightRenderSystem& ) = delete;
	CatPointLightRenderSystem& operator=( const CatPointLightRenderSystem& ) = delete;

	void update( const CatFrameInfo& rFrameInfo, GlobalUbo& ubo, bool bIsRotating ) const;
	void render( const CatFrameInfo& rFrameInfo ) const;

private:
	void createPipelineLayout( vk::DescriptorSetLayout pGlobalSetLayout );
	void createPipeline( vk::RenderPass pRenderPass );

	CatDevice& m_rDevice;

	std::unique_ptr< CatPipeline > m_pPipeline;
	vk::PipelineLayout m_pPipelineLayout;
};
} // namespace cat

#endif // CATENGINE_CATPOINTLIGHTRENDERSYSTEM_HPP
