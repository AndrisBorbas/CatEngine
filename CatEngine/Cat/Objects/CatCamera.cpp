#include "CatCamera.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace cat
{
void CatCamera::setOrthographicProjection( float fLeft, float fRight, float fTop, float fBottom, float fNear, float fFar )
{
	m_mxProjection = glm::mat4{ 1.0f };
	m_mxProjection[0][0] = 2.f / ( fRight - fLeft );
	m_mxProjection[1][1] = 2.f / ( fBottom - fTop );
	m_mxProjection[2][2] = 1.f / ( fFar - fNear );
	m_mxProjection[3][0] = -( fRight + fLeft ) / ( fRight - fLeft );
	m_mxProjection[3][1] = -( fBottom + fTop ) / ( fBottom - fTop );
	m_mxProjection[3][2] = -fNear / ( fFar - fNear );
}

void CatCamera::setPerspectiveProjection( float fFOVY, float fAspectRatio, float fNear, float fFar )
{
	assert( glm::abs( fAspectRatio - std::numeric_limits< float >::epsilon() ) > 0.0f );
	const float tanHalfFOVY = tan( fFOVY / 2.f );
	m_mxProjection = glm::mat4{ 0.0f };
	m_mxProjection[0][0] = 1.f / ( fAspectRatio * tanHalfFOVY );
	m_mxProjection[1][1] = 1.f / ( tanHalfFOVY );
	m_mxProjection[2][2] = fFar / ( fFar - fNear );
	m_mxProjection[2][3] = 1.f;
	m_mxProjection[3][2] = -( fFar * fNear ) / ( fFar - fNear );
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
}

void CatCamera::setViewTarget( glm::vec3 vPosition, glm::vec3 vTarget, glm::vec3 vUp )
{
	setViewDirection( vPosition, vTarget - vPosition, vUp );
}

void CatCamera::setViewYXZ( glm::vec3 vPosition, glm::vec3 vRotation )
{
	m_mxView = glm::lookAt( vPosition, vPosition + vRotation, glm::vec3{ 0.f, 0.f, 1.f } );
}
} // namespace cat
