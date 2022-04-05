#ifndef CATENGINE_CATFRAMEINFO_HPP
#define CATENGINE_CATFRAMEINFO_HPP

#include "Cat/Objects/CatCamera.hpp"
#include "Cat/Objects/CatObject.hpp"

#include "vulkan/vulkan.hpp"


namespace cat
{
struct GlobalUbo
{
	glm::mat4 projectionView{ 1.f };
	glm::vec4 ambientLightColor{ .8f, .8f, 1.f, .086f }; // w is intensity
	glm::vec3 lightPosition{ .8f, .9f, 1.5f };
	alignas( 16 ) glm::vec4 lightColor{ 1.f }; // w is light intensity
};

typedef struct CatFrameInfo_t
{
	uint64_t m_nFrameIndex = 0;
	float m_fFrameTime = 0.0f;
	vk::CommandBuffer m_pCommandBuffer;
	CatCamera& m_rCamera;
	vk::DescriptorSet m_pGlobalDescriptorSet;
	GlobalUbo& m_rUBO;
	CatObject::Map& m_mObjects;
	CatObject::id_t m_selectedItemId;

	CatFrameInfo_t( vk::CommandBuffer commandBuffer,
		CatCamera& rCamera,
		vk::DescriptorSet globalDescriptorSet,
		GlobalUbo& rUBO,
		CatObject::Map& mObjects,
		const float fFrameTime = 0.0f,
		const uint64_t nFrameIndex = 0,
		const CatObject::id_t& selectedItemId = 1 )
		: m_nFrameIndex( nFrameIndex ),
		  m_fFrameTime( fFrameTime ),
		  m_pCommandBuffer( commandBuffer ),
		  m_rCamera( rCamera ),
		  m_pGlobalDescriptorSet( globalDescriptorSet ),
		  m_rUBO( rUBO ),
		  m_mObjects( mObjects ),
		  m_selectedItemId( selectedItemId )
	{
	}

	void update( vk::CommandBuffer commandBuffer,
		vk::DescriptorSet descriptorSet,
		const float fFrameTime,
		const uint64_t nFrameIndex )
	{
		m_pCommandBuffer = commandBuffer;
		m_pGlobalDescriptorSet = descriptorSet;
		m_fFrameTime = fFrameTime;
		m_nFrameIndex = nFrameIndex;
	}

} CatFrameInfo;
} // namespace cat


#endif // CATENGINE_CATFRAMEINFO_HPP
