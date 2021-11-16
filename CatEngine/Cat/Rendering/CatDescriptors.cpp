#include "CatDescriptors.hpp"

#include <cassert>
#include <stdexcept>

namespace cat
{
// *************** Descriptor Set Layout Builder *********************

CatDescriptorSetLayout::Builder& CatDescriptorSetLayout::Builder::addBinding( uint32_t binding,
	vk::DescriptorType descriptorType,
	vk::ShaderStageFlags stageFlags,
	uint32_t count )
{
	assert( m_mBindings.count( binding ) == 0 && "Binding already in use" );
	vk::DescriptorSetLayoutBinding layoutBinding{
		.binding = binding,
		.descriptorType = descriptorType,
		.descriptorCount = count,
		.stageFlags = stageFlags,

	};
	m_mBindings[binding] = layoutBinding;
	return *this;
}

std::unique_ptr< CatDescriptorSetLayout > CatDescriptorSetLayout::Builder::build() const
{
	return std::make_unique< CatDescriptorSetLayout >( m_rDevice, m_mBindings );
}

// *************** Descriptor Set Layout *********************

CatDescriptorSetLayout::CatDescriptorSetLayout( CatDevice& rDevice,
	std::unordered_map< uint32_t, vk::DescriptorSetLayoutBinding > bindings )
	: m_rDevice{ rDevice }, m_mBindings{ bindings }
{
	std::vector< vk::DescriptorSetLayoutBinding > setLayoutBindings{};
	for ( auto kv : bindings )
	{
		setLayoutBindings.push_back( kv.second );
	}

	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{
		.bindingCount = static_cast< uint32_t >( setLayoutBindings.size() ), .pBindings = setLayoutBindings.data() };

	if ( m_rDevice.getDevice().createDescriptorSetLayout( &descriptorSetLayoutInfo, nullptr, &m_pDescriptorSetLayout )
		 != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create descriptor set layout!" );
	}
}

CatDescriptorSetLayout::~CatDescriptorSetLayout()
{
	m_rDevice.getDevice().destroy( m_pDescriptorSetLayout, nullptr );
}

// *************** Descriptor Pool Builder *********************

CatDescriptorPool::Builder& CatDescriptorPool::Builder::addPoolSize( vk::DescriptorType descriptorType, uint32_t count )
{
	m_aPoolSizes.push_back( { descriptorType, count } );
	return *this;
}

CatDescriptorPool::Builder& CatDescriptorPool::Builder::setPoolFlags( vk::DescriptorPoolCreateFlags flags )
{
	m_pPoolFlags = flags;
	return *this;
}
CatDescriptorPool::Builder& CatDescriptorPool::Builder::setMaxSets( uint32_t count )
{
	m_nMaxSets = count;
	return *this;
}

std::unique_ptr< CatDescriptorPool > CatDescriptorPool::Builder::build() const
{
	return std::make_unique< CatDescriptorPool >( m_rDevice, m_nMaxSets, m_pPoolFlags, m_aPoolSizes );
}

// *************** Descriptor Pool *********************

CatDescriptorPool::CatDescriptorPool( CatDevice& rDevice,
	uint32_t maxSets,
	vk::DescriptorPoolCreateFlags poolFlags,
	const std::vector< vk::DescriptorPoolSize >& poolSizes )
	: m_rDevice{ rDevice }
{
	vk::DescriptorPoolCreateInfo descriptorPoolInfo{
		.flags = poolFlags,
		.maxSets = maxSets,
		.poolSizeCount = static_cast< uint32_t >( poolSizes.size() ),
		.pPoolSizes = poolSizes.data(),
	};

	if ( m_rDevice.getDevice().createDescriptorPool( &descriptorPoolInfo, nullptr, &m_pDescriptorPool )
		 != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create descriptor pool!" );
	}
}

CatDescriptorPool::~CatDescriptorPool()
{
	m_rDevice.getDevice().destroy( m_pDescriptorPool, nullptr );
}

bool CatDescriptorPool::allocateDescriptor( const vk::DescriptorSetLayout descriptorSetLayout,
	vk::DescriptorSet& descriptor ) const
{
	vk::DescriptorSetAllocateInfo allocInfo{
		.descriptorPool = m_pDescriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptorSetLayout,
	};

	// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
	// a new pool whenever an old pool fills up. But this is beyond our current scope
	if ( m_rDevice.getDevice().allocateDescriptorSets( &allocInfo, &descriptor ) != vk::Result::eSuccess )
	{
		return false;
	}
	return true;
}

void CatDescriptorPool::freeDescriptors( std::vector< vk::DescriptorSet >& descriptors ) const
{
	m_rDevice.getDevice().free( m_pDescriptorPool, static_cast< uint32_t >( descriptors.size() ), descriptors.data() );
}

void CatDescriptorPool::resetPool()
{
	m_rDevice.getDevice().resetDescriptorPool( m_pDescriptorPool, {} );
}

// *************** Descriptor Writer *********************

CatDescriptorWriter::CatDescriptorWriter( CatDescriptorSetLayout& setLayout, CatDescriptorPool& pool )
	: m_rSetLayout{ setLayout }, m_rPool{ pool }
{
}

CatDescriptorWriter& CatDescriptorWriter::writeBuffer( uint32_t binding, vk::DescriptorBufferInfo* bufferInfo )
{
	assert( m_rSetLayout.m_mBindings.count( binding ) == 1 && "Layout does not contain specified binding" );

	auto& bindingDescription = m_rSetLayout.m_mBindings[binding];

	assert( bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple" );

	vk::WriteDescriptorSet write{
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = bindingDescription.descriptorType,
		.pBufferInfo = bufferInfo,
	};

	m_aWrites.push_back( write );
	return *this;
}

CatDescriptorWriter& CatDescriptorWriter::writeImage( uint32_t binding, vk::DescriptorImageInfo* imageInfo )
{
	assert( m_rSetLayout.m_mBindings.count( binding ) == 1 && "Layout does not contain specified binding" );

	auto& bindingDescription = m_rSetLayout.m_mBindings[binding];

	assert( bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple" );

	vk::WriteDescriptorSet write{
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = bindingDescription.descriptorType,
		.pImageInfo = imageInfo,
	};

	m_aWrites.push_back( write );
	return *this;
}

bool CatDescriptorWriter::build( vk::DescriptorSet& set )
{
	bool success = m_rPool.allocateDescriptor( m_rSetLayout.getDescriptorSetLayout(), set );
	if ( !success )
	{
		return false;
	}
	overwrite( set );
	return true;
}

void CatDescriptorWriter::overwrite( vk::DescriptorSet& set )
{
	for ( auto& write : m_aWrites )
	{
		write.dstSet = set;
	}
	m_rPool.m_rDevice.getDevice().updateDescriptorSets( m_aWrites.size(), m_aWrites.data(), 0, nullptr );
}
} // namespace cat