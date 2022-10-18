#ifndef CATENGINE_CATFRAMEINFO_HPP
#define CATENGINE_CATFRAMEINFO_HPP

#include "Cat/Controller/CatCamera.hpp"
#include "Cat/Objects/CatObject.hpp"

#include "vulkan/vulkan.hpp"


namespace cat
{
static constexpr auto MAX_LIGHTS = 16;

struct PointLight
{
	glm::vec4 position{}; // ignore w
	glm::vec4 color{};	  // w is intensity
};

struct GlobalUbo
{
	glm::mat4 projection{ 1.f };
	glm::mat4 view{ 1.f };
	glm::mat4 inverseView{ 1.f };
	glm::vec4 ambientLightColor{ .8f, .8f, 1.f, .086f }; // w is intensity
	PointLight pointLights[MAX_LIGHTS];
	int numLights;
};

using CatFrameInfo = struct CatFrameInfo_t
{
	short m_nFrameIndex = 0;
	uint64_t m_nFrameNumber = 0;
	double m_dFrameTime = 0.0;
	vk::CommandBuffer m_pCommandBuffer;
	CatCamera& m_rCamera;
	CatObject& m_rCameraObject;
	vk::DescriptorSet m_pGlobalDescriptorSet;
	GlobalUbo& m_rUBO;
	CatObject::Map& m_mObjects;
	id_t m_selectedItemId;

	CatFrameInfo_t( vk::CommandBuffer commandBuffer,
		CatCamera& rCamera,
		CatObject& rCameraObject,
		vk::DescriptorSet globalDescriptorSet,
		GlobalUbo& rUBO,
		CatObject::Map& mObjects,
		const double fFrameTime = 0.0,
		const short nFrameIndex = 0,
		const uint64_t nFrameNumber = 0,
		const id_t& selectedItemId = 0 )
		: m_nFrameIndex( nFrameIndex ),
		  m_nFrameNumber( nFrameNumber ),
		  m_dFrameTime( fFrameTime ),
		  m_pCommandBuffer( commandBuffer ),
		  m_rCamera( rCamera ),
		  m_rCameraObject( rCameraObject ),
		  m_pGlobalDescriptorSet( globalDescriptorSet ),
		  m_rUBO( rUBO ),
		  m_mObjects( mObjects ),
		  m_selectedItemId( selectedItemId )
	{
	}

	void update( vk::CommandBuffer commandBuffer,
		vk::DescriptorSet descriptorSet,
		const double fFrameTime,
		const short nFrameIndex,
		const uint64_t nFrameNumber )
	{
		m_pCommandBuffer = commandBuffer;
		m_pGlobalDescriptorSet = descriptorSet;
		m_dFrameTime = fFrameTime;
		m_nFrameIndex = nFrameIndex;
		m_nFrameNumber = nFrameNumber;
	}

	void updateSelectedItemId( const id_t& selectedItemId ) { m_selectedItemId = selectedItemId; }
};
} // namespace cat


#endif // CATENGINE_CATFRAMEINFO_HPP
