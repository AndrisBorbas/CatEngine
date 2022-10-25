#ifndef CATENGINE_CATLEVEL_HPP
#define CATENGINE_CATLEVEL_HPP

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
	const glm::ivec2 m_vPosition;
	CatObject::Map m_mObjects;

public:
	CatChunk( const id_t id, glm::ivec2 vPosition, glm::ivec2 vSize ) : m_id( id ), m_vPosition( vPosition ) {}
	~CatChunk() = default;
};

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
	std::vector< id_t > m_aLoadedChunkIds;
	CatObject::Map m_mObjects;
	std::future< void > m_fLoaded;


public:
	virtual ~CatLevel() = default;

	[[nodiscard]] static std::unique_ptr< CatLevel > create( const std::string& sName,
		glm::ivec2 vSize = glm::ivec2( 9, 9 ),
		glm::ivec2 vChunkSize = glm::ivec2( 32, 32 ) );
	void save();
	[[nodiscard]] static std::unique_ptr< CatLevel > load( const std::string& sName );

	bool isFullyLoaded()
	{
		using namespace std::chrono_literals;
		return m_fLoaded.valid() && m_fLoaded.wait_for( 0ns ) == std::future_status::ready;
	}

	auto& getAllObjects() { return m_mObjects; }

protected:
	[[nodiscard]] explicit CatLevel( std::string sName, const glm::ivec2 vSize, const glm::ivec2 vChunkSize )
		: m_sName( std::move( sName ) ), m_vSize( vSize ), m_vChunkSize( vChunkSize )
	{
	}

public:
	CAT_PROPERTY( m_sName, getName, setName, sName, m_SName );
};

} // namespace cat

#endif // CATENGINE_CATLEVEL_HPP