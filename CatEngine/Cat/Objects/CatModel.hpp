#ifndef CATENGINE_CATMODEL_HPP
#define CATENGINE_CATMODEL_HPP

#include "Cat/VulkanRHI/CatDevice.hpp"
#include "Cat/VulkanRHI/CatBuffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include <memory>
#include <vector>

namespace cat
{
class CatModel
{
public:
	struct Vertex
	{
		glm::vec3 vPosition{};
		glm::vec3 vColor{};
		glm::vec3 vNormal{};
		glm::vec2 vUV{};

		static std::vector< vk::VertexInputBindingDescription > getBindingDescriptions();
		static std::vector< vk::VertexInputAttributeDescription > getAttributeDescriptions();

		bool operator==( const Vertex& other ) const
		{
			return vPosition == other.vPosition && vColor == other.vColor && vNormal == other.vNormal && vUV == other.vUV;
		}
	};

	struct Builder
	{
		std::vector< Vertex > aVertices{};
		std::vector< uint32_t > aIndices{};

		void loadModel( const std::string& filepath );
	};

	CatModel( CatDevice* pDevice, const CatModel::Builder& builder );
	~CatModel();

	CatModel( const CatModel& ) = delete;
	CatModel& operator=( const CatModel& ) = delete;

	// TODO: Don't load a model twice
	static std::shared_ptr< CatModel > createModelFromFile( CatDevice* pDevice, const std::string& filepath );

	void bind( vk::CommandBuffer commandBuffer );
	void draw( vk::CommandBuffer commandBuffer );

private:
	void createVertexBuffers( const std::vector< Vertex >& vertices );
	void createIndexBuffers( const std::vector< uint32_t >& indices );

	CatDevice* m_pDevice;

	std::unique_ptr< CatBuffer > m_pVertexBuffer;
	uint32_t m_nVertexCount;

	bool m_bHasIndexBuffer = false;
	std::unique_ptr< CatBuffer > m_pIndexBuffer;
	uint32_t m_nIndexCount;
};
} // namespace cat

#endif // CATENGINE_CATMODEL_HPP
