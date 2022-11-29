#include "CatCamera.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace cat
{
void CatCamera::setOrthographicProjection( float fLeft, float fRight, float fTop, float fBottom, float fNear, float fFar )
{
	m_mxProjection = glm::orthoRH_ZO( fLeft, fRight, fBottom, fTop, fNear, fFar );
	m_mxProjection[1][1] *= -1.f;
}

void CatCamera::setPerspectiveProjection( float fFOVY, float fAspectRatio, float fNear, float fFar )
{
	m_mxProjection = glm::perspectiveRH_ZO( fFOVY, fAspectRatio, fNear, fFar );
	m_mxProjection[1][1] *= -1.f;
}

void CatCamera::setPerspectiveProjectionImGuizmo( float fFOVY, float fAspectRatio, float fNear, float fFar )
{
	m_mxProjection = glm::perspectiveRH_ZO( fFOVY, fAspectRatio, fNear, fFar );
	// Not needed in ImGuizmo
	// m_mxProjection[1][1] *= -1.f;
}

void CatCamera::setViewDirection( glm::vec3 vPosition, glm::vec3 vDirection, glm::vec3 vUp )
{
	const glm::vec3 w{ glm::normalize( vDirection ) };
	const glm::vec3 u{ glm::normalize( glm::cross( w, vUp ) ) };
	const glm::vec3 v{ glm::cross( w, u ) };

	m_mxView = glm::mat4{ 1.f };
	m_mxView[0][0] = u.x;
	m_mxView[1][0] = u.y;
	m_mxView[2][0] = u.z;
	m_mxView[0][1] = v.x;
	m_mxView[1][1] = v.y;
	m_mxView[2][1] = v.z;
	m_mxView[0][2] = w.x;
	m_mxView[1][2] = w.y;
	m_mxView[2][2] = w.z;
	m_mxView[3][0] = -glm::dot( u, vPosition );
	m_mxView[3][1] = -glm::dot( v, vPosition );
	m_mxView[3][2] = -glm::dot( w, vPosition );

	m_mxInverseViewMatrix = glm::inverse( m_mxView );
}

void CatCamera::setViewTarget( glm::vec3 vPosition, glm::vec3 vTarget, glm::vec3 vUp )
{
	setViewDirection( vPosition, vTarget - vPosition, vUp );
}

void CatCamera::setViewYXZ( glm::vec3 vPosition, glm::vec3 vRotation )
{
	m_mxView = glm::lookAtRH( vPosition, vPosition + vRotation, glm::vec3{ 0.f, 1.f, 0.f } );

	m_mxInverseViewMatrix = glm::inverse( m_mxView );

	// Old view matrix builder code
	/*
	const float c3 = glm::cos( vRotation.z );
	const float s3 = glm::sin( vRotation.z );
	const float c2 = glm::cos( vRotation.x );
	const float s2 = glm::sin( vRotation.x );
	const float c1 = glm::cos( vRotation.y );
	const float s1 = glm::sin( vRotation.y );
	const glm::vec3 u{ ( c1 * c3 + s1 * s2 * s3 ), ( c2 * s3 ), ( c1 * s2 * s3 - c3 * s1 ) };
	const glm::vec3 v{ ( c3 * s1 * s2 - c1 * s3 ), ( c2 * c3 ), ( c1 * c3 * s2 + s1 * s3 ) };
	const glm::vec3 w{ ( c2 * s1 ), ( -s2 ), ( c1 * c2 ) };
	m_mxView = glm::mat4{ 1.f };
	m_mxView[0][0] = u.x;
	m_mxView[1][0] = u.y;
	m_mxView[2][0] = u.z;
	m_mxView[0][1] = v.x;
	m_mxView[1][1] = v.y;
	m_mxView[2][1] = v.z;
	m_mxView[0][2] = w.x;
	m_mxView[1][2] = w.y;
	m_mxView[2][2] = w.z;
	m_mxView[3][0] = -glm::dot( u, vPosition );
	m_mxView[3][1] = -glm::dot( v, vPosition );
	m_mxView[3][2] = -glm::dot( w, vPosition );
	*/
}

} // namespace cat
