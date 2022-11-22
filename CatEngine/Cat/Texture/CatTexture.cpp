#include "CatTexture.hpp"

#include <loguru.hpp>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif // STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace cat
{

CatTexture::CatTexture( CatDevice* pDevice,
	const std::string& rFilename,
	vk::Format format,
	int stbiFormat,
	vk::Flags< vk::ImageUsageFlagBits > usage /* = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled */ )
	: m_pDevice( pDevice )
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load( rFilename.c_str(), &texWidth, &texHeight, &texChannels, stbiFormat );
	vk::DeviceSize imageSize = texWidth * texHeight * texChannels;

	if ( !pixels )
	{
		LOG_F( ERROR, "Failed to load texture image!" );
	}

	m_nWidth = static_cast< uint32_t >( texWidth );
	m_nHeight = static_cast< uint32_t >( texHeight );

	m_pStagingBuffer = new CatBuffer( m_pDevice, imageSize, 1, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );

	m_pStagingBuffer->map();
	m_pStagingBuffer->writeToBuffer( pixels );

	m_pStagingBuffer->unmap();
	stbi_image_free( pixels );
}

CatTexture::CatTexture( cat::CatDevice* pDevice,
	const std::string& rFilename,
	vk::Flags< vk::ImageUsageFlagBits > usage /* = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled */ )
	: m_pDevice( pDevice )
{
	dds::Image image;

	dds::readFile( rFilename, &image );

	m_nWidth = image.width;
	m_nHeight = image.height;
	m_nMipLevels = image.numMips;
	m_nLayerCount = image.depth;
	m_rImageFormat = vk::Format{ dds::getVulkanFormat( image.format, image.supportsAlpha ) };
	m_rImageCreateInfo = dds::getVulkanImageCreateInfo( &image );
	m_rImageCreateInfo.usage = usage;
	m_rImageCreateInfo.samples = vk::SampleCountFlagBits::e1;
	m_rImageViewCreateInfo = dds::getVulkanImageViewCreateInfo( &image );
	m_rImageViewCreateInfo.image = m_rImage;

	m_pStagingBuffer = new CatBuffer( m_pDevice, image.arraySize, 1, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );

	m_pStagingBuffer->map();
	m_pStagingBuffer->writeToBuffer( image.data.data() );

	m_pStagingBuffer->unmap();
}

CatTexture::~CatTexture()
{
	( **m_pDevice ).destroyImageView( m_rImageView );
	( **m_pDevice ).destroyImage( m_rImage );
	( **m_pDevice ).destroySampler( m_rSampler );
	( **m_pDevice ).freeMemory( m_rImageMemory );
	delete m_pStagingBuffer;
}

void CatTexture::updateDescriptor()
{
	m_rDescriptor.imageLayout = m_rImageLayout;
	m_rDescriptor.imageView = m_rImageView;
	m_rDescriptor.sampler = m_rSampler;
}

CatTexture2D::CatTexture2D( CatDevice* pDevice,
	const std::string& rFilename,
	vk::Format format /* = vk::Format::eR8G8B8A8Srgb */,
	int stbiFormat /* = STBI_rgb_alpha */,
	vk::Flags< vk::ImageAspectFlagBits > aspectMask /* = vk::ImageAspectFlagBits::eColor */,
	vk::Flags< vk::ImageUsageFlagBits > usage /* = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled */ )
	: CatTexture( pDevice, rFilename, format, stbiFormat, usage )
{
	vk::ImageCreateInfo imageInfo{
		.imageType = vk::ImageType::e2D,
		.format = format,
		.extent = { .width = m_nWidth, .height = m_nHeight, .depth = 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = vk::SampleCountFlagBits::e1,
		.tiling = vk::ImageTiling::eOptimal,
		.usage = usage,
		.sharingMode = vk::SharingMode::eExclusive,
		.initialLayout = vk::ImageLayout::eUndefined,
	};

	m_pDevice->createImageWithInfo( imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, m_rImage, m_rImageMemory );

	m_pDevice->transitionImageLayout(
		m_rImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, { aspectMask, 0, 1, 0, 1 } );

	m_pDevice->copyBufferToImage(
		m_pStagingBuffer->getBuffer(), m_rImage, { m_nWidth, m_nHeight, 1 }, { aspectMask, 0, 0, 1 } );

	m_pDevice->transitionImageLayout(
		m_rImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, { aspectMask, 0, 1, 0, 1 } );

	delete m_pStagingBuffer;
	m_pStagingBuffer = nullptr;

	vk::SamplerCreateInfo samplerCreateInfo{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.mipLodBias = 0.0f,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = 16,
		.compareEnable = VK_FALSE,
		.compareOp = vk::CompareOp::eAlways,
		.minLod = 0.0f,
		.maxLod = 0.0f,
		.borderColor = vk::BorderColor::eFloatTransparentBlack,
		.unnormalizedCoordinates = VK_FALSE,
	};

	if ( ( **m_pDevice ).createSampler( &samplerCreateInfo, nullptr, &m_rSampler ) != vk::Result::eSuccess )
	{
		LOG_F( ERROR, "Failed to create texture sampler!" );
	}

	vk::ImageViewCreateInfo viewInfo{
		.image = m_rImage,
		.viewType = vk::ImageViewType::e2D,
		.format = format,
		.subresourceRange =
			{ .aspectMask = aspectMask, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 },
	};

	if ( ( **m_pDevice ).createImageView( &viewInfo, nullptr, &m_rImageView ) != vk::Result::eSuccess )
	{
		LOG_F( ERROR, "Failed to create texture image view!" );
	}

	updateDescriptor();
}

CatTexture2D::CatTexture2D( CatDevice* pDevice,
	const std::string& rFilename,
	vk::Flags< vk::ImageUsageFlagBits > usage /* = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled */ )
	: CatTexture( pDevice, rFilename, usage )
{
	m_pDevice->createImageWithInfo( m_rImageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, m_rImage, m_rImageMemory );

	m_pDevice->transitionImageLayout(
		m_rImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, m_rImageViewCreateInfo.subresourceRange );

	m_pDevice->copyBufferToImage( m_pStagingBuffer->getBuffer(), m_rImage, m_rImageCreateInfo.extent,
		{
			m_rImageViewCreateInfo.subresourceRange.aspectMask,
			0,
			m_rImageViewCreateInfo.subresourceRange.baseArrayLayer,
			m_rImageViewCreateInfo.subresourceRange.layerCount,
		} );

	m_pDevice->transitionImageLayout( m_rImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
		m_rImageViewCreateInfo.subresourceRange );

	delete m_pStagingBuffer;
	m_pStagingBuffer = nullptr;

	vk::SamplerCreateInfo samplerCreateInfo{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.mipLodBias = 0.0f,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = 16,
		.compareEnable = VK_FALSE,
		.compareOp = vk::CompareOp::eAlways,
		.minLod = 0.0f,
		.maxLod = 0.0f,
		.borderColor = vk::BorderColor::eFloatTransparentBlack,
		.unnormalizedCoordinates = VK_FALSE,
	};

	if ( ( **m_pDevice ).createSampler( &samplerCreateInfo, nullptr, &m_rSampler ) != vk::Result::eSuccess )
	{
		LOG_F( ERROR, "Failed to create texture sampler!" );
	}

	if ( ( **m_pDevice ).createImageView( &m_rImageViewCreateInfo, nullptr, &m_rImageView ) != vk::Result::eSuccess )
	{
		LOG_F( ERROR, "Failed to create texture image view!" );
	}

	updateDescriptor();
}
} // namespace cat
