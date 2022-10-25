#include <fstream>
#include "CatLevel.hpp"
#include "Cat/CatApp.hpp"
#include "Cat/Objects/CatLight.hpp"
#include "Cat/Objects/CatAssetLoader.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/constants.hpp>

namespace cat
{
std::unique_ptr< CatLevel > CatLevel::create( const std::string& sName,
	const glm::ivec2 vSize /*= glm::ivec2( 9, 9 )*/,
	const glm::ivec2 vChunkSize /*= glm::ivec2( 32, 32 )*/ )
{
	auto level = std::unique_ptr< CatLevel >( new CatLevel( sName, vSize, vChunkSize ) );

	auto grid = CatObject::create( "BaseGrid", "", ObjectType::eGrid );
	level->m_mObjects.emplace( grid->getId(), std::move( grid ) );

	return level;
}

void CatLevel::save()
{
	const auto sPath = LEVELS_BASE_PATH + m_sName;
	json file;

	file["size"] = m_vSize;
	file["chunkSize"] = m_vChunkSize;

	json objects;
	int i = 0;
	for ( const auto& obj : m_mObjects | std::views::values )
	{
		objects[i] = obj->save();
		++i;
	}
	file["globals"] = objects;

	std::ofstream ofs( sPath );
	ofs << file.dump( -1, '\t' ) << std::endl;
	ofs.close();

	DLOG_F( INFO, "Saved level: %s", sPath.c_str() );
}

std::unique_ptr< CatLevel > CatLevel::load( const std::string& sName )
{
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
		DLOG_F( INFO, "Running load task" );
		auto aGlobalFutures = std::vector< std::shared_future< std::shared_ptr< CatModel > > >();
		for ( auto& object : file["globals"] )
		{
			DLOG_F( INFO, "Loading object: %s", object["name"].get< std::string >().c_str() );
			aGlobalFutures.push_back( GetEditorInstance()->m_AssetLoader.load( object, level->m_mObjects ) );
		}

		for ( auto& chunk : file["chunks"] )
		{
			// TODO: create new chunk and load it.
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

		DLOG_F( INFO, "Fully Loaded level %s", level->m_sName.c_str() );
	};

	level->m_fLoaded = GetEditorInstance()->m_TLevelLoader.submit( std::move( task ), file, level.get() );

	DLOG_F( INFO, "Loaded level: %s", sPath.c_str() );

	return level;
}

} // namespace cat
