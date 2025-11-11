#include"Camera.h"

Camera::Camera(int width, int height, glm::vec3 position) {
	this->width = width;
	this->height = height;
	Position = position;
}

void Camera::updateMatrix(vector screenSize) {
	view = glm::lookAt(Position, Position + Orientation, Up);
	projection = glm::perspective(glm::radians(FOVdeg), screenSize.x / screenSize.y, nearPlane, farPlane);
	cameraMatrix = projection * view;
}

void Camera::Matrix(Shader* shader, const char* uniform) {
	glUniformMatrix4fv(glGetUniformLocation(shader->ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

void Camera::Update(float deltaTime, GLFWwindow* window, Shader* shader, vector screenSize) {
	Inputs(window, deltaTime);
	updateMatrix(screenSize);
	Matrix(shader, "projection");
}

void Camera::Inputs(GLFWwindow* window, float deltaTime) {
	glm::vec3 move = glm::vec3(0);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		move += Orientation;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		move += -glm::normalize(glm::cross(Orientation, Up));
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		move += -Orientation;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		move += glm::normalize(glm::cross(Orientation, Up));
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		move += Up;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		move += -Up;

	if (glm::length(move))
		move = glm::normalize(move);
	Position += speed * move * deltaTime;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick) {
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		float rotX = sensitivity * (float)(mouseY - (float(height) / 2.f)) / float(height);
		float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));

		if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f)) {
			Orientation = newOrientation;
		}

		Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

		glfwSetCursorPos(window, (width / 2), (height / 2));
	} else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}
}