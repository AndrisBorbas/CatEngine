#ifndef CATENGINE_CATTERRAINRENDERSYSTEM_HPP
#define CATENGINE_CATTERRAINRENDERSYSTEM_HPP

#pragma once

#include "Cat/VulkanRHI/CatDevice.hpp"
#include "Cat/Controller/CatCamera.hpp"
#include "Cat/CatFrameInfo.hpp"
#include "Cat/Objects/CatObject.hpp"
#include "Cat/VulkanRHI/CatPipeline.hpp"
#include "Cat/Terrain/CatTerrain.hpp"

#include <memory>
#include <vector>

namespace cat
{

class CatTerrainRenderSystem
{
public:
	CatTerrainRenderSystem( CatDevice* pDevice, vk::RenderPass renderPass, vk::DescriptorSetLayout descriptorSetLayout );
	~CatTerrainRenderSystem();

	CatTerrainRenderSystem( const CatTerrainRenderSystem& ) = delete;
	CatTerrainRenderSystem& operator=( const CatTerrainRenderSystem& ) = delete;

	void render( const CatFrameInfo& rFrameInfo );

private:
	void createPipelineLayout( vk::DescriptorSetLayout descriptorSetLayout );
	void createPipeline( vk::RenderPass renderPass );

	CatDevice* m_pDevice;

	std::unique_ptr< CatPipeline > m_pPipeline;
	vk::PipelineLayout m_pPipelineLayout;
};

} // namespace cat

#endif // CATENGINE_CATTERRAINRENDERSYSTEM_HPP
