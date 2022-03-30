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
	glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f }; // w is intensity
	glm::vec3 lightPosition{ -1.f };
	alignas( 16 ) glm::vec4 lightColor{ 1.f }; // w is light intensity
};

struct CatFrameInfo
{
	uint64_t m_nFrameIndex = 0;
	float m_fFrameTime = 0.0f;
	vk::CommandBuffer m_pCommandBuffer;
	CatCamera& m_rCamera;
	vk::DescriptorSet m_pGlobalDescriptorSet;
	GlobalUbo& m_rUBO;
	CatObject::Map& m_mObjects;
};
} // namespace cat


#endif // CATENGINE_CATFRAMEINFO_HPP
