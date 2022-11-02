#ifndef CATENGINE_CATWIREFRAMERENDERSYSTEM_HPP
#define CATENGINE_CATWIREFRAMERENDERSYSTEM_HPP

#pragma once

#include "Cat/VulkanRHI/CatDevice.hpp"
#include "Cat/Controller/CatCamera.hpp"
#include "Cat/CatFrameInfo.hpp"
#include "Cat/Objects/CatObject.hpp"
#include "Cat/VulkanRHI/CatPipeline.hpp"

#include <memory>
#include <vector>

namespace cat
{
class CatWireframeRenderSystem
{
public:
	CatWireframeRenderSystem( CatDevice* pDevice, vk::RenderPass renderPass, vk::DescriptorSetLayout globalSetLayout );
	~CatWireframeRenderSystem();

	CatWireframeRenderSystem( const CatWireframeRenderSystem& ) = delete;
	CatWireframeRenderSystem& operator=( const CatWireframeRenderSystem& ) = delete;

	void renderObjects( const CatFrameInfo& frameInfo );

private:
	void createPipelineLayout( vk::DescriptorSetLayout globalSetLayout );
	void createPipeline( vk::RenderPass renderPass );

	CatDevice* m_pDevice;

	std::unique_ptr< CatPipeline > m_pPipeline;
	vk::PipelineLayout m_pPipelineLayout;
};
} // namespace cat

#endif // CATENGINE_CATWIREFRAMERENDERSYSTEM_HPP
