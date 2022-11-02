#ifndef CATENGINE_CATVOLUME_HPP
#define CATENGINE_CATVOLUME_HPP

#include "CatObject.hpp"
#include "Cat/CatApp.hpp"

namespace cat
{


class CatVolume : public CatObject
{
public:
	[[nodiscard]] static std::unique_ptr< CatVolume > create( const std::string& sName,
		const std::string& sFile = "assets/models/cube.obj",
		const ObjectType& eType = ObjectType::eVolume )
	{
		auto volume = std::unique_ptr< CatVolume >( new CatVolume( sName, sFile, eType ) );
		volume->m_vColor = { 1.0f, 0.0f, 0.0f };
		return volume;
	}

	virtual bool isInside( const CatObject& other );
	json save() override;

	virtual ~CatVolume() override = default;

protected:
	[[nodiscard]] CatVolume( const std::string& sName,
		const std::string& sFile = "assets/models/cube.obj",
		const ObjectType& eType = ObjectType::eVolume )
		: CatObject( sName, sFile, eType )
	{
	}

	std::string m_sSaveLevel;
	std::string m_sLoadLevel;
	bool m_bIsLoaded = false;
	bool m_bIsSaved = false;

public:
	CAT_PROPERTY( m_sSaveLevel, getSaveLevel, setSaveLevel, m_SSaveLevel );
	CAT_PROPERTY( m_sLoadLevel, getLoadLevel, setLoadLevel, m_SLoadLevel );
	CAT_PROPERTY( m_bIsLoaded, isLoaded, setIsLoaded, m_BIsLoaded );
	CAT_PROPERTY( m_bIsSaved, isSaved, setIsSaved, m_BIsSaved );
};

} // namespace cat

#endif // CATENGINE_CATVOLUME_HPP
