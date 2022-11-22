#ifndef CATENGINE_CATTEXTURE_HPP
#define CATENGINE_CATTEXTURE_HPP

#include "Cat/VulkanRHI/CatDevice.hpp"
#include "Cat/VulkanRHI/CatBuffer.hpp"

#include <stb_image.h>

#include <dds.hpp>

namespace cat
{

class CatTexture
{
public:
	CatTexture( CatDevice* pDevice,
		const std::string& rFilename,
		vk::Format format,
		int stbiFormat,
		vk::Flags< vk::ImageUsageFlagBits > usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled );
	CatTexture( CatDevice* pDevice,
		const std::string& rFilename,
		vk::Flags< vk::ImageUsageFlagBits > usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled );
	virtual ~CatTexture();

protected:
	CatDevice* m_pDevice;
	vk::Image m_rImage;
	vk::ImageLayout m_rImageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	vk::ImageView m_rImageView;
	vk::DeviceMemory m_rImageMemory;
	CatBuffer* m_pStagingBuffer;
	uint32_t m_nWidth, m_nHeight;
	uint32_t m_nMipLevels;
	uint32_t m_nLayerCount;
	vk::DescriptorImageInfo m_rDescriptor;
	vk::Sampler m_rSampler;
	vk::ImageCreateInfo m_rImageCreateInfo;
	vk::ImageViewCreateInfo m_rImageViewCreateInfo;
	vk::Format m_rImageFormat;

	void updateDescriptor();

public:
	CAT_READONLY_PROPERTY( m_rSampler, getSampler, m_RSampler );
};

class CatTexture2D : public CatTexture
{
public:
	// STBI loader without mips
	CatTexture2D( CatDevice* pDevice,
		const std::string& rFilename,
		vk::Format format = vk::Format::eR8G8B8A8Srgb,
		int stbiFormat = STBI_rgb_alpha,
		vk::Flags< vk::ImageAspectFlagBits > aspectMask = vk::ImageAspectFlagBits::eColor,
		vk::Flags< vk::ImageUsageFlagBits > usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled );
	// TODO: FIX
	// DDS loader
	CatTexture2D( CatDevice* pDevice, const std::string& rFilename, vk::Flags< vk::ImageUsageFlagBits > usage );
	~CatTexture2D() override = default;
};

} // namespace cat

#endif // CATENGINE_CATTEXTURE_HPP
