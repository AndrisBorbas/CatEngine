#pragma once

#include "Defines.h"

namespace cat
{

struct FrameInfo
{
	uint_fast64_t m_nFrameIndex;
	float m_fFrameTime;
	vk::CommandBuffer& m_rCommandBuffer;
	CatCamera& m_rCamera;
	vk::DescriptorSet& m_rGlobalDescriptorset;
};

} // namespace cat
