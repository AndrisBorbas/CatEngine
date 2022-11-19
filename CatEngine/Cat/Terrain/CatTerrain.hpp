#ifndef CATENGINE_CATTERRAIN_HPP
#define CATENGINE_CATTERRAIN_HPP

#include "Cat/VulkanRHI/CatBuffer.hpp"

namespace cat
{

class CatTerrain
{
public:
	CatTerrain();
	~CatTerrain();

protected:
	CatBuffer m_pVertices;
	CatBuffer m_pIndices;
};

} // namespace cat

#endif // CATENGINE_CATTERRAIN_HPP
