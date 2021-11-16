#include "CatPipeline.hpp"

#include "CatModel.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace cat
{
CatPipeline::CatPipeline( CatDevice& device,
	const std::string& vertFilepath,
	const std::string& fragFilepath,
	const PipelineConfigInfo& configInfo )
	: m_rDevice{ device }
{
	createGraphicsPipeline( vertFilepath, fragFilepath, configInfo );
}

CatPipeline::~CatPipeline()
{
	m_rDevice.getDevice().destroy( m_pVertShaderModule );
	m_rDevice.getDevice().destroy( m_pFragShaderModule );
	m_rDevice.getDevice().destroy( m_pGraphicsPipeline );
}

std::vector< char > CatPipeline::readFile( const std::string& filepath )
{
	std::ifstream file{ filepath, std::ios::ate | std::ios::binary };

	if ( !file.is_open() )
	{
		throw std::runtime_error( "failed to open file: " + filepath );
	}

	size_t fileSize = static_cast< size_t >( file.tellg() );
	std::vector< char > buffer( fileSize );

	file.seekg( 0 );
	file.read( buffer.data(), fileSize );

	file.close();
	return buffer;
}

void CatPipeline::createGraphicsPipeline( const std::string& vertFilepath,
	const std::string& fragFilepath,
	const PipelineConfigInfo& configInfo )
{
	assert( !!configInfo.m_pPipelineLayout && "Cannot create graphics pipeline: no pipelineLayout provided in configInfo" );
	assert( !!configInfo.m_pRenderPass && "Cannot create graphics pipeline: no renderPass provided in configInfo" );

	auto vertCode = readFile( vertFilepath );
	auto fragCode = readFile( fragFilepath );

	createShaderModule( vertCode, &m_pVertShaderModule );
	createShaderModule( fragCode, &m_pFragShaderModule );

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = m_pVertShaderModule,
		.pName = "main",
	};

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = m_pFragShaderModule,
		.pName = "main",
	};

	static const int nShaderStages = 2;

	vk::PipelineShaderStageCreateInfo shaderStages[nShaderStages] = { vertShaderStageInfo, fragShaderStageInfo };

	auto bindingDescriptions = CatModel::Vertex::getBindingDescriptions();
	auto attributeDescriptions = CatModel::Vertex::getAttributeDescriptions();
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
		.vertexBindingDescriptionCount = static_cast< uint32_t >( bindingDescriptions.size() ),
		.pVertexBindingDescriptions = bindingDescriptions.data(),
		.vertexAttributeDescriptionCount = static_cast< uint32_t >( attributeDescriptions.size() ),
		.pVertexAttributeDescriptions = attributeDescriptions.data(),
	};

	vk::GraphicsPipelineCreateInfo pipelineInfo{
		.stageCount = nShaderStages,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &configInfo.m_pInputAssemblyInfo,
		.pViewportState = &configInfo.m_pViewportInfo,
		.pRasterizationState = &configInfo.m_pRasterizationInfo,
		.pMultisampleState = &configInfo.m_pMultisampleInfo,
		.pDepthStencilState = &configInfo.m_pDepthStencilInfo,
		.pColorBlendState = &configInfo.m_pColorBlendInfo,
		.pDynamicState = &configInfo.m_pDynamicStateInfo,
		.layout = configInfo.m_pPipelineLayout,
		.renderPass = configInfo.m_pRenderPass,
		.subpass = configInfo.m_nSubpass,
		.basePipelineHandle = nullptr,
		.basePipelineIndex = -1,
	};

	if ( m_rDevice.getDevice().createGraphicsPipelines( nullptr, 1, &pipelineInfo, nullptr, &m_pGraphicsPipeline )
		 != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create graphics pipeline" );
	}
}

void CatPipeline::createShaderModule( const std::vector< char >& code, vk::ShaderModule* shaderModule )
{
	vk::ShaderModuleCreateInfo createInfo{
		.codeSize = code.size(),
		.pCode = reinterpret_cast< const uint32_t* >( code.data() ),
	};

	if ( m_rDevice.getDevice().createShaderModule( &createInfo, nullptr, shaderModule ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create shader module" );
	}
}

void CatPipeline::bind( vk::CommandBuffer commandBuffer )
{
	commandBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics, m_pGraphicsPipeline );
}

