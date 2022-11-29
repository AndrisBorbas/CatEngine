#include "CatInput.hpp"
#include "Cat/CatApp.hpp"

#include <limits>

namespace cat
{
bool CatInput::moveInPlaneXZ( GLFWwindow* window, float dt, CatObject& gameObject )
{
	bool bWasMatrixupdated = false;
	if ( glfwGetMouseButton( window, m_eKeys.look ) == GLFW_RELEASE )
	{
		m_bFirstMouse = true;
		glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
	}

	auto fLastPitch = m_fPitch;
	auto fLastYaw = m_fYaw;

	if ( glfwGetMouseButton( window, m_eKeys.look ) == GLFW_PRESS )
	{
		glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
		auto* pXPos = new double;
		auto* pYPos = new double;
		glfwGetCursorPos( window, pXPos, pYPos );

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

		delete pXPos;
		delete pYPos;
	}
	m_fPitch = glm::clamp( m_fPitch, -1.5f, 1.5f );
	m_fYaw = glm::mod( m_fYaw, glm::two_pi< float >() );

	if ( fLastPitch != m_fPitch || fLastYaw != m_fYaw )
	{
		bWasMatrixupdated = true;
	}

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

	if ( glfwGetKey( window, m_eKeys.speed ) == GLFW_PRESS ) m_fMovementSpeed = 3.0f * GetEditorInstance()->m_FCameraSpeed;
	if ( glfwGetKey( window, m_eKeys.speed ) == GLFW_RELEASE ) m_fMovementSpeed = 3.0f;

	if ( glm::dot( moveDir, moveDir ) > std::numeric_limits< float >::epsilon() )
	{
		gameObject.m_transform.translation += m_fMovementSpeed * dt * glm::normalize( moveDir );
		bWasMatrixupdated = true;
	}

	return bWasMatrixupdated;
}

void CatInput::registerInputHandlers()
{
	GEI()->m_FKeyCallback = glfwSetKeyCallback( **GEI()->m_PWindow, nullptr );
	glfwSetKeyCallback( **GEI()->m_PWindow,
		[]( GLFWwindow* window, int key, int scancode, int action, int mods )
		{
			GEI()->m_FKeyCallback( window, key, scancode, action, mods );
			if ( key == GLFW_KEY_ENTER && action == GLFW_PRESS && mods == GLFW_MOD_ALT )
			{
				GEI()->m_PWindow->toggleFullscreen();
			}
		} );
}

} // namespace cat
