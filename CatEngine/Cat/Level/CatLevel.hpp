#ifndef CATENGINE_CATLEVEL_HPP
#define CATENGINE_CATLEVEL_HPP

#include "Cat/Utils/CatUtils.hpp"
#include "Cat/Objects/CatObject.hpp"
#include "Cat/Level/CatChunk.hpp"
#include "Cat/Terrain/CatTerrain.hpp"

#include <string>
#include <utility>
#include <future>
#include <queue>

namespace cat
{

// TODO: Levels are rectangular, and chunks have ascending ids, so you can easily calculate the neighbouring chunk ids on the x
// axis by adding/subtracting 1 and ont the z axis by adding/subtracting the width of the level in chunks.
// By storing the currently loaded chunk ids you can unload chunks when moving by checking if the old loadedchunks contained the
// id or not

class CatLevel
{
private:
	std::string m_sName;
	glm::ivec2 m_vSize;
	glm::ivec2 m_vChunkSize;
	id_t m_idCurrentChunk = 0;
	std::unordered_map< id_t, std::unique_ptr< CatChunk > > m_mChunks;
	std::vector< bool > m_aLoadedChunks;
	std::vector< bool > m_aLastLoadedChunks;
	CatObject::Map m_mObjects;
	std::unique_ptr< CatTerrain > m_pTerrain;
	std::future< void > m_fLoaded;
	json m_jData;

public:
	virtual ~CatLevel() = default;

	[[nodiscard]] static std::unique_ptr< CatLevel > create( const std::string& sName,
		glm::ivec2 vSize = glm::ivec2( 7, 7 ),
		glm::ivec2 vChunkSize = glm::ivec2( 10, 10 ) );
	void save( const std::string& sFileName = "" );
	[[nodiscard]] static std::unique_ptr< CatLevel > load( const std::string& levelData );

	bool isFullyLoaded();
	bool isLoadingFinished();

	void updateObjectLocation( id_t id );
	id_t getChunkAtLocation( const glm::vec3& vLocation );

	CatObject::Map getAllObjects();

	void loadChunk( const glm::vec3& vLocationm, int nRadius = 1 );

	bool m_bIsFullyLoaded = false;

protected:
	[[nodiscard]] explicit CatLevel( std::string sName, const glm::ivec2 vSize, const glm::ivec2 vChunkSize )
		: m_sName( std::move( sName ) ), m_vSize( vSize ), m_vChunkSize( vChunkSize )
	{
		m_aLoadedChunks = std::vector< bool >( m_vSize.x * m_vSize.y + 1, false );
		m_aLastLoadedChunks = std::vector< bool >( m_vSize.x * m_vSize.y + 1, false );
	}

public:
	CAT_PROPERTY( m_sName, getName, setName, m_SName );
	CAT_READONLY_PROPERTY( m_idCurrentChunk, getCurrentChunkId, m_IDCurrentChunk );
	CAT_READONLY_PROPERTY( m_pTerrain, getTerrain, m_PTerrain );
};

} // namespace cat

#endif // CATENGINE_CATLEVEL_HPP