void CatPipeline::defaultPipelineConfigInfo( PipelineConfigInfo& configInfo )
{
	configInfo.m_pInputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
	configInfo.m_pInputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	configInfo.m_pViewportInfo.viewportCount = 1;
	configInfo.m_pViewportInfo.pViewports = nullptr;
	configInfo.m_pViewportInfo.scissorCount = 1;
	configInfo.m_pViewportInfo.pScissors = nullptr;

	configInfo.m_pRasterizationInfo.depthClampEnable = VK_FALSE;
	configInfo.m_pRasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	configInfo.m_pRasterizationInfo.polygonMode = vk::PolygonMode::eFill;
	configInfo.m_pRasterizationInfo.lineWidth = 1.0f;
	configInfo.m_pRasterizationInfo.cullMode = vk::CullModeFlagBits::eNone;
	configInfo.m_pRasterizationInfo.frontFace = vk::FrontFace::eClockwise;
	configInfo.m_pRasterizationInfo.depthBiasEnable = false;
	configInfo.m_pRasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
	configInfo.m_pRasterizationInfo.depthBiasClamp = 0.0f;			// Optional
	configInfo.m_pRasterizationInfo.depthBiasSlopeFactor = 0.0f;	// Optional

	configInfo.m_pMultisampleInfo.sampleShadingEnable = false;
	configInfo.m_pMultisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
	configInfo.m_pMultisampleInfo.minSampleShading = 1.0f;		 // Optional
	configInfo.m_pMultisampleInfo.pSampleMask = nullptr;		 // Optional
	configInfo.m_pMultisampleInfo.alphaToCoverageEnable = false; // Optional
	configInfo.m_pMultisampleInfo.alphaToOneEnable = false;		 // Optional

	configInfo.m_pColorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
														| vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	configInfo.m_pColorBlendAttachment.blendEnable = false;
	configInfo.m_pColorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;	 // Optional
	configInfo.m_pColorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero; // Optional
	configInfo.m_pColorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;			 // Optional
	configInfo.m_pColorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;	 // Optional
	configInfo.m_pColorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero; // Optional
	configInfo.m_pColorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;			 // Optional

	configInfo.m_pColorBlendInfo.logicOpEnable = false;
	configInfo.m_pColorBlendInfo.logicOp = vk::LogicOp::eCopy; // Optional
	configInfo.m_pColorBlendInfo.attachmentCount = 1;
	configInfo.m_pColorBlendInfo.pAttachments = &configInfo.m_pColorBlendAttachment;
	configInfo.m_pColorBlendInfo.blendConstants[0] = 0.0f; // Optional
	configInfo.m_pColorBlendInfo.blendConstants[1] = 0.0f; // Optional
	configInfo.m_pColorBlendInfo.blendConstants[2] = 0.0f; // Optional
	configInfo.m_pColorBlendInfo.blendConstants[3] = 0.0f; // Optional

	configInfo.m_pDepthStencilInfo.depthTestEnable = true;
	configInfo.m_pDepthStencilInfo.depthWriteEnable = true;
	configInfo.m_pDepthStencilInfo.depthCompareOp = vk::CompareOp::eLess;
	configInfo.m_pDepthStencilInfo.depthBoundsTestEnable = false;
	configInfo.m_pDepthStencilInfo.minDepthBounds = 0.0f; // Optional
	configInfo.m_pDepthStencilInfo.maxDepthBounds = 1.0f; // Optional
	configInfo.m_pDepthStencilInfo.stencilTestEnable = false;
	// configInfo.m_pDepthStencilInfo.front = {}; // Optional
	// configInfo.m_pDepthStencilInfo.back = {};  // Optional

	configInfo.m_aDynamicStateEnables = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	configInfo.m_pDynamicStateInfo.pDynamicStates = configInfo.m_aDynamicStateEnables.data();
	configInfo.m_pDynamicStateInfo.dynamicStateCount = static_cast< uint32_t >( configInfo.m_aDynamicStateEnables.size() );
}

} // namespace cat
