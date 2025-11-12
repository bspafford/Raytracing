#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#define GLM_ENABLE_EXPERIMENTAL

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

#include "math.h"
#include"shaderClass.h"

class Camera {
public:
	// Stores the main vectors of the camera
	glm::vec3 Position;
	glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 cameraMatrix = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	float FOVdeg = 45.f;
	float nearPlane = 0.001f;
	float farPlane = 1000.f;

	bool firstClick = true;

	int width;
	int height;

	// Adjust the speed of the camera and it's sensitivity when looking around
	float speed = 5.f;
	float sensitivity = 75.0f;

	// Camera constructor to set up initial values
	Camera(int width, int height, glm::vec3 position);

	// Updates the camera matrix to the Vertex Shader
	void updateMatrix(glm::vec2 screenSize);
	// Exports the camera matrix to a shader
	void Matrix(Shader* shader, const char* uniform);
	// Handles camera inputs
	void Inputs(GLFWwindow* window, float deltaTime);

	void Update(float deltaTime, GLFWwindow* window, Shader* shader, glm::vec2 screenSize);
};
#endif