#ifndef CATENGINE_CATPIPELINE_HPP
#define CATENGINE_CATPIPELINE_HPP

#include "CatDevice.hpp"

#include <string>
#include <vector>

namespace cat
{
struct PipelineConfigInfo
{
	PipelineConfigInfo(){};
	PipelineConfigInfo( const PipelineConfigInfo& ) = delete;
	PipelineConfigInfo& operator=( const PipelineConfigInfo& ) = delete;

	vk::PipelineViewportStateCreateInfo m_pViewportInfo;
	vk::PipelineInputAssemblyStateCreateInfo m_pInputAssemblyInfo;
	vk::PipelineRasterizationStateCreateInfo m_pRasterizationInfo;
	vk::PipelineMultisampleStateCreateInfo m_pMultisampleInfo;
	vk::PipelineColorBlendAttachmentState m_pColorBlendAttachment;
	vk::PipelineColorBlendStateCreateInfo m_pColorBlendInfo;
	vk::PipelineDepthStencilStateCreateInfo m_pDepthStencilInfo;
	std::vector< vk::DynamicState > m_aDynamicStateEnables;
	vk::PipelineDynamicStateCreateInfo m_pDynamicStateInfo;
	vk::PipelineLayout m_pPipelineLayout = nullptr;
	vk::RenderPass m_pRenderPass = nullptr;
	uint32_t m_nSubpass = 0;
};

class CatPipeline
{
public:
	CatPipeline( CatDevice& device,
		const std::string& vertFilepath,
		const std::string& fragFilepath,
		const PipelineConfigInfo& configInfo );
	~CatPipeline();

	CatPipeline( const CatPipeline& ) = delete;
	CatPipeline& operator=( const CatPipeline& ) = delete;

	void bind( vk::CommandBuffer commandBuffer );

	static void defaultPipelineConfigInfo( PipelineConfigInfo& configInfo );

private:
	static std::vector< char > readFile( const std::string& filepath );

	void createGraphicsPipeline( const std::string& vertFilepath,
		const std::string& fragFilepath,
		const PipelineConfigInfo& configInfo );

	void createShaderModule( const std::vector< char >& code, vk::ShaderModule* shaderModule );

	CatDevice& m_rDevice;
	vk::Pipeline m_pGraphicsPipeline;
	vk::ShaderModule m_pVertShaderModule;
	vk::ShaderModule m_pFragShaderModule;
};
} // namespace cat


#endif // CATENGINE_CATPIPELINE_HPP
