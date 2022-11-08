#include "CatAssetLoader.hpp"
#include "CatLight.hpp"
#include "Cat/CatApp.hpp"

namespace cat
{
std::shared_future< std::shared_ptr< CatModel > > CatAssetLoader::load( const json& object, CatObject::Map& mObjects )
{
	auto type = cat::ObjectType( object["type"] );
	if ( type >= cat::ObjectType::eCamera )
	{
		return {};
	}
	if ( type >= cat::ObjectType::eLight )
	{
		auto light = CatLight::create( object["name"] );

		light->load( object );

		mObjects[light->getId()] = std::move( light );

		return {};
	}
	if ( type >= cat::ObjectType::eGameObject )
	{
		// Load object if it's not already loaded.
		if ( !m_mModelCache.contains( object["file"] ) )
		{
			auto task = [&object, &mObjects]()
			{
				auto model = CatModel::createModelFromFile( GetEditorInstance()->m_PDevice, object["file"] );

				auto obj = CatObject::create( object["name"], object["file"] );

				obj->load( object );
				obj->m_pModel = model;

				mObjects[obj->getId()] = std::move( obj );

				return model;
			};
			auto sharedFuture = GetEditorInstance()->m_TAssetLoader.submit( std::move( task ) ).share();

			m_mModelCache[object["file"]] = sharedFuture;
			return sharedFuture;
		}
		else
		{
			auto obj = CatObject::create( object["name"], object["file"] );

			obj->load( object );
			obj->m_pModel = m_mModelCache[object["file"]].get();

			mObjects[obj->getId()] = std::move( obj );

			return {};
		}
	}

	return {};
}

std::shared_future< std::shared_ptr< CatModel > > CatAssetLoader::get( const std::string& file )
{
	if ( !m_mModelCache.contains( file ) )
	{
		auto task = [file]() { return CatModel::createModelFromFile( GetEditorInstance()->m_PDevice, file ); };
		auto sharedFuture = GetEditorInstance()->m_TAssetLoader.submit( std::move( task ) ).share();

		m_mModelCache[file] = sharedFuture;
		return sharedFuture;
	}
	else
	{
		return m_mModelCache[file];
	}
}

} // namespace cat
