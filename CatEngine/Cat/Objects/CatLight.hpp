#ifndef CATENGINE_CATLIGHT_HPP
#define CATENGINE_CATLIGHT_HPP

#include "CatObject.hpp"

namespace cat
{

class CatLight : public CatObject
{
public:
	[[nodiscard]] static std::unique_ptr< CatLight > create( const std::string& sName,
		const glm::vec3 vColor = glm::vec3( 1.f ),
		const float fIntensity = 10.f,
		const float fRadius = .1f,
		const ObjectType& eType = ObjectType::eLight )
	{
		auto light = std::unique_ptr< CatLight >( new CatLight( sName, eType ) );
		light->m_vColor = vColor;
		light->m_transform.scale.x = fIntensity;
		light->m_transform.scale.y = fRadius;
		return light;
	}

	virtual ~CatLight() override = default;

protected:
	[[nodiscard]] explicit CatLight( const std::string& sName, const ObjectType& eType = ObjectType::eLight )
		: CatObject( sName, std::string(), eType )
	{
	}
};

} // namespace cat

#endif // CATENGINE_CATLIGHT_HPP
