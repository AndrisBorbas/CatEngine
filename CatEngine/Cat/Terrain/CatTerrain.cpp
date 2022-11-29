#include "CatTerrain.hpp"
#include "Cat/Objects/CatModel.hpp"

namespace cat
{
CatTerrain::CatTerrain( CatDevice* pDevice, const std::string& sHeightmap, const std::string& sTexture ) : m_pDevice( pDevice )
{
	m_pHeightMap = std::make_unique< CatTexture2D >( m_pDevice, sHeightmap, vk::Format::eR8Srgb, STBI_grey );
	m_nWidth = m_pHeightMap->m_NWidth;
	m_pHeightData = new unsigned char[m_nWidth * m_nWidth];
	memcpy( m_pHeightData, m_pHeightMap->m_PPixels, m_nWidth * m_nWidth );
	m_nScale = m_nWidth / m_nPatchSize;

	m_pTexture = std::make_unique< CatTexture2D >( m_pDevice, sTexture, vk::Format::eR8G8B8A8Srgb, STBI_rgb_alpha );


	m_pDescriptorPool = CatDescriptorPool::Builder( *m_pDevice )
							.setMaxSets( CatSwapChain::MAX_FRAMES_IN_FLIGHT * 2 * 3 )
							.addPoolSize( vk::DescriptorType::eUniformBuffer, CatSwapChain::MAX_FRAMES_IN_FLIGHT * 3 )
							.addPoolSize( vk::DescriptorType::eCombinedImageSampler, CatSwapChain::MAX_FRAMES_IN_FLIGHT * 3 )
							.build();

	m_aUboBuffers = std::vector< std::unique_ptr< CatBuffer > >( CatSwapChain::MAX_FRAMES_IN_FLIGHT );
	for ( auto& uboBuffer : m_aUboBuffers )
	{
		uboBuffer = std::make_unique< CatBuffer >( m_pDevice, sizeof( TerrainUbo ), 1, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
		uboBuffer->map();
	}

	m_pDescriptorSetLayout =
		CatDescriptorSetLayout::Builder( *m_pDevice )
			// Tessellation shader ubo
			.addBinding( 0, vk::DescriptorType::eUniformBuffer,
				vk::ShaderStageFlagBits::eTessellationControl | vk::ShaderStageFlagBits::eTessellationEvaluation )
			// Heightmap texture
			.addBinding( 1, vk::DescriptorType::eCombinedImageSampler,
				vk::ShaderStageFlagBits::eTessellationControl | vk::ShaderStageFlagBits::eTessellationEvaluation
					| vk::ShaderStageFlagBits::eFragment )
			// Terrain texture
			.addBinding( 2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment )
			.build();

	m_aDescriptorSets = std::vector< vk::DescriptorSet >( CatSwapChain::MAX_FRAMES_IN_FLIGHT );
	for ( size_t i = 0; i < m_aDescriptorSets.size(); i++ )
	{
		auto bufferInfo = m_aUboBuffers[i]->descriptorInfo();
		// this might be the global pool
		CatDescriptorWriter( *m_pDescriptorSetLayout, *m_pDescriptorPool )
			.writeBuffer( 0, &bufferInfo )
			.writeImage( 1, &m_pHeightMap->m_RDescriptor )
			.writeImage( 2, &m_pTexture->m_RDescriptor )
			.build( m_aDescriptorSets[i] );
	}

	generateTerrain();
}

void CatTerrain::generateTerrain()
{
	const uint32_t nVertexCount = m_nPatchSize * m_nPatchSize;
	CatModel::Vertex* pVertices = new CatModel::Vertex[nVertexCount];
	const float fWX = 2.0f;
	const float fWY = 2.0f;

	for ( int x = 0; x < m_nPatchSize; x++ )
	{
		for ( int y = 0; y < m_nPatchSize; y++ )
		{
			uint32_t index = x + y * m_nPatchSize;
			pVertices[index].vPosition[0] = x * fWX + fWX / 2.0f - (float)m_nPatchSize * fWX / 2.0f;
			pVertices[index].vPosition[1] = 0.0f;
			pVertices[index].vPosition[2] = y * fWY + fWY / 2.0f - (float)m_nPatchSize * fWY / 2.0f;
			pVertices[index].vUV = glm::vec2( (float)x / (float)m_nPatchSize, (float)y / (float)m_nPatchSize ) * m_fUVScale;
		}
	}

	for ( int x = 0; x < m_nPatchSize; x++ )
	{
		for ( int y = 0; y < m_nPatchSize; y++ )
		{
			float vHeights[3][3];
			for ( int hx = -1; hx <= 1; hx++ )
			{
				for ( int hy = -1; hy <= 1; hy++ )
				{
					vHeights[hx + 1][hy + 1] = getHeight( x + hx, y + hy );
				}
			}

			// Sobel filter for normals
			glm::vec3 vNormal;
			vNormal.x = vHeights[0][0] - vHeights[2][0] + 2.0f * vHeights[0][1] - 2.0f * vHeights[2][1] + vHeights[0][2]
						- vHeights[2][2];
			vNormal.z = vHeights[0][0] + 2.0f * vHeights[1][0] + vHeights[2][0] - vHeights[0][2] - 2.0f * vHeights[1][2]
						- vHeights[2][2];
			vNormal.y = sqrtf( 1.0f - vNormal.x * vNormal.x - vNormal.z * vNormal.z ) / 4.0f;

			pVertices[x + y * m_nPatchSize].vNormal = glm::normalize( vNormal * glm::vec3( 2.0f, 1.0f, 2.0f ) );
		}
	}

	CatBuffer vertexStagingBuffer{
		m_pDevice,
		sizeof( CatModel::Vertex ),
		nVertexCount,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
	};
	vertexStagingBuffer.map();
	vertexStagingBuffer.writeToBuffer( pVertices );

	uint32_t nVertexBufferSize = nVertexCount * sizeof( CatModel::Vertex );
	m_pVertexBuffer = std::make_unique< CatBuffer >( m_pDevice, nVertexBufferSize, nVertexCount,
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal );

	m_pDevice->copyBuffer( *vertexStagingBuffer, **m_pVertexBuffer, nVertexBufferSize );

	delete[] pVertices;


	const uint32_t w = m_nPatchSize - 1;
	m_nIndexCount = w * w * 4;
	uint32_t* pIndices = new uint32_t[m_nIndexCount];

	for ( int x = 0; x < w; x++ )
	{
		for ( int y = 0; y < w; y++ )
		{
			uint32_t index = ( x + y * w ) * 4;
			pIndices[index] = x + y * m_nPatchSize;
			pIndices[index + 1] = pIndices[index] + m_nPatchSize;
			pIndices[index + 2] = pIndices[index + 1] + 1;
			pIndices[index + 3] = pIndices[index] + 1;
		}
	}

	CatBuffer indexStagingBuffer{
		m_pDevice,
		sizeof( uint32_t ),
		m_nIndexCount,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
	};
	indexStagingBuffer.map();
	indexStagingBuffer.writeToBuffer( pIndices );

	uint32_t nIndexBufferSize = m_nIndexCount * sizeof( uint32_t );
	m_pIndexBuffer = std::make_unique< CatBuffer >( m_pDevice, nIndexBufferSize, m_nIndexCount,
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal );

	m_pDevice->copyBuffer( *indexStagingBuffer, **m_pIndexBuffer, nIndexBufferSize );

	delete[] pIndices;
}

float CatTerrain::getHeight( uint32_t x, uint32_t y ) const
{
	glm::ivec2 vPoint = glm::ivec2( x, y ) * glm::ivec2( m_nScale );
	vPoint.x = std::max( 0, std::min( vPoint.x, (int)m_nWidth - 1 ) );
	vPoint.y = std::max( 0, std::min( vPoint.y, (int)m_nWidth - 1 ) );
	vPoint /= glm::ivec2( m_nScale );
	return 1.0 - m_pHeightData[( vPoint.x + vPoint.y * m_nWidth ) * m_nScale] / 65535.0f;
}

void CatTerrain::bind( vk::CommandBuffer commandBuffer )
{
	vk::Buffer buffers[] = { **m_pVertexBuffer };
	vk::DeviceSize offsets[] = { 0 };
	commandBuffer.bindVertexBuffers( 0, 1, buffers, offsets );
	commandBuffer.bindIndexBuffer( **m_pIndexBuffer, 0, vk::IndexType::eUint32 );
}


void CatTerrain::draw( vk::CommandBuffer commandBuffer )
{
	commandBuffer.drawIndexed( m_nIndexCount, 1, 0, 0, 0 );
}


} // namespace cat
