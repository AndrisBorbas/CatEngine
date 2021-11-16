#ifndef CATENGINE_CATMODEL_HPP
#define CATENGINE_CATMODEL_HPP


#include "Cat/Rendering/CatDevice.hpp"
#include "Cat/Rendering/CatBuffer.hpp"

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
		glm::vec3 m_vPosition{};
		glm::vec3 m_vColor{};
		glm::vec3 m_vNormal{};
		glm::vec2 m_vUV{};

		static std::vector< vk::VertexInputBindingDescription > getBindingDescriptions();
		static std::vector< vk::VertexInputAttributeDescription > getAttributeDescriptions();

		bool operator==( const Vertex& other ) const
		{
			return m_vPosition == other.m_vPosition && m_vColor == other.m_vColor && m_vNormal == other.m_vNormal
				   && m_vUV == other.m_vUV;
		}
	};

	struct Builder
	{
		std::vector< Vertex > m_aVertices{};
		std::vector< uint32_t > m_aIndices{};

		void loadModel( const std::string& filepath );
	};

	CatModel( CatDevice& device, const CatModel::Builder& builder );
	~CatModel();

	CatModel( const CatModel& ) = delete;
	CatModel& operator=( const CatModel& ) = delete;

	static std::unique_ptr< CatModel > createModelFromFile( CatDevice& device, const std::string& filepath );

	void bind( vk::CommandBuffer commandBuffer );
	void draw( vk::CommandBuffer commandBuffer );

private:
	void createVertexBuffers( const std::vector< Vertex >& vertices );
	void createIndexBuffers( const std::vector< uint32_t >& indices );

	CatDevice& m_rDevice;

	std::unique_ptr< CatBuffer > m_pVertexBuffer;
	uint32_t m_nVertexCount;

	bool m_bHasIndexBuffer = false;
	std::unique_ptr< CatBuffer > m_pIndexBuffer;
	uint32_t m_nIndexCount;
};
} // namespace cat

#endif // CATENGINE_CATMODEL_HPP
