#ifndef CATENGINE_CATASSETLOADER_HPP
#define CATENGINE_CATASSETLOADER_HPP

#include "Globals.hpp"
#include "CatObjectType.hpp"
#include "CatObject.hpp"

#include <string>
#include <memory>
#include <future>

namespace cat
{

class CatAssetLoader
{
private:
	std::unordered_map< std::string, std::shared_future< std::shared_ptr< CatModel > > > m_mModelCache;

public:
	CatAssetLoader() = default;
	virtual ~CatAssetLoader() = default;

	std::shared_future< std::shared_ptr< CatModel > > load( const json& object, CatObject::Map& mObjects );

	CAT_READONLY_PROPERTY( m_mModelCache, getModelCache, mModelCache, m_MModelCache );
};

} // namespace cat

#endif // CATENGINE_CATASSETLOADER_HPP
