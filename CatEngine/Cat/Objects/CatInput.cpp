#include "CatInput.hpp"

#include <limits>

namespace cat
{
void CatInput::moveInPlaneXY( GLFWwindow* window, float dt, CatObject& gameObject )
{
	glm::vec3 rotate{ 0 };
	if ( glfwGetKey( window, keys.lookRight ) == GLFW_PRESS ) rotate.z += 1.f;
	if ( glfwGetKey( window, keys.lookLeft ) == GLFW_PRESS ) rotate.z -= 1.f;
	if ( glfwGetKey( window, keys.lookUp ) == GLFW_PRESS ) rotate.x += 1.f;
	if ( glfwGetKey( window, keys.lookDown ) == GLFW_PRESS ) rotate.x -= 1.f;

	if ( glm::dot( rotate, rotate ) > std::numeric_limits< float >::epsilon() )
	{
		gameObject.m_transform.rotation += lookSpeed * dt * glm::normalize( rotate );
	}

	// limit pitch values between about +/- 85ish degrees
	gameObject.m_transform.rotation.x = glm::clamp( gameObject.m_transform.rotation.x, -1.5f, 1.5f );
	gameObject.m_transform.rotation.z = glm::mod( gameObject.m_transform.rotation.z, glm::two_pi< float >() );

	float yaw = gameObject.m_transform.rotation.z;
	const glm::vec3 forwardDir{ sin( yaw ), cos( yaw ), 0.f };
	const glm::vec3 rightDir{ forwardDir.y, -forwardDir.x, 0.f };
	const glm::vec3 upDir{ 0.f, 0.f, -1.0f };

	glm::vec3 moveDir{ 0.f };
	if ( glfwGetKey( window, keys.moveForward ) == GLFW_PRESS ) moveDir += forwardDir;
	if ( glfwGetKey( window, keys.moveBackward ) == GLFW_PRESS ) moveDir -= forwardDir;
	if ( glfwGetKey( window, keys.moveRight ) == GLFW_PRESS ) moveDir += rightDir;
	if ( glfwGetKey( window, keys.moveLeft ) == GLFW_PRESS ) moveDir -= rightDir;
	if ( glfwGetKey( window, keys.moveUp ) == GLFW_PRESS ) moveDir += upDir;
	if ( glfwGetKey( window, keys.moveDown ) == GLFW_PRESS ) moveDir -= upDir;

	if ( glm::dot( moveDir, moveDir ) > std::numeric_limits< float >::epsilon() )
	{
		gameObject.m_transform.translation += moveSpeed * dt * glm::normalize( moveDir );
	}
}
} // namespace cat
