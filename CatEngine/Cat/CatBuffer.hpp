#ifndef CATENGINE_CATBUFFER_HPP
#define CATENGINE_CATBUFFER_HPP

#include "CatDevice.hpp"

namespace cat
{
class CatBuffer
{
public:
	CatBuffer( CatDevice& device,
		vk::DeviceSize instanceSize,
		uint32_t instanceCount,
		vk::BufferUsageFlags usageFlags,
		vk::MemoryPropertyFlags memoryPropertyFlags,
		vk::DeviceSize minOffsetAlignment = 1 );
	~CatBuffer();

	CatBuffer( const CatBuffer& ) = delete;
	CatBuffer& operator=( const CatBuffer& ) = delete;

	vk::Result map( vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0 );
	void unmap();

	void writeToBuffer( void* data, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0 );
	vk::Result flush( vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0 );
	vk::DescriptorBufferInfo descriptorInfo( vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0 );
	vk::Result invalidate( vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0 );

	void writeToIndex( void* data, int index );
	vk::Result flushIndex( int index );
	vk::DescriptorBufferInfo descriptorInfoForIndex( int index );
	vk::Result invalidateIndex( int index );

	vk::Buffer getBuffer() const { return m_pBuffer; }
	void* getMappedMemory() const { return m_pMapped; }
	uint32_t getInstanceCount() const { return m_nInstanceCount; }
	vk::DeviceSize getInstanceSize() const { return m_pInstanceSize; }
	vk::DeviceSize getAlignmentSize() const { return m_pAlignmentSize; }
	vk::BufferUsageFlags getUsageFlags() const { return m_pUsageFlags; }
	vk::MemoryPropertyFlags getMemoryPropertyFlags() const { return m_pMemoryPropertyFlags; }
	vk::DeviceSize getBufferSize() const { return m_pBufferSize; }

private:
	static vk::DeviceSize getAlignment( vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment );

	CatDevice& m_rDevice;
	void* m_pMapped = nullptr;
	vk::Buffer m_pBuffer = nullptr;
	vk::DeviceMemory m_pMemory = nullptr;

	vk::DeviceSize m_pBufferSize;
	uint32_t m_nInstanceCount;
	vk::DeviceSize m_pInstanceSize;
	vk::DeviceSize m_pAlignmentSize;
	vk::BufferUsageFlags m_pUsageFlags;
	vk::MemoryPropertyFlags m_pMemoryPropertyFlags;
};
} // namespace cat


#endif // CATENGINE_CATBUFFER_HPP
