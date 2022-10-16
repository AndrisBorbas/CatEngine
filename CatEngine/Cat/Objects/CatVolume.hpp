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
		const std::string& sFile = "assets/models/cube.obj" )
	{
		auto volume = std::unique_ptr< CatVolume >( new CatVolume( sName, sFile ) );
		volume->m_vColor = { 1.0f, 0.0f, 0.0f };
		return volume;
	}

	virtual bool isInside( const CatObject& other );
	json save() override;
	
	virtual ~CatVolume() override = default;

protected:
	[[nodiscard]] CatVolume( const std::string& sName,
		const std::string& sFile,
		const CatObject::Type& sType = "TriggerVolume" )
		: CatObject( sName, sFile, sType )
	{
	}

	std::string m_sSaveLevel;
	std::string m_sLoadLevel;

	bool m_bIsLoaded = false;
	bool m_bIsSaved = false;

public:
	CAT_PROPERTY( m_sSaveLevel, getSaveLevel, setSaveLevel, sSaveLevel, m_SSaveLevel );
	CAT_PROPERTY( m_sLoadLevel, getLoadLevel, setLoadLevel, sLoadLevel, m_SLoadLevel );
	CAT_PROPERTY( m_bIsLoaded, isLoaded, setIsLoaded, bIsLoaded, m_BIsLoaded );
	CAT_PROPERTY( m_bIsSaved, isSaved, setIsSaved, bIsSaved, m_BIsSaved );
};

} // namespace cat

#endif // CATENGINE_CATVOLUME_HPP
