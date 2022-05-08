#ifndef CATENGINE_CATWIREFRAMERENDERSYSTEM_HPP
#define CATENGINE_CATWIREFRAMERENDERSYSTEM_HPP

#pragma once

#include "Cat/Rendering/CatDevice.hpp"
#include "Cat/Objects/CatCamera.hpp"
#include "Cat/CatFrameInfo.hpp"
#include "Cat/Objects/CatObject.hpp"
#include "Cat/Rendering/CatPipeline.hpp"

#include <memory>
#include <vector>

namespace cat
{
class CatWireframeRenderSystem
{
public:
	CatWireframeRenderSystem( CatDevice& device, vk::RenderPass renderPass, vk::DescriptorSetLayout globalSetLayout );
	~CatWireframeRenderSystem();

	CatWireframeRenderSystem( const CatWireframeRenderSystem& ) = delete;
	CatWireframeRenderSystem& operator=( const CatWireframeRenderSystem& ) = delete;

	void renderObjects( const CatFrameInfo& frameInfo );

private:
	void createPipelineLayout( vk::DescriptorSetLayout globalSetLayout );
	void createPipeline( vk::RenderPass renderPass );

	CatDevice& m_rDevice;

	std::unique_ptr< CatPipeline > m_pPipeline;
	vk::PipelineLayout m_pPipelineLayout;
};
} // namespace cat

#endif // CATENGINE_CATWIREFRAMERENDERSYSTEM_HPP
