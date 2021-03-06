#include "CatInput.hpp"

#include <limits>

namespace cat
{
void CatInput::moveInPlaneXZ( GLFWwindow* window, float dt, CatObject& gameObject )
{
	if ( glfwGetMouseButton( window, m_eKeys.look ) == GLFW_RELEASE )
	{
		m_bFirstMouse = true;
		glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
	}

	if ( glfwGetMouseButton( window, m_eKeys.look ) == GLFW_PRESS )
	{
		glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
		double* pXPos = new double;
		double* pYPos = new double;
		glfwGetCursorPos( window, pXPos, pYPos );
		if ( pXPos && pYPos )
		{
			if ( m_bFirstMouse )
			{
				m_vMouseLastPos.x = *pXPos;
				m_vMouseLastPos.y = *pYPos;
				m_bFirstMouse = false;
			}

			float fXOffset = *pXPos - m_vMouseLastPos.x;
			float fYOffset = m_vMouseLastPos.y - *pYPos; // reversed since y-coordinates go from bottom to top

			m_vMouseLastPos.x = *pXPos;
			m_vMouseLastPos.y = *pYPos;

			m_fYaw += fXOffset * m_fMouseSensitivity;
			m_fPitch += fYOffset * m_fMouseSensitivity;
		}
		delete pXPos;
		delete pYPos;
	}
	m_fPitch = glm::clamp( m_fPitch, -1.5f, 1.5f );
	m_fYaw = glm::mod( m_fYaw, glm::two_pi< float >() );

	glm::vec3 vFront;
	vFront.x = cos( m_fYaw ) * cos( m_fPitch );
	vFront.y = sin( m_fPitch );
	vFront.z = sin( m_fYaw ) * cos( m_fPitch );
	// normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower
	// movement.
	vFront = glm::normalize( vFront );
	glm::vec3 vRight = glm::normalize( glm::cross( vFront, glm::vec3{ 0.f, 1.f, 0.f } ) );
	glm::vec3 vUp = glm::normalize( glm::cross( vRight, vFront ) );

	gameObject.m_transform.rotation = vFront;

	glm::vec3 moveDir{ 0.f };
	if ( glfwGetKey( window, m_eKeys.moveForward ) == GLFW_PRESS ) moveDir += vFront;
	if ( glfwGetKey( window, m_eKeys.moveBackward ) == GLFW_PRESS ) moveDir -= vFront;
	if ( glfwGetKey( window, m_eKeys.moveRight ) == GLFW_PRESS ) moveDir += vRight;
	if ( glfwGetKey( window, m_eKeys.moveLeft ) == GLFW_PRESS ) moveDir -= vRight;
	if ( glfwGetKey( window, m_eKeys.moveUp ) == GLFW_PRESS ) moveDir += vUp;
	if ( glfwGetKey( window, m_eKeys.moveDown ) == GLFW_PRESS ) moveDir -= vUp;

	if ( glfwGetKey( window, m_eKeys.speed ) == GLFW_PRESS ) m_fMovementSpeed = 6.9f;
	if ( glfwGetKey( window, m_eKeys.speed ) == GLFW_RELEASE ) m_fMovementSpeed = 3.0f;

	if ( glm::dot( moveDir, moveDir ) > std::numeric_limits< float >::epsilon() )
	{
		gameObject.m_transform.translation += m_fMovementSpeed * dt * glm::normalize( moveDir );
	}
}
} // namespace cat
