#ifndef CATENGINE_CATLIGHT_HPP
#define CATENGINE_CATLIGHT_HPP

#include "CatObject.hpp"

namespace cat
{

class CatLight : public CatObject
{
public:
	[[nodiscard]] static std::unique_ptr< CatLight > create( const std::string& sName,
		const std::string& sFile = "Light",
		const glm::vec3 vColor = glm::vec3( 1.f ),
		const float fIntensity = 10.f,
		const float fRadius = .1f )
	{
		auto light = std::unique_ptr< CatLight >( new CatLight( sName, sFile ) );
		light->m_vColor = vColor;
		light->m_transform.scale.x = fIntensity;
		light->m_transform.scale.y = fRadius;
		return light;
	}

	virtual ~CatLight() override = default;

protected:
	[[nodiscard]] CatLight( const std::string& sName, const std::string& sFile, const CatObject::Type& sType = "Light" )
		: CatObject( sName, sFile, sType )
	{
	}
};

} // namespace cat

#endif // CATENGINE_CATLIGHT_HPP
