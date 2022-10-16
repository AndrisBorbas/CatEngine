#include "CatModel.hpp"

#include "Cat/Utils/CatUtils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include <cassert>
#include <cstring>
#include <unordered_map>

namespace std
{
template <>
struct hash< cat::CatModel::Vertex >
{
	size_t operator()( cat::CatModel::Vertex const& vertex ) const
	{
		size_t seed = 0;
		cat::HashCombine( seed, vertex.m_vPosition, vertex.m_vColor, vertex.m_vNormal, vertex.m_vUV );
		return seed;
	}
};
} // namespace std


namespace cat
{
CatModel::CatModel( CatDevice& device, const CatModel::Builder& builder ) : m_rDevice{ device }
{
	createVertexBuffers( builder.m_aVertices );
	createIndexBuffers( builder.m_aIndices );
}

CatModel::~CatModel()
{
}

std::unique_ptr< CatModel > CatModel::createModelFromFile( CatDevice& device, const std::string& filepath )
{
	Builder builder{};
	builder.loadModel( filepath );
	return std::make_unique< CatModel >( device, builder );
}

void CatModel::createVertexBuffers( const std::vector< Vertex >& vertices )
{
	m_nVertexCount = static_cast< uint32_t >( vertices.size() );
	assert( m_nVertexCount >= 3 && "Vertex count must be at least 3" );
	vk::DeviceSize bufferSize = sizeof( vertices[0] ) * m_nVertexCount;
	uint32_t vertexSize = sizeof( vertices[0] );

	CatBuffer stagingBuffer{
		m_rDevice,
		vertexSize,
		m_nVertexCount,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
	};

	stagingBuffer.map();
	stagingBuffer.writeToBuffer( (void*)vertices.data() );

	m_pVertexBuffer = std::make_unique< CatBuffer >( m_rDevice, vertexSize, m_nVertexCount,
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal );

	m_rDevice.copyBuffer( stagingBuffer.getBuffer(), m_pVertexBuffer->getBuffer(), bufferSize );
}

void CatModel::createIndexBuffers( const std::vector< uint32_t >& indices )
{
	m_nIndexCount = static_cast< uint32_t >( indices.size() );
	m_bHasIndexBuffer = m_nIndexCount > 0;

	if ( !m_bHasIndexBuffer )
	{
		return;
	}

	vk::DeviceSize bufferSize = sizeof( indices[0] ) * m_nIndexCount;
	uint32_t indexSize = sizeof( indices[0] );

	CatBuffer stagingBuffer{
		m_rDevice,
		indexSize,
		m_nIndexCount,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
	};

	stagingBuffer.map();
	stagingBuffer.writeToBuffer( (void*)indices.data() );

	m_pIndexBuffer = std::make_unique< CatBuffer >( m_rDevice, indexSize, m_nIndexCount,
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal );

	m_rDevice.copyBuffer( stagingBuffer.getBuffer(), m_pIndexBuffer->getBuffer(), bufferSize );
}

void CatModel::draw( vk::CommandBuffer commandBuffer )
{
	if ( m_bHasIndexBuffer )
	{
		commandBuffer.drawIndexed( m_nIndexCount, 1, 0, 0, 0 );
	}
	else
	{
		commandBuffer.draw( m_nVertexCount, 1, 0, 0 );
	}
}

void CatModel::bind( vk::CommandBuffer commandBuffer )
{
	vk::Buffer buffers[] = { m_pVertexBuffer->getBuffer() };
	vk::DeviceSize offsets[] = { 0 };
	commandBuffer.bindVertexBuffers( 0, 1, buffers, offsets );

	if ( m_bHasIndexBuffer )
	{
		vkCmdBindIndexBuffer( commandBuffer, m_pIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32 );
	}
}

std::vector< vk::VertexInputBindingDescription > CatModel::Vertex::getBindingDescriptions()
{
	std::vector< vk::VertexInputBindingDescription > bindingDescriptions( 1 );
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof( Vertex );
	bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;
	return bindingDescriptions;
}

std::vector< vk::VertexInputAttributeDescription > CatModel::Vertex::getAttributeDescriptions()
{
	std::vector< vk::VertexInputAttributeDescription > attributeDescriptions{};

	attributeDescriptions.push_back(
		{ 0, 0, vk::Format::eR32G32B32Sfloat, static_cast< uint32_t >( offsetof( Vertex, m_vPosition ) ) } );
	attributeDescriptions.push_back(
		{ 1, 0, vk::Format::eR32G32B32Sfloat, static_cast< uint32_t >( offsetof( Vertex, m_vColor ) ) } );
	attributeDescriptions.push_back(
		{ 2, 0, vk::Format::eR32G32B32Sfloat, static_cast< uint32_t >( offsetof( Vertex, m_vNormal ) ) } );
	attributeDescriptions.push_back(
		{ 3, 0, vk::Format::eR32G32Sfloat, static_cast< uint32_t >( offsetof( Vertex, m_vUV ) ) } );

	return attributeDescriptions;
}

void CatModel::Builder::loadModel( const std::string& filepath )
{
	tinyobj::attrib_t attrib;
	std::vector< tinyobj::shape_t > shapes;
	std::vector< tinyobj::material_t > materials;
	std::string warn, err;

	if ( !tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, filepath.c_str() ) )
	{
		throw std::runtime_error( warn + err );
	}

	m_aVertices.clear();
	m_aIndices.clear();

	std::unordered_map< Vertex, uint32_t > uniqueVertices{};
	for ( const auto& shape : shapes )
	{
		for ( const auto& index : shape.mesh.indices )
		{
			Vertex vertex{};

			if ( index.vertex_index >= 0 )
			{
				vertex.m_vPosition = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2],
				};

				vertex.m_vColor = {
					attrib.colors[3 * index.vertex_index + 0],
					attrib.colors[3 * index.vertex_index + 1],
					attrib.colors[3 * index.vertex_index + 2],
				};
			}

			if ( index.normal_index >= 0 )
			{
				vertex.m_vNormal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2],
				};
			}

			if ( index.texcoord_index >= 0 )
			{
				vertex.m_vUV = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1],
				};
			}

			if ( !uniqueVertices.contains( vertex ) )
			{
				uniqueVertices[vertex] = static_cast< uint32_t >( m_aVertices.size() );
				m_aVertices.push_back( vertex );
			}
			m_aIndices.push_back( uniqueVertices[vertex] );
		}
	}
}
} // namespace cat
