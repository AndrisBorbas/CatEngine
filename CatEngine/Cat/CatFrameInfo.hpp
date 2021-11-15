#ifndef CATENGINE_CATFRAMEINFO_HPP
#define CATENGINE_CATFRAMEINFO_HPP

#include "CatCamera.hpp"

#include "vulkan/vulkan.hpp"


namespace cat
{
struct CatFrameInfo
{
	uint64_t m_nFrameIndex;
	float m_fFrameTime;
	vk::CommandBuffer m_pCommandBuffer;
	CatCamera& m_rCamera;
	vk::DescriptorSet m_pGlobalDescriptorSet;
};
} // namespace cat


#endif // CATENGINE_CATFRAMEINFO_HPP
