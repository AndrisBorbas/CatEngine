#include "CatLevel.hpp"
#include "Cat/CatApp.hpp"
#include "Cat/Objects/CatLight.hpp"
#include "Cat/Objects/CatVolume.hpp"
#include "Cat/Objects/CatAssetLoader.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <fstream>

namespace cat
{

bool CatLevel::isFullyLoaded()
{
	return m_bIsFullyLoaded;
}

bool CatLevel::isLoadingFinished()
{
	if ( m_fLoaded.valid() )
	{
		using namespace std::chrono_literals;
		if ( m_fLoaded.wait_for( 0ms ) == std::future_status::ready )
		{
			m_fLoaded = {};
			m_bIsFullyLoaded = true;
			return true;
		}
	}

	return false;
}

CatObject::Map CatLevel::getAllObjects()
{
	// return m_mObjects;

	// Old copy code
	CatObject::Map mObjects;
	mObjects.insert( m_mObjects.begin(), m_mObjects.end() );
	for ( const auto& chunk : m_mChunks | std::views::values )
	{
		mObjects.insert( chunk->m_MObjects.begin(), chunk->m_MObjects.end() );
	}
	return mObjects;
}

std::unique_ptr< CatLevel > CatLevel::create( const std::string& sName,
	const glm::ivec2 vSize /*= glm::ivec2( 7, 7 )*/,
	const glm::ivec2 vChunkSize /*= glm::ivec2( 10, 10 )*/ )
{
	auto level = std::unique_ptr< CatLevel >( new CatLevel( sName, vSize, vChunkSize ) );

	auto grid = CatObject::create( "BaseGrid", "", ObjectType::eGrid );
	// grid->m_transform.translation = glm::vec3( 0.0f, 0.001f, 0.0f );
	level->m_mObjects.emplace( grid->getId(), std::move( grid ) );

	id_t id = 0;

	for ( int z = 0; z < vSize.x; ++z )
	{
		for ( int x = 0; x < vSize.y; ++x )
		{
			auto chunk = std::make_unique< CatChunk >(
				++id, glm::ivec2( x * vChunkSize.x, z * vChunkSize.y ), vChunkSize, vSize * vChunkSize );
			level->m_mChunks.emplace( chunk->m_ID, std::move( chunk ) );
		}
	}

	level->m_pTerrain = std::make_unique< CatTerrain >(
		GEI()->m_PDevice, "assets/textures/terrain_orig.tga", "assets/textures/Grass_Base_Color.tga" );

	level->m_bIsFullyLoaded = true;

	return level;
}

void CatLevel::save( const std::string& sFileName /* = "" */ )
{
	LOG_SCOPE_FUNCTION( INFO );

	if ( !sFileName.empty() )
	{
		m_sName = sFileName;
	}

	auto sPath = LEVELS_BASE_PATH + m_sName;


	nlohmann::ordered_json file;

	file["size"] = m_vSize;
	file["chunkSize"] = m_vChunkSize;

	{
		json objects = json::array();
		int i = 0;
		for ( const auto& obj : m_mObjects | std::views::values )
		{
			if ( const auto& save = obj->save(); !save.empty() )
			{
				objects[i++] = save;
			}
		}
		file["globals"] = objects;
	}

	{
		json chunks = json::array();
		int j = 0;
		for ( const auto& chunk : m_mChunks | std::views::values )
		{
			nlohmann::ordered_json chunkData;
			chunkData["id"] = chunk->m_ID;
			chunkData["position"] = chunk->m_VPosition;

			json objects = json::array();
			int i = 0;
			for ( const auto& obj : chunk->m_MObjects | std::views::values )
			{
				if ( const auto& save = obj->save(); !save.empty() )
				{
					objects[i++] = save;
				}
			}
			chunkData["objects"] = objects;

			chunks[j++] = chunkData;
		}
		file["chunks"] = chunks;
	}


	if ( !sPath.ends_with( ".json" ) )
	{
		sPath += ".json";
	}
	std::ofstream ofs( sPath );
	ofs << file.dump( -1, '\t' ) << std::endl;
	ofs.close();

	m_jData = file;

	LOG_F( INFO, "Saved level: %s", sPath.c_str() );
}

std::unique_ptr< CatLevel > CatLevel::load( const std::string& sName )
{
	LOG_SCOPE_FUNCTION( INFO );
	std::string sPath = LEVELS_BASE_PATH + sName;
	if ( !sPath.ends_with( ".json" ) )
	{
		sPath += ".json";
	}

	json jLevelData;
	std::ifstream ifs( sPath );
	ifs >> jLevelData;
	ifs.close();

	glm::ivec2 vSize = glm::make_vec2( jLevelData["size"].get< std::vector< int > >().data() );
	glm::ivec2 vChunkSize = glm::make_vec2( jLevelData["chunkSize"].get< std::vector< int > >().data() );

	// We only block to parse the level data from disk, loading objects is done async.
	auto level = create( sName, vSize, vChunkSize );
	level->m_jData = jLevelData;
	level->m_bIsFullyLoaded = false;

	auto task = []( CatLevel* level )
	{
		LOG_SCOPE_F( INFO, "Running level load task" );
		auto aGlobalFutures = std::vector< std::shared_future< std::shared_ptr< CatModel > > >();
		for ( auto& object : level->m_jData["globals"] )
		{
			if ( object.is_null() ) continue;

			// TODO: multiple objects of same model (shouldn't wait) block the object after them
			LOG_SCOPE_F( INFO, "Loading object: %s", object["name"].get< std::string >().c_str() );
			aGlobalFutures.push_back( GetEditorInstance()->m_AssetLoader.load( object, level->m_mObjects ) );
		}

		for ( auto& chunkData : level->m_jData["chunks"] )
		{
			if ( chunkData.is_null() ) continue;

			auto id = chunkData["id"].get< id_t >();
			auto vPosition = glm::make_vec2( chunkData["position"].get< std::vector< int > >().data() );

			auto chunk = level->m_mChunks.at( id ).get();
			chunk->m_VPosition = vPosition;

			for ( auto& object : chunkData["objects"] )
			{
				if ( object.is_null() ) continue;

				// TODO: multiple objects of same model (shouldn't wait) block the object after them
				LOG_SCOPE_F( INFO, "Loading object: %s", object["name"].get< std::string >().c_str() );
				aGlobalFutures.push_back( GetEditorInstance()->m_AssetLoader.load( object, chunk->m_MObjects ) );
			}
		}

		// Wait for all aGlobalFutures to finish.
		// This only blocks the level loading thread, so all objects can load on their threads while we wait for the last one to
		// finish. This esentially returns only when all objects are loaded.
		for ( auto& f : aGlobalFutures )
		{
			if ( f.valid() )
			{
				f.wait();
			}
		}

		for ( auto& chunk : level->m_mChunks | std::views::values )
		{
			for ( auto& key : chunk->m_MObjects | std::views::keys )
			{
				chunk->m_AObjectIds.push_back( key );
			}
		}

		DLOG_F( INFO, "Fully loaded level: %s", ( LEVELS_BASE_PATH + level->m_sName ).c_str() );
	};

	LOG_F( INFO, "Loaded level data: %s", ( LEVELS_BASE_PATH + level->m_sName ).c_str() );

	level->m_fLoaded = GetEditorInstance()->m_TLevelLoader.submit( std::move( task ), level.get() );

	return level;
}

void CatLevel::updateObjectLocation( id_t id )
{
	{
		auto it = m_mObjects.find( id );
		if ( it != m_mObjects.end() )
		{
			return;
		}
	}
	// Iterate through all chunks and update the object
	for ( auto& [key, chunk] : m_mChunks )
	{
		auto it = chunk->m_MObjects.find( id );
		if ( it != chunk->m_MObjects.end() )
		{
			auto obj = it->second.get();
			auto newId = getChunkAtLocation( obj->m_transform.translation );
			if ( newId <= 0 || newId > m_vSize.x * m_vSize.y ) return;

			auto newChunk = m_mChunks[newId].get();

			if ( newChunk == nullptr || newChunk->getId() == key ) return;

			newChunk->m_MObjects[obj->getId()] = std::move( it->second );
			chunk->m_MObjects.erase( it );
			return;
		}
	}
}

id_t CatLevel::getChunkAtLocation( const glm::vec3& vLocation )
{
	auto vChunkLocation = glm::ivec2( floor( vLocation.x ), floor( vLocation.z ) ) / m_vChunkSize;
	int x = floor( vChunkLocation.x );
	int y = floor( vChunkLocation.y );
	if ( x < 0 || x >= m_vSize.x || y < 0 || y >= m_vSize.y ) return 0;
	return ( y * m_vSize.y ) + x + 1;
}

void CatLevel::loadChunk( const glm::vec3& vLocation, int nRadius /* = 1 */ )
{
	auto id = getChunkAtLocation( vLocation );
	if ( id <= 0 || id > m_vSize.x * m_vSize.y ) return;

	std::swap( m_aLoadedChunks, m_aLastLoadedChunks );
	std::fill( m_aLoadedChunks.begin(), m_aLoadedChunks.end(), false );

	if ( m_mChunks[id]->load() )
	{
		m_aLoadedChunks[id] = true;
	}

	int lx = -std::min( m_mChunks[id]->m_VPosition.x / m_vChunkSize.x, nRadius );
	int hx = std::min( m_mChunks[id]->m_VPosition.x / m_vChunkSize.x, nRadius );

	for ( int j = -nRadius; j <= nRadius; j++ )
	{
		for ( int i = -nRadius; i <= nRadius; i++ )
		{
			auto neighbourId = id + ( j * m_vSize.y ) + i;
			if ( neighbourId <= 0 || neighbourId > m_vSize.x * m_vSize.y ) continue;
			// dont wrap on edge
			// if ( m_mChunks[neighbourId]->m_VPosition.x + i <= 0 || m_mChunks[neighbourId]->m_VPosition.x + i >= m_vSize.x )
			// continue;

			if ( m_mChunks[neighbourId]->load() )
			{
				m_aLoadedChunks[neighbourId] = true;
			}
		}
	}

	// Unload chunks that are too far away

	for ( int i = 0; i < m_aLastLoadedChunks.size(); i++ )
	{
		if ( m_aLastLoadedChunks[i] && !m_aLoadedChunks[i] )
		{
			m_mChunks[i]->unload();
		}
	}
}

// 01  2  3  4  5  6  7  8  9 10
// 11 12 13 14 15 16 17 18 19 20
// 21 22 23 24 25 26 27 28 29 30
// 31 32 33 34 35 36 37 38 39 40
// 41 42 43 44 45 46 47 48 49 50
// 51 52 53 54 55 56 57 58 59 60

} // namespace cat
