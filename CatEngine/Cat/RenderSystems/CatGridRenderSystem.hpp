#ifndef CATENGINE_CATGRIDRENDERSYSTEM_HPP
#define CATENGINE_CATGRIDRENDERSYSTEM_HPP

#include "Cat/VulkanRHI/CatDevice.hpp"
#include "Cat/Controller/CatCamera.hpp"
#include "Cat/CatFrameInfo.hpp"
#include "Cat/Objects/CatObject.hpp"
#include "Cat/VulkanRHI/CatPipeline.hpp"

#include <memory>
#include <vector>

namespace cat
{
class CatGridRenderSystem
{
public:
	CatGridRenderSystem( CatDevice* pDevice, vk::RenderPass renderPass, vk::DescriptorSetLayout globalSetLayout );
	~CatGridRenderSystem();

	CatGridRenderSystem( const CatGridRenderSystem& ) = delete;
	CatGridRenderSystem& operator=( const CatGridRenderSystem& ) = delete;

	void renderObjects( const CatFrameInfo& frameInfo );

private:
	void createPipelineLayout( vk::DescriptorSetLayout globalSetLayout );
	void createPipeline( vk::RenderPass renderPass );

	CatDevice* m_pDevice;

	std::unique_ptr< CatPipeline > m_pPipeline;
	vk::PipelineLayout m_pPipelineLayout;
};
} // namespace cat

#endif // CATENGINE_CATGRIDRENDERSYSTEM_HPP
