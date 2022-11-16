#include "CatChunk.hpp"

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
	volume->m_BVisible = false;

	fMesh.wait();
	volume->m_pModel = fMesh.get();
	m_mObjects.emplace( volume->getId(), std::move( volume ) );
}

bool CatChunk::load()
{
	if ( m_bLoaded ) return true;

	std::for_each( m_mObjects.begin(), m_mObjects.end(), []( auto& pair ) { pair.second->m_BVisible = true; } );

	m_bLoaded = true;

	return true;
}

bool CatChunk::unload()
{
	if ( !m_bLoaded ) return false;

	std::for_each( m_mObjects.begin(), m_mObjects.end(), []( auto& pair ) { pair.second->m_BVisible = false; } );

	m_bLoaded = false;

	return true;
}

} // namespace cat
