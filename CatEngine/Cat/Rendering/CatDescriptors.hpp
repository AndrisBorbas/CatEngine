#ifndef CATENGINE_CATDESCRIPTORS_HPP
#define CATENGINE_CATDESCRIPTORS_HPP

#include "CatDevice.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace cat
{
class CatDescriptorSetLayout
{
public:
	class Builder
	{
	public:
		Builder( CatDevice& rDevice ) : m_rDevice{ rDevice } {}

		Builder& addBinding( uint32_t binding,
			vk::DescriptorType descriptorType,
			vk::ShaderStageFlags stageFlags,
			uint32_t count = 1 );
		std::unique_ptr< CatDescriptorSetLayout > build() const;

	private:
		CatDevice& m_rDevice;
		std::unordered_map< uint32_t, vk::DescriptorSetLayoutBinding > m_mBindings{};
	};

	CatDescriptorSetLayout( CatDevice& rDevice, std::unordered_map< uint32_t, vk::DescriptorSetLayoutBinding > mBindings );
	~CatDescriptorSetLayout();
	CatDescriptorSetLayout( const CatDescriptorSetLayout& ) = delete;
	CatDescriptorSetLayout& operator=( const CatDescriptorSetLayout& ) = delete;

	vk::DescriptorSetLayout getDescriptorSetLayout() const { return m_pDescriptorSetLayout; }

private:
	CatDevice& m_rDevice;
	vk::DescriptorSetLayout m_pDescriptorSetLayout;
	std::unordered_map< uint32_t, vk::DescriptorSetLayoutBinding > m_mBindings;

	friend class CatDescriptorWriter;
};

class CatDescriptorPool
{
public:
	class Builder
	{
	public:
		Builder( CatDevice& rDevice ) : m_rDevice{ rDevice } {}

		Builder& addPoolSize( vk::DescriptorType descriptorType, uint32_t count );
		Builder& setPoolFlags( vk::DescriptorPoolCreateFlags flags );
		Builder& setMaxSets( uint32_t count );
		std::unique_ptr< CatDescriptorPool > build() const;

	private:
		CatDevice& m_rDevice;
		std::vector< vk::DescriptorPoolSize > m_aPoolSizes{};
		uint32_t m_nMaxSets = 1000;
		vk::DescriptorPoolCreateFlags m_pPoolFlags{};
	};

	CatDescriptorPool( CatDevice& lveDevice,
		uint32_t maxSets,
		vk::DescriptorPoolCreateFlags poolFlags,
		const std::vector< vk::DescriptorPoolSize >& poolSizes );
	~CatDescriptorPool();
	CatDescriptorPool( const CatDescriptorPool& ) = delete;
	CatDescriptorPool& operator=( const CatDescriptorPool& ) = delete;

	bool allocateDescriptor( const vk::DescriptorSetLayout descriptorSetLayout, vk::DescriptorSet& descriptor ) const;

	void freeDescriptors( std::vector< vk::DescriptorSet >& descriptors ) const;

	void resetPool();

private:
	CatDevice& m_rDevice;
	vk::DescriptorPool m_pDescriptorPool;

	friend class CatDescriptorWriter;
};

class CatDescriptorWriter
{
public:
	CatDescriptorWriter( CatDescriptorSetLayout& setLayout, CatDescriptorPool& pool );

	CatDescriptorWriter& writeBuffer( uint32_t binding, vk::DescriptorBufferInfo* bufferInfo );
	CatDescriptorWriter& writeImage( uint32_t binding, vk::DescriptorImageInfo* imageInfo );

	bool build( vk::DescriptorSet& set );
	void overwrite( vk::DescriptorSet& set );

private:
	CatDescriptorSetLayout& m_rSetLayout;
	CatDescriptorPool& m_rPool;
	std::vector< vk::WriteDescriptorSet > m_aWrites;
};
} // namespace cat


#endif // CATENGINE_CATDESCRIPTORS_HPP
