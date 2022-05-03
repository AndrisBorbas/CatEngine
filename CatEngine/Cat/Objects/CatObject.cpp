#include "CatObject.hpp"

#include "Cat/CatApp.hpp"
#include "Cat/CatApp.hpp"
#include "glm/gtc/matrix_inverse.hpp"

namespace cat
{
glm::mat4 TransformComponent::mat4() const
{
	auto m = glm::mat4( *ID_MX );

	m = glm::translate( m, translation );
	m = glm::scale( m, scale );
	m = glm::rotate( m, rotation.y, { 0.f, 1.f, 0.f } );
	m = glm::rotate( m, rotation.x, { 1.f, 0.f, 0.f } );
	m = glm::rotate( m, rotation.z, { 0.f, 0.f, 1.f } );

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

CatObject CatObject::makePointLight( const std::string& sName /* = "Light" */,
	const float fIntensity /* = 10.f */,
	const float fRadius /* = .1f */,
	const glm::vec3 vColor /* = glm::vec3( 1.f ) */ )
{
	CatObject gameObj = CatObject::createObject( sName, sName );
	gameObj.m_vColor = vColor;
	gameObj.m_transform.scale.x = fRadius;
	gameObj.m_pPointLight = std::make_unique< PointLightComponent >();
	gameObj.m_pPointLight->lightIntensity = fIntensity;
	return gameObj;
}
} // namespace cat
