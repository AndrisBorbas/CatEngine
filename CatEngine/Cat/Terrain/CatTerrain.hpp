#ifndef CATENGINE_CATTERRAIN_HPP
#define CATENGINE_CATTERRAIN_HPP

#include "Globals.hpp"
#include "Cat/VulkanRHI/CatBuffer.hpp"
#include "Cat/VulkanRHI/CatSwapChain.hpp"
#include "Cat/VulkanRHI/CatDescriptors.hpp"


#include "glm/glm.hpp"
#include "Cat/Texture/CatTexture.hpp"


namespace cat
{

struct TerrainUbo
{
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec4 ambientLightColor{ .8f, .8f, 1.f, .086f }; // w is intensity
	glm::vec4 lightPos{ 1.0f, 64.0f, 1.0f, 1.0f };
	glm::vec4 frustumPlanes[6];
	glm::vec2 viewportDimensions;
	float displacementFactor = 13.0f;
	float tessellationFactor = 0.725f;
	float tessellatedEdgeSize = 20.0f;
	float uvScale = 256.0f;
};

class CatTerrain
{
public:
	CatTerrain( CatDevice* pDevice, const std::string& sHeightmap, const std::string& sTexture );
	~CatTerrain() = default;

	void generateTerrain();

	void bind( vk::CommandBuffer commandBuffer );
	void draw( vk::CommandBuffer commandBuffer );

protected:
	float getHeight( uint32_t x, uint32_t y ) const;

	uint32_t m_nPatchSize = 64;
	float m_fUVScale = 1.0f;

	CatDevice* m_pDevice;

	std::vector< std::unique_ptr< CatBuffer > > m_aUboBuffers;
	std::unique_ptr< CatDescriptorPool > m_pDescriptorPool;
	std::unique_ptr< CatDescriptorSetLayout > m_pDescriptorSetLayout;
	std::vector< vk::DescriptorSet > m_aDescriptorSets;
	TerrainUbo m_ubo;

	std::unique_ptr< CatTexture2D > m_pHeightMap;
	std::unique_ptr< CatTexture2D > m_pTexture;
	unsigned char* m_pHeightData;
	uint32_t m_nScale;
	uint32_t m_nWidth;
	uint32_t m_nIndexCount;
	std::unique_ptr< CatBuffer > m_pVertexBuffer;
	std::unique_ptr< CatBuffer > m_pIndexBuffer;

public:
	CAT_READONLY_PROPERTY( m_ubo, getUbo, m_Ubo );
	CAT_READONLY_PROPERTY( m_aDescriptorSets, getDescriptorSets, m_ADescriptorSets );
	CAT_READONLY_PROPERTY( m_pDescriptorSetLayout, getDescriptorSetLayout, m_PDescriptorSetLayout );
	CAT_READONLY_PROPERTY( m_aUboBuffers, getUboBuffers, m_AUboBuffers );
};

} // namespace cat

#endif // CATENGINE_CATTERRAIN_HPP
