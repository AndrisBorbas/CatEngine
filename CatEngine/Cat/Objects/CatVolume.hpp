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


	[[nodiscard]] virtual const std::string& getLoadLevel() const { return m_sLoadLevel; }
	virtual void setLoadLevel( const std::string& sLoadLevel ) { m_sLoadLevel = sLoadLevel; }
	__declspec( property( get = getLoadLevel, put = setLoadLevel ) ) std::string m_SLoadLevel;

	[[nodiscard]] virtual bool isIsLoaded() const { return m_bIsLoaded; }
	virtual void setIsLoaded( const bool isLoaded ) { m_bIsLoaded = isLoaded; }
	__declspec( property( get = isIsLoaded, put = setIsLoaded ) ) bool m_BIsLoaded;

	[[nodiscard]] virtual bool isIsSaved() const { return m_bIsSaved; }
	virtual void setIsSaved( const bool isSaved ) { m_bIsSaved = isSaved; }
	__declspec( property( get = isIsSaved, put = setIsSaved ) ) bool m_BIsSaved;


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
	CAT_PROPERTY( m_sSaveLevel, getSaveLevel, setSaveLevel, sSaveLevel, m_SSaveLevel )
};

} // namespace cat

#endif // CATENGINE_CATVOLUME_HPP
