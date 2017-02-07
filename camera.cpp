#include "camera.h"

glm::mat4 Camera::getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 Camera::getProjectionMatrix(){
	return ProjectionMatrix;
}

void Camera::rotate()
{
	static double lastTime = SDL_GetTicks();
	double currentTime = SDL_GetTicks();
	float deltaTime = float(currentTime - lastTime);
	timeDifference = deltaTime;
	// Get mouse position
	int xpos = e.motion.xrel;
	int ypos = e.motion.yrel;

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(/*(1024 / 2) -*/ -xpos);
	verticalAngle += mouseSpeed * float(/*(768 / 2) -*/ -ypos);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;

}
void Camera::computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called

	// Compute time difference between current and last frame

}

void Camera::move(CameraDirection dir)
{
	switch (dir)
	{
	case FORWARD:	// Move forward
		cout << "w pressed-" << endl;
		position += direction * timeDifference * speed;
		break;

	case BACK:		// Move backward
		cout << "s pressed-" << endl;
		position -= direction * timeDifference * speed;
		break;

	case RIGHT:		// Move right
		cout << "d pressed-" << endl;
		position += right * timeDifference * speed;
		break;

	case LEFT:		// Move left
		cout << "a pressed-" << endl;
		position -= right * timeDifference * speed;
		break;

	case UP:		// Look up
		verticalAngle += 0.1;
		break;

	case DOWN:
		verticalAngle -= 0.1;
		break;

	case ROT_LEFT:
		horizontalAngle += 0.1;
		break;


	case ROT_RIGHT:
		horizontalAngle -= 0.1;
		break;


	}

}

void Camera::SetPosition(glm::vec3 pos)
{
	position = InitialPos = pos;
}

void Camera::calcMatrices()
{
	// Direction : Spherical coordinates to Cartesian coordinates conversion
	direction = glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
		);

	// Right vector
	right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
		);

	// Up vector
	up = glm::cross(right, direction);

	float FoV = initialFoV;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
		);
}

void Camera::Reset()
{
	position = InitialPos;
	horizontalAngle = 3.14f;
	verticalAngle = 0.0f;
	calcMatrices();
}