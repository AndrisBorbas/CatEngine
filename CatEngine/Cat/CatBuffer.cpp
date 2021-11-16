/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

#include "CatBuffer.hpp"

#include <cassert>
#include <cstring>

namespace cat
{

/**
 * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
 *
 * @param instanceSize The size of an instance
 * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
 * minUniformBufferOffsetAlignment)
 *
 * @return vk::Result of the buffer mapping call
 */
vk::DeviceSize CatBuffer::getAlignment( vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment )
{
	if ( minOffsetAlignment > 0 )
	{
		return ( instanceSize + minOffsetAlignment - 1 ) & ~( minOffsetAlignment - 1 );
	}
	return instanceSize;
}

CatBuffer::CatBuffer( CatDevice& device,
	vk::DeviceSize instanceSize,
	uint32_t instanceCount,
	vk::BufferUsageFlags usageFlags,
	vk::MemoryPropertyFlags memoryPropertyFlags,
	vk::DeviceSize minOffsetAlignment )
	: m_rDevice{ device },
	  m_pInstanceSize{ instanceSize },
	  m_nInstanceCount{ instanceCount },
	  m_pUsageFlags{ usageFlags },
	  m_pMemoryPropertyFlags{ memoryPropertyFlags }
{
	m_pAlignmentSize = getAlignment( instanceSize, minOffsetAlignment );
	m_pBufferSize = m_pAlignmentSize * instanceCount;
	device.createBuffer( m_pBufferSize, usageFlags, memoryPropertyFlags, m_pBuffer, m_pMemory );
}

CatBuffer::~CatBuffer()
{
	unmap();
	vkDestroyBuffer( m_rDevice.getDevice(), m_pBuffer, nullptr );
	vkFreeMemory( m_rDevice.getDevice(), m_pMemory, nullptr );
}

/**
 * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
 *
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
 * buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return vk::Result of the buffer mapping call
 */
vk::Result CatBuffer::map( vk::DeviceSize size, vk::DeviceSize offset )
{
	assert( m_pBuffer && m_pMemory && "Called map on buffer before create" );
	return m_rDevice.getDevice().mapMemory( m_pMemory, offset, size, {}, &m_pMapped );
}

/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void CatBuffer::unmap()
{
	if ( m_pMapped )
	{
		m_rDevice.getDevice().unmapMemory( m_pMemory );
		m_pMapped = nullptr;
	}
}

/**
 * Copies the specified data to the mapped buffer. Default value writes whole buffer range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
 * range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 *
 */
void CatBuffer::writeToBuffer( void* data, vk::DeviceSize size, vk::DeviceSize offset )
{
	assert( m_pMapped && "Cannot copy to unmapped buffer" );

	if ( size == VK_WHOLE_SIZE )
	{
		memcpy( m_pMapped, data, m_pBufferSize );
	}
	else
	{
		char* memOffset = (char*)m_pMapped;
		memOffset += offset;
		memcpy( memOffset, data, size );
	}
}

/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
 * complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return vk::Result of the flush call
 */
vk::Result CatBuffer::flush( vk::DeviceSize size, vk::DeviceSize offset )
{
	vk::MappedMemoryRange mappedRange = {
		.memory = m_pMemory,
		.offset = offset,
		.size = size,
	};
	return m_rDevice.getDevice().flushMappedMemoryRanges( 1, &mappedRange );
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
 * the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return vk::Result of the invalidate call
 */
vk::Result CatBuffer::invalidate( vk::DeviceSize size, vk::DeviceSize offset )
{
	vk::MappedMemoryRange mappedRange = {
		.memory = m_pMemory,
		.offset = offset,
		.size = size,
	};
	return m_rDevice.getDevice().invalidateMappedMemoryRanges( 1, &mappedRange );
}

/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return vk::DescriptorBufferInfo of specified offset and range
 */
vk::DescriptorBufferInfo CatBuffer::descriptorInfo( vk::DeviceSize size, vk::DeviceSize offset )
{
	return vk::DescriptorBufferInfo{
		.buffer = m_pBuffer,
		.offset = offset,
		.range = size,
	};
}

/**
 * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
 *
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 *
 */
void CatBuffer::writeToIndex( void* data, int index )
{
	writeToBuffer( data, m_pInstanceSize, index * m_pAlignmentSize );
}

/**
 *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
 *
 * @param index Used in offset calculation
 *
 */
vk::Result CatBuffer::flushIndex( int index )
{
	return flush( m_pAlignmentSize, index * m_pAlignmentSize );
}

/**
 * Create a buffer info descriptor
 *
 * @param index Specifies the region given by index * alignmentSize
 *
 * @return vk::DescriptorBufferInfo for instance at index
 */
vk::DescriptorBufferInfo CatBuffer::descriptorInfoForIndex( int index )
{
	return descriptorInfo( m_pAlignmentSize, index * m_pAlignmentSize );
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param index Specifies the region to invalidate: index * alignmentSize
 *
 * @return vk::Result of the invalidate call
 */
vk::Result CatBuffer::invalidateIndex( int index )
{
	return invalidate( m_pAlignmentSize, index * m_pAlignmentSize );
}

} // namespace cat
