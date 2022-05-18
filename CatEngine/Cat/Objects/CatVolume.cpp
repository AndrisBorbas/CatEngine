#include "CatVolume.hpp"

#include "glm/gtc/type_ptr.hpp"

namespace cat
{
bool CatVolume::isInside( const CatObject& other )
{
	glm::vec3 point = other.m_transform.translation;
	glm::vec3 center = m_transform.translation;
	glm::vec3 scale = m_transform.scale;

	if ( ( point.x < center.x + scale.x && point.x > center.x - scale.x )
		 && ( point.y < center.y + scale.y && point.y > center.y - scale.y )
		 && ( point.z < center.z + scale.z && point.z > center.z - scale.z ) )
	{
		return true;
	}
	return false;
}

json CatVolume::save()
{
	auto object = CatObject::save();

	object["trigger"]["saveLevel"] = m_SSaveLevel;
	object["trigger"]["loadLevel"] = m_SLoadLevel;

	return object;
}

} // namespace cat
