#include "CatObject.hpp"

#include "Cat/CatApp.hpp"
#include "Cat/CatApp.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "glm/gtc/matrix_inverse.hpp"

namespace cat
{
CatObject::id_t CatObject::m_idCurrent = 1;

glm::mat4 TransformComponent::mat4() const
{
	auto m = glm::mat4( 1.0f );

	const auto rotationRad = glm::radians( rotation );

	m = glm::translate( m, translation );
	m = glm::rotate( m, rotationRad.y, { 0.f, 1.f, 0.f } );
	m = glm::rotate( m, rotationRad.x, { 1.f, 0.f, 0.f } );
	m = glm::rotate( m, rotationRad.z, { 0.f, 0.f, 1.f } );
	m = glm::scale( m, scale );

	return m;
}

glm::mat3 TransformComponent::normalMatrix() const
{
	return glm::inverseTranspose( glm::mat3( this->mat4() ) );

	const float c3 = glm::cos( rotation.z );
	const float s3 = glm::sin( rotation.z );
	const float c2 = glm::cos( rotation.x );
	const float s2 = glm::sin( rotation.x );
	const float c1 = glm::cos( rotation.y );
	const float s1 = glm::sin( rotation.y );
	const glm::vec3 invScale = 1.0f / scale;

	return glm::mat3{
		{
			invScale.x * ( c1 * c3 + s1 * s2 * s3 ),
			invScale.x * ( c2 * s3 ),
			invScale.x * ( c1 * s2 * s3 - c3 * s1 ),
		},
		{
			invScale.y * ( c3 * s1 * s2 - c1 * s3 ),
			invScale.y * ( c2 * c3 ),
			invScale.y * ( c1 * c3 * s2 + s1 * s3 ),
		},
		{
			invScale.z * ( c2 * s1 ),
			invScale.z * ( -s2 ),
			invScale.z * ( c1 * c2 ),
		},
	};
}

json CatObject::save()
{
	json object;
	object["name"] = getName();
	object["file"] = getFileName();
	object["type"] = getType();

	object["transform"]["t"] = m_transform.translation;
	object["transform"]["r"] = m_transform.rotation;
	object["transform"]["s"] = m_transform.scale;

	object["color"] = m_vColor;

	return object;
}

void CatObject::updateTransform( const glm::vec3& vTranslation, const glm::vec3& vRotation, const glm::vec3& vScale )
{
	m_transform.translation = vTranslation;
	m_transform.rotation = vRotation;
	m_transform.scale = vScale;
}

void CatObject::updateTransform( const glm::vec3& vTranslation, const glm::vec3& vRotation )
{
	m_transform.translation = vTranslation;
	m_transform.rotation = vRotation;
}

void CatObject::updateTransform( const glm::vec3& vTranslation )
{
	m_transform.translation = vTranslation;
}
} // namespace cat
