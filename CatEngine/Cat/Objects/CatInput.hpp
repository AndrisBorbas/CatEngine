#ifndef CATENGINE_CATINPUT_HPP
#define CATENGINE_CATINPUT_HPP

#include "Cat/CatWindow.hpp"
#include "CatObject.hpp"

namespace cat
{
class CatInput
{
public:
	struct KeyMappings
	{
		int look = GLFW_MOUSE_BUTTON_RIGHT;
		int moveLeft = GLFW_KEY_A;
		int moveRight = GLFW_KEY_D;
		int moveForward = GLFW_KEY_W;
		int moveBackward = GLFW_KEY_S;
		int moveUp = GLFW_KEY_E;
		int moveDown = GLFW_KEY_Q;
		int speed = GLFW_KEY_LEFT_SHIFT;
		int lookLeft = GLFW_KEY_LEFT;
		int lookRight = GLFW_KEY_RIGHT;
		int lookUp = GLFW_KEY_UP;
		int lookDown = GLFW_KEY_DOWN;
	};

	void moveInPlaneXZ( GLFWwindow* window, float dt, CatObject& gameObject );

private:
	KeyMappings m_eKeys{};
	float m_fMovementSpeed = 3.f;
	float m_fMouseSensitivity = 0.0045f;
	glm::vec2 m_vMouseLastPos = { 0.f, 0.f };
	bool m_bFirstMouse = true;
	float m_fYaw = -1.5f;
	float m_fPitch = -.35f;
};
} // namespace cat


#endif // CATENGINE_CATINPUT_HPP
