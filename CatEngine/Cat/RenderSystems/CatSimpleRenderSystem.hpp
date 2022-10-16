#ifndef CATENGINE_CATSIMPLERENDERSYSTEM_HPP
#define CATENGINE_CATSIMPLERENDERSYSTEM_HPP

#include "Cat/Rendering/CatDevice.hpp"
#include "Cat/Controller/CatCamera.hpp"
#include "Cat/CatFrameInfo.hpp"
#include "Cat/Objects/CatObject.hpp"
#include "Cat/Rendering/CatPipeline.hpp"

#include <memory>
#include <vector>

namespace cat
{
class CatSimpleRenderSystem
{
public:
	CatSimpleRenderSystem( CatDevice& device, vk::RenderPass renderPass, vk::DescriptorSetLayout globalSetLayout );
	~CatSimpleRenderSystem();

	CatSimpleRenderSystem( const CatSimpleRenderSystem& ) = delete;
	CatSimpleRenderSystem& operator=( const CatSimpleRenderSystem& ) = delete;

	void renderObjects( const CatFrameInfo& frameInfo );

private:
	void createPipelineLayout( vk::DescriptorSetLayout globalSetLayout );
	void createPipeline( vk::RenderPass renderPass );

	CatDevice& m_rDevice;

	std::unique_ptr< CatPipeline > m_pPipeline;
	vk::PipelineLayout m_pPipelineLayout;
};
} // namespace cat


#endif // CATENGINE_CATSIMPLERENDERSYSTEM_HPP
