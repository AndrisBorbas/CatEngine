#ifndef CATENGINE_CATRENDERSYSTEM_HPP
#define CATENGINE_CATRENDERSYSTEM_HPP

#include "CatDevice.hpp"
#include "CatCamera.hpp"
#include "CatFrameInfo.hpp"
#include "CatObject.hpp"
#include "CatPipeline.hpp"

#include <memory>
#include <vector>

namespace cat
{
class CatRenderSystem
{
public:
	CatRenderSystem( CatDevice& device, vk::RenderPass renderPass, vk::DescriptorSetLayout globalSetLayout );
	~CatRenderSystem();

	CatRenderSystem( const CatRenderSystem& ) = delete;
	CatRenderSystem& operator=( const CatRenderSystem& ) = delete;

	void renderObjects( CatFrameInfo& frameInfo, std::vector< CatObject >& gameObjects );

private:
	void createPipelineLayout( vk::DescriptorSetLayout globalSetLayout );
	void createPipeline( vk::RenderPass renderPass );

	CatDevice& m_rDevice;

	std::unique_ptr< CatPipeline > m_pPipeline;
	vk::PipelineLayout m_pPipelineLayout;
};
} // namespace cat


#endif // CATENGINE_CATRENDERSYSTEM_HPP
