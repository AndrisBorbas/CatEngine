#ifndef CATENGINE_CATVOLUME_HPP
#define CATENGINE_CATVOLUME_HPP

#include "CatObject.hpp"
#include "Cat/CatApp.hpp"

namespace cat
{
class CatVolume : public CatObject
{
public:
	[[nodiscard]] static std::unique_ptr< CatVolume > create( const std::string& sName, const std::string& sFile )
	{
		auto volume = std::make_unique< CatVolume >( sName, sFile );
		volume->m_vColor = { 1.0f, 0.0f, 0.0f };
		return volume;
	}

	[[nodiscard]] CatVolume( const std::string& sName, const std::string& sFile ) : CatObject( sName, sFile ) {}
};

} // namespace cat

#endif // CATENGINE_CATVOLUME_HPP
