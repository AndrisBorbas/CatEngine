#include <fstream>
#include "CatLevel.hpp"
#include "Cat/CatApp.hpp"
#include "Cat/Objects/CatLight.hpp"
#include "Cat/Objects/CatVolume.hpp"
#include "Cat/Objects/CatAssetLoader.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>

namespace cat
{

CatChunk::CatChunk( const id_t id, glm::ivec2 vPosition, glm::ivec2 vSize, glm::ivec2 vMaxSize )
	: m_id( id ), m_vPosition( vPosition )
{
	auto fMesh = GetEditorInstance()->m_AssetLoader.get( "assets/models/cube.obj" );

	auto volume = CatVolume::create( "ChunkVisualizer" );
	volume->m_transform.translation = glm::vec3( vPosition.x + vSize.x / 2.f, -0.02f, vPosition.y + vSize.y / 2.f );
	volume->m_transform.scale = glm::vec3( vSize.x / 2.f, 0.02f, vSize.y / 2.f );
	volume->m_vColor =
		glm::vec3( float( vPosition.x ) / float( vMaxSize.x ), 0.25f, float( vPosition.y ) / float( vMaxSize.y ) );

	fMesh.wait();
	volume->m_pModel = fMesh.get();
	m_mObjects.emplace( volume->getId(), std::move( volume ) );
}

bool CatLevel::isFullyLoaded()
{
	if ( m_fLoaded.valid() )
	{
		using namespace std::chrono_literals;
		if ( m_fLoaded.wait_for( 0ms ) == std::future_status::ready )
		{
			m_fLoaded = {};
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

	json file;
	std::ifstream ifs( sPath );
	ifs >> file;
	ifs.close();

	glm::ivec2 vSize = glm::make_vec2( file["size"].get< std::vector< int > >().data() );
	glm::ivec2 vChunkSize = glm::make_vec2( file["chunkSize"].get< std::vector< int > >().data() );

	// We only block to parse the level data from disk, loading objects is done async.
	auto level = create( sName, vSize, vChunkSize );

	auto task = []( json file, CatLevel* level )
	{
		LOG_SCOPE_F( INFO, "Running level load task" );
		auto aGlobalFutures = std::vector< std::shared_future< std::shared_ptr< CatModel > > >();
		for ( auto& object : file["globals"] )
		{
			if ( object.is_null() ) continue;

			// TODO: multiple objects of same model (shouldn't wait) block the object after them
			LOG_SCOPE_F( INFO, "Loading object: %s", object["name"].get< std::string >().c_str() );
			aGlobalFutures.push_back( GetEditorInstance()->m_AssetLoader.load( object, level->m_mObjects ) );
		}

		for ( auto& chunkData : file["chunks"] )
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

	level->m_fLoaded = GetEditorInstance()->m_TLevelLoader.submit( std::move( task ), file, level.get() );

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
			auto newChunk = m_mChunks[getChunkAtLocation( obj->m_transform.translation )].get();

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
	return ( x * m_vSize.x ) + y + 1;
}
} // namespace cat
