#ifndef CATENGINE_CATCHUNK_HPP
#define CATENGINE_CATCHUNK_HPP

#include "Cat/Utils/CatUtils.hpp"
#include "Cat/Objects/CatObject.hpp"

#include <string>
#include <utility>
#include <future>

namespace cat
{

class CatChunk
{
private:
	const id_t m_id;
	glm::ivec2 m_vPosition;
	CatObject::Map m_mObjects;
	std::vector< id_t > m_aObjectIds;
	bool m_bLoaded = false;

public:
	CatChunk( const id_t id, glm::ivec2 vPosition, glm::ivec2 vSize, glm::ivec2 vMaxSize );
	~CatChunk() = default;

	bool load();
	bool unload();

	CAT_READONLY_PROPERTY( m_id, getId, m_ID );
	CAT_PROPERTY( m_vPosition, getPosition, setPosition, m_VPosition );
	CAT_READONLY_PROPERTY( m_mObjects, getObjects, m_MObjects );
	CAT_READONLY_PROPERTY( m_aObjectIds, getObjectIds, m_AObjectIds );
};
} // namespace cat

#endif // CATENGINE_CATCHUNK_HPP
