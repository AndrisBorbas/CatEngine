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
		int moveLeft = GLFW_KEY_A;
		int moveRight = GLFW_KEY_D;
		int moveForward = GLFW_KEY_W;
		int moveBackward = GLFW_KEY_S;
		int moveUp = GLFW_KEY_E;
		int moveDown = GLFW_KEY_Q;
		int lookLeft = GLFW_KEY_LEFT;
		int lookRight = GLFW_KEY_RIGHT;
		int lookUp = GLFW_KEY_UP;
		int lookDown = GLFW_KEY_DOWN;
	};

	void moveInPlaneXY( GLFWwindow* window, float dt, CatObject& gameObject );

	KeyMappings keys{};
	float moveSpeed{ 3.f };
	float lookSpeed{ 1.5f };
};
} // namespace cat


#endif // CATENGINE_CATINPUT_HPP
