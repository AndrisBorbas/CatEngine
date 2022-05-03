#ifndef CATENGINE_CATCAMERA_HPP
#define CATENGINE_CATCAMERA_HPP

#include "glm/glm.hpp"

namespace cat
{
class CatCamera
{
public:
	void setOrthographicProjection( float fLeft, float fRight, float fTop, float fBottom, float fNear, float fFar );

	void setPerspectiveProjection( float fFOVY, float fAspectRatio, float fNear, float fFar );
	void setPerspectiveProjectionRH( float fFOVY, float fAspectRatio, float fNear, float fFar );

	void setViewDirection( glm::vec3 vPosition, glm::vec3 vDirection, glm::vec3 vUp = glm::vec3{ 0.f, 1.f, 0.f } );
	void setViewTarget( glm::vec3 vPosition, glm::vec3 vTarget, glm::vec3 vUp = glm::vec3{ 0.f, 1.f, 0.f } );
	void setViewYXZ( glm::vec3 vPosition, glm::vec3 vRotation );
	void setViewYXZRH( glm::vec3 vPosition, glm::vec3 vRotation );

	[[nodiscard]] const glm::mat4& getProjection() const { return m_mxProjection; }
	[[nodiscard]] const glm::mat4& getView() const { return m_mxView; }
	[[nodiscard]] const glm::mat4& getInverseView() const { return m_mxInverseViewMatrix; }
	[[nodiscard]] glm::vec3 getPosition() const { return { m_mxInverseViewMatrix[3] }; }

private:
	glm::mat4 m_mxProjection{ 1.f };
	glm::mat4 m_mxView{ 1.f };
	glm::mat4 m_mxInverseViewMatrix{ 1.f };
};
} // namespace cat


#endif // CATENGINE_CATCAMERA_HPP
